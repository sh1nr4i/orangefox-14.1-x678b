#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdint>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <sstream>
#include <android-base/properties.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

using android::base::GetProperty;
using std::string;

void property_override(string prop, string value)
{
    auto pi = (prop_info *)__system_property_find(prop.c_str());

    if (pi != nullptr)
        __system_property_update(pi, value.c_str(), value.size());
    else
        __system_property_add(prop.c_str(), prop.size(), value.c_str(), value.size());
}

namespace {

// A slot suffix must be obtained from a real bootloader/kernel source. Never
// infer or hard-code _a/_b: the selected slot can change after an OTA.
static string trim_slot_value(string value)
{
    auto is_separator = [](unsigned char c) {
        return std::isspace(c) || c == '\0' || c == '"' || c == '\'';
    };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(),
        [&](unsigned char c) { return !is_separator(c); }));
    value.erase(std::find_if(value.rbegin(), value.rend(),
        [&](unsigned char c) { return !is_separator(c); }).base(), value.end());
    return value;
}

static bool is_valid_slot_suffix(const string &value)
{
    return value.size() == 2 && value[0] == '_' &&
           value[1] >= 'a' && value[1] <= 'z';
}

static string normalize_slot(string value)
{
    value = trim_slot_value(value);
    if (is_valid_slot_suffix(value)) return value;
    if (value.size() == 1 && value[0] >= 'a' && value[0] <= 'z')
        return string("_") + value;
    return "";
}

static string read_slot_from_bootconfig()
{
    std::ifstream input("/proc/bootconfig");
    string line;
    while (std::getline(input, line)) {
        const auto equal = line.find('=');
        if (equal == string::npos) continue;

        const string key = trim_slot_value(line.substr(0, equal));
        const string value = trim_slot_value(line.substr(equal + 1));
        if (key == "androidboot.slot_suffix") {
            const string suffix = normalize_slot(value);
            if (!suffix.empty()) return suffix;
        } else if (key == "androidboot.slot") {
            const string suffix = normalize_slot(value);
            if (!suffix.empty()) return suffix;
        }
    }
    return "";
}

static string read_slot_from_cmdline()
{
    std::ifstream input("/proc/cmdline");
    string line;
    std::getline(input, line);
    std::istringstream tokens(line);
    string token;
    while (tokens >> token) {
        const auto equal = token.find('=');
        if (equal == string::npos) continue;

        const string key = token.substr(0, equal);
        const string value = token.substr(equal + 1);
        if (key == "androidboot.slot_suffix" || key == "androidboot.slot") {
            const string suffix = normalize_slot(value);
            if (!suffix.empty()) return suffix;
        }
    }
    return "";
}

static string read_slot_from_device_tree()
{
    const char *paths[] = {
        "/proc/device-tree/firmware/android/slot_suffix",
        "/sys/firmware/devicetree/base/firmware/android/slot_suffix",
    };
    for (const char *path : paths) {
        std::ifstream input(path, std::ios::binary);
        if (!input) continue;
        string value((std::istreambuf_iterator<char>(input)),
                     std::istreambuf_iterator<char>());
        const string suffix = normalize_slot(value);
        if (!suffix.empty()) return suffix;
    }
    return "";
}

static uint32_t read_le32(const unsigned char *data)
{
    return static_cast<uint32_t>(data[0]) |
           (static_cast<uint32_t>(data[1]) << 8) |
           (static_cast<uint32_t>(data[2]) << 16) |
           (static_cast<uint32_t>(data[3]) << 24);
}

static uint32_t boot_control_crc32(const unsigned char *data, size_t size)
{
    uint32_t crc = 0xffffffffU;
    for (size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ (0xedb88320U & (0U - (crc & 1U)));
    }
    return crc ^ 0xffffffffU;
}

static string read_slot_from_misc()
{
    // bootloader_message is 2048 bytes; bootloader_control is stored in its
    // slot_suffix field immediately after that message on standard A/B devices.
    constexpr off_t kBootloaderControlOffset = 2048;
    constexpr size_t kBootloaderControlSize = 32;
    constexpr uint32_t kBootControlMagic = 0x42414342U;
    const char *paths[] = {
        "/dev/block/by-name/misc",
        "/dev/block/bootdevice/by-name/misc",
        "/dev/block/platform/bootdevice/by-name/misc",
        "/dev/block/platform/soc/11270000.ufshci/by-name/misc",
    };

    for (const char *path : paths) {
        const int fd = open(path, O_RDONLY | O_CLOEXEC);
        if (fd < 0) continue;

        unsigned char control[kBootloaderControlSize] = {};
        const ssize_t bytes = pread(fd, control, sizeof(control), kBootloaderControlOffset);
        close(fd);
        if (bytes != static_cast<ssize_t>(sizeof(control))) continue;

        // Validate magic, version, slot count and CRC before trusting metadata.
        if (read_le32(control + 4) != kBootControlMagic || control[8] != 1)
            continue;
        const unsigned int slot_count = control[9] & 0x07;
        if (slot_count < 2 || slot_count > 4) continue;
        if (read_le32(control + 28) != boot_control_crc32(control, 28))
            continue;
        if (control[2] != '\0') continue;

        const string suffix(reinterpret_cast<const char *>(control), 2);
        if (is_valid_slot_suffix(suffix)) return suffix;
    }
    return "";
}

static string detect_slot_suffix()
{
    string suffix = normalize_slot(GetProperty("ro.boot.slot_suffix", ""));
    if (!suffix.empty()) return suffix;

    suffix = normalize_slot(GetProperty("ro.boot.slot", ""));
    if (!suffix.empty()) return suffix;

    // Android 12+ commonly delivers androidboot arguments through bootconfig,
    // which is not necessarily visible in /proc/cmdline.
    suffix = read_slot_from_bootconfig();
    if (!suffix.empty()) return suffix;

    suffix = read_slot_from_cmdline();
    if (!suffix.empty()) return suffix;

    // AOSP also permits the bootloader to publish the suffix in the DT node.
    suffix = read_slot_from_device_tree();
    if (!suffix.empty()) return suffix;

    // If the bootloader omitted the boot arguments, use only validated A/B
    // metadata from misc. If that metadata is absent/invalid, fail closed.
    return read_slot_from_misc();
}

}  // namespace

void vendor_load_properties()
{
    string prop_partitions[] = {"", "vendor.", "odm."};
    for (const string &prop : prop_partitions)
    {
        property_override(string("ro.product.") + prop + string("brand"), "Infinix");
        property_override(string("ro.product.") + prop + string("name"), "X678B-OP");
        property_override(string("ro.product.") + prop + string("device"), "Infinix-X678B");
        property_override(string("ro.product.") + prop + string("model"), "Infinix X678B");
        property_override(string("ro.product.") + prop + string("marketname"), "Infinix NOTE 30 Pro");
        property_override(string("ro.product.system.") + prop + string("device"), "Infinix NOTE 30 Pro");
    }

    property_override("ro.build.product", "Infinix-X678B");

    const string slot_suffix = detect_slot_suffix();
    if (!slot_suffix.empty()) {
        // Export the already-selected slot to recovery tools such as AnyKernel3.
        property_override("ro.boot.slot_suffix", slot_suffix);
        property_override("ro.boot.slot", slot_suffix.substr(1));
    }

    property_override("ro.bootimage.build.date.utc", "1609430400");
    property_override("ro.build.date.utc", "1609430400");
}

#ifdef BUILD_SLOT_DETECT_BINARY
#include <sys/wait.h>

static void run_resetprop(const char *bin, const char *key, const string &val)
{
    pid_t pid = fork();
    if (pid == 0) {
        execl(bin, bin, key, val.c_str(), nullptr);
        _exit(127);
    } else if (pid > 0) {
        int status = 0;
        waitpid(pid, &status, 0);
    }
}

int main()
{
    // If slot suffix property was already populated (e.g. by vendor_load_properties
    // in init), do not fork-exec any external tool.
    const string existing = GetProperty("ro.boot.slot_suffix", "");
    if (!existing.empty()) {
        return 0;
    }

    const string slot_suffix = detect_slot_suffix();
    if (!slot_suffix.empty()) {
        const char *bin = nullptr;
        if (access("/system/bin/resetprop", X_OK) == 0) {
            bin = "/system/bin/resetprop";
        } else if (access("/sbin/resetprop", X_OK) == 0) {
            bin = "/sbin/resetprop";
        } else if (access("/system/bin/setprop", X_OK) == 0) {
            bin = "/system/bin/setprop";
        }

        if (bin) {
            run_resetprop(bin, "ro.boot.slot_suffix", slot_suffix);
            run_resetprop(bin, "ro.boot.slot", slot_suffix.substr(1));
        }
    }
    return 0;
}
#endif

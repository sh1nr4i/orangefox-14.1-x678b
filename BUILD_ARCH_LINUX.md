# Panduan Kompilasi OrangeFox 14.1 di Arch Linux (Infinix NOTE 30 Pro - X678B)

Panduan ini berisi langkah-langkah kompilasi lokal OrangeFox Recovery R12.0 (Android 14 / `fox_14.1`) untuk perangkat **Infinix NOTE 30 Pro (`Infinix-X678B` / `X678B` / `satire`)** menggunakan distribusi **Arch Linux**.

---

## 1. Kebutuhan Hardware, RAM, dan Swap / zRAM

Kompilasi AOSP Android 14 menggunakan Clang 17 dan Soong/Ninja membutuhkan memori virtual yang cukup besar saat fase linking.

### Rekomendasi Alokasi Memori:
| RAM Fisik | Alokasi Swap / zRAM | Total Memori Virtual | Keterangan |
| :--- | :--- | :--- | :--- |
| **16 GB** | **16 GB s.d. 24 GB** | ~32 GB - 40 GB | **Sangat Dianjurkan** agar proses linking Clang tidak terkena OOM (*Out Of Memory* killer). |
| **32 GB** | **8 GB s.d. 16 GB** | ~40 GB - 48 GB | Aman dan stabil. |
| **8 GB** | **24 GB s.d. 32 GB Swapfile** | ~32 GB - 40 GB | Bisa berjalan, tetapi proses akan lebih lambat karena I/O swap. |

### Rekomendasi Media Penyimpanan (Storage):
* Minimal **80 GB - 100 GB ruang kosong** pada partisi bertipe `ext4` atau `btrfs` (SSD/NVMe sangat disarankan).
  * Source tree OrangeFox 14.1 (shallow sync `--depth=1`): ~35 GB
  * Folder build (`out/`): ~30 GB - 45 GB

---

## 2. Cara Setup zRAM dan Swap di Arch Linux

Disarankan menggunakan kombinasi **zRAM (kompresi zstd)** untuk kecepatan, ditambah **Swapfile di SSD** sebagai cadangan memori.

### Opsi A: Mengaktifkan zRAM via `systemd-zram-generator`
1. Install paket zram generator:
   ```bash
   sudo pacman -S --needed systemd-zram-generator
   ```
2. Buat file konfigurasi `/etc/systemd/zram-generator.conf`:
   ```ini
   [zram0]
   zram-size = ram
   compression-algorithm = zstd
   swap-priority = 100
   ```
3. Restart systemd daemon zram:
   ```bash
   sudo systemctl daemon-reload
   sudo systemctl start /dev/zram0
   ```
4. Cek status:
   ```bash
   zramctl
   ```

### Opsi B: Membuat Swapfile Tambahan (Contoh: 16 GB)
Jika RAM fisik 16 GB dan ingin swapfile cadangan di SSD:
```bash
sudo mkswap -U clear --size 16G --file /swapfile
sudo chmod 0600 /swapfile
sudo swapon /swapfile
```
*(Tambahkan `/swapfile none swap defaults 0 0` ke dalam `/etc/fstab` jika ingin permanen).*

---

## 3. Instalasi Dependensi & Paket di Arch Linux

### Langkah 3.1: Aktifkan Repositori `multilib`
Android build system membutuhkan beberapa library 32-bit.
1. Buka file konfigurasi pacman:
   ```bash
   sudo nano /etc/pacman.conf
   ```
2. Pastikan baris berikut tidak memiliki tanda pagar (`#`):
   ```ini
   [multilib]
   Include = /etc/pacman.d/mirrorlist
   ```
3. Update database paket:
   ```bash
   sudo pacman -Syu
   ```

### Langkah 3.2: Install Paket Dasar & Tool Kompilasi
Jalankan perintah berikut:
```bash
sudo pacman -S --needed \
    base-devel git git-lfs repo python rsync zip unzip tar cpio \
    bc bison flex gperf sed gawk curl wget openssl libxml2 \
    ncurses zlib libxcrypt-compat procps-ng inetutils \
    jdk17-openjdk gcc-multilib lib32-zlib lib32-ncurses lib32-gcc-libs lib32-glibc
```

### Langkah 3.3: Atur Java Default ke OpenJDK 17
OrangeFox 14.1 (Android 14) kompatibel dengan Java 17:
```bash
sudo archlinux-java set java-17-openjdk
java -version
```

### Langkah 3.4: Konfigurasi Identitas Git
```bash
git config --global user.name "Your Name"
git config --global user.email "your.email@example.com"
```

---

## 4. Persiapan Source Code OrangeFox 14.1

1. Buat direktori kerja:
   ```bash
   mkdir -p ~/OrangeFox/fox_14.1
   cd ~/OrangeFox
   ```
2. Clone script sinkronisasi OrangeFox resmi:
   ```bash
   git clone --depth=1 https://gitlab.com/OrangeFox/sync.git
   cd sync
   ./orangefox_sync.sh --branch 14.1 --path ~/OrangeFox/fox_14.1
   ```

---

## 5. Menyiapkan Device Tree, Common Tree & Kernel Modul

Masuk ke direktori root source code:
```bash
cd ~/OrangeFox/fox_14.1
```

### Langkah 5.1: Clone Transsion MT6789 Common Tree
```bash
git clone --depth=1 -b fox_12.1-tranos15 https://github.com/transsion-mt6789-recovery/twrp-device_transsion_mt6789-common.git device/transsion/mt6789-common
```

### Langkah 5.2: Tempatkan Device Tree Infinix-X678B
Clone repo device tree kamu:
```bash
git clone --depth=1 -b master https://github.com/sh1nr4i/just_test.git device/infinix/Infinix-X678B
```

### Langkah 5.3: Sanitasi Common Tree untuk Android 14
Jalankan penyesuaian otomatis berikut:
```bash
COMMON_PATH="device/transsion/mt6789-common"
DEVICE_PATH="device/infinix/Infinix-X678B"

# Salin konfigurasi fstab, flags, dan init rc terbaru
cp -f "$DEVICE_PATH/recovery/root/first_stage_ramdisk/fstab.mt6789" "$COMMON_PATH/recovery/root/first_stage_ramdisk/fstab.mt6789"
cp -f "$DEVICE_PATH/recovery/root/system/etc/recovery.fstab" "$COMMON_PATH/recovery/root/system/etc/recovery.fstab"
cp -f "$DEVICE_PATH/recovery/root/system/etc/twrp.flags" "$COMMON_PATH/recovery/root/system/etc/twrp.flags"
cp -f "$DEVICE_PATH/recovery/root/init.recovery.mt6789.rc" "$COMMON_PATH/recovery/root/init.recovery.mt6789.rc"
cp -f "$DEVICE_PATH/recovery/root/system/etc/task_profiles.json" "$COMMON_PATH/recovery/root/system/etc/task_profiles.json"

# Sesuaikan deklarasi makefile pada common tree
sed -i 's/inherit-product,/inherit-product-if-exists,/g' "$COMMON_PATH/common.mk"
sed -i -E '/gsi_keys\.mk/d; /emulated_storage\.mk/d; /virtual_ab_ota\//d' "$COMMON_PATH/common.mk"
sed -i '/PRODUCT_TARGET_VNDK_VERSION/d' "$COMMON_PATH/common.mk"
sed -i '/TW_RECOVERY_ADDITIONAL_RELINK_LIBRARY_FILES/,+1d' "$COMMON_PATH/common.mk"
sed -i 's/PRODUCT_PACKAGES_DEBUG/PRODUCT_PACKAGES/g' "$COMMON_PATH/common.mk"

sed -i 's/BOARD_STSTEMIMAGE_FILE_SYSTEM_TYPE/BOARD_SYSTEMIMAGE_FILE_SYSTEM_TYPE/g' "$COMMON_PATH/BoardConfigCommon.mk"
sed -i 's/ --board ""//g' "$COMMON_PATH/BoardConfigCommon.mk"
sed -i '/RECOVERY_BINARY_SOURCE_FILES/d; /debuggerd/d; /strace/d' "$COMMON_PATH/BoardConfigCommon.mk"
sed -i '/TW_INCLUDE_LPTOOLS/d; /TW_INCLUDE_FB2PNG/d' "$COMMON_PATH/BoardConfigCommon.mk"

cat >> "$COMMON_PATH/BoardConfigCommon.mk" <<'EOF'
OF_DEFAULT_KEYMASTER_VERSION := 4.1
TW_EXCLUDE_LPTOOLS := true
EOF
```

### Langkah 5.4: Download Kernel Ramdisk Modules & DTB
```bash
git clone --depth=1 -b sixteen-mck https://github.com/akifakif32/android_device_infinix_X678B-kernel.git /tmp/akif_kernel
mkdir -p "$DEVICE_PATH/prebuilt" "$DEVICE_PATH/recovery/root/lib/modules"
cp -f /tmp/akif_kernel/dtb/mt6789.dtb "$DEVICE_PATH/prebuilt/dtb.img"
cp -f /tmp/akif_kernel/ramdisk/*.ko "$DEVICE_PATH/recovery/root/lib/modules/"
cp -f /tmp/akif_kernel/ramdisk/modules.dep "$DEVICE_PATH/recovery/root/lib/modules/modules.dep"
cp -f /tmp/akif_kernel/ramdisk/modules.alias "$DEVICE_PATH/recovery/root/lib/modules/modules.alias"
```

---

## 6. Proses Kompilasi (Build)

Di root source code (`~/OrangeFox/fox_14.1`):

1. **Set Environment Variables:**
   ```bash
   export ALLOW_MISSING_DEPENDENCIES=true
   export FOX_BUILD_DEVICE="X678B"
   export FOX_TARGET_DEVICES="Infinix-X678B,X678B,satire"
   export OF_NO_TREBLE_COMPATIBILITY_CHECK=1
   export OF_IGNORE_LOGICAL_MOUNT_ERRORS=1
   export OF_DISABLE_MIUI_OTA_BY_DEFAULT=1
   export OF_DEFAULT_KEYMASTER_VERSION="4.1"
   export FOX_AB_DEVICE=1
   export FOX_VIRTUAL_AB_DEVICE=1
   export FOX_VANILLA_BUILD=1
   export LC_ALL=C
   export HOST_CROSS_OS=""
   ```

2. **Nonaktifkan Cross-Compile Host Windows:**
   *(Agar Soong tidak mencoba mengompilasi DLL Windows yang memerlukan mingw)*:
   ```bash
   find build -name "envsetup.mk" -exec sed -i 's/HOST_CROSS_OS := windows/HOST_CROSS_OS :=/g' {} + 2>/dev/null || true
   ```

3. **Inisialisasi Environment & Lunch:**
   ```bash
   source build/envsetup.sh
   lunch twrp_X678B-ap2a-eng
   ```

4. **Siapkan direktori konfigurasi OrangeFox:**
   ```bash
   mkdir -p /tmp/X678B /tmp/Infinix-X678B
   if [ -f /tmp/X678B/fox_env.sh ] && [ ! -f /tmp/Infinix-X678B/fox_env.sh ]; then
     cp -f /tmp/X678B/fox_env.sh /tmp/Infinix-X678B/fox_env.sh
   fi
   ```

5. **Mulai Kompilasi `vendor_boot.img`:**
   ```bash
   mka adbd vendorbootimage
   ```

---

## 7. Hasil Build & Cara Flashing ke Perangkat

Setelah build selesai dengan pesan `#### build completed successfully ####`:

### Lokasi File Output:
File hasil build berada di direktori:
```
out/target/product/X678B/vendor_boot.img
```
*(Atau zip installer OrangeFox di folder yang sama jika target zip di-generate)*.

### Cara Flash via Fastboot:
1. Masuk ke mode Fastboot / Bootloader pada Infinix NOTE 30 Pro:
   * Matikan HP, lalu tahan tombol **Power + Volume Bawah** sampai muncul logo Fastboot.
2. Sambungkan HP ke PC via kabel USB.
3. Cek koneksi:
   ```bash
   fastboot devices
   ```
4. Flash partisi `vendor_boot`:
   ```bash
   fastboot flash vendor_boot out/target/product/X678B/vendor_boot.img
   ```
5. Masuk langsung ke OrangeFox Recovery:
   ```bash
   fastboot reboot recovery
   ```

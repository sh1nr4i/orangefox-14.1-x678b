# OrangeFox 14.1 / TWRP Device Tree — Infinix NOTE 30 Pro (`Infinix-X678B`)

Device tree recovery lengkap yang sudah diperbaiki untuk **OrangeFox 14.1 (dan 12.1)** pada **Infinix NOTE 30 Pro (`Infinix-X678B` / `X678B` / `satire`)**, kompatibel penuh untuk:
- **Android 17 / 16 / 15 / 14 / 13 / 12**
- **Custom ROM terbaru** seperti **Project Infinity-X 17.0** (`akifakif32`), LineageOS, dll. (partisi `erofs` maupun `ext4`).
- **Stock ROM XOS 13 / 14 / 15** (partisi `ext4`/`erofs` + `tr_*` Transsion partitions).

---

## 🔧 Apa Saja yang Sudah Diperbaiki di Tree Ini?

1. **Anti-Bootloop First-Stage Ramdisk (`first_stage_ramdisk/fstab.mt6789`)**:
   - Mendukung **dual filesystem (`erofs` + `ext4`)** untuk `system`, `system_ext`, `vendor`, `product`, `vendor_dlkm`, dan `odm_dlkm` beserta kunci AVB GSI dan flag `readahead_size_kb=128` dari tree 17.0.
   - Mendukung partisi `tr_*` Transsion dengan flag `nofail` (sehingga baik Stock XOS maupun Custom ROM AOSP/Infinity-X yang tidak memiliki partisi `tr_*` bisa booting normal tanpa nyangkut di logo).
2. **Deteksi Boot Slot A/B Otomatis (`init/init_Infinix-X678B.cpp`)**:
   - Mengambil kode `libinit_Infinix-X678B` yang mendeteksi slot aktif (`_a` / `_b`) secara berurutan dari:
     1. `ro.boot.slot_suffix` & `ro.boot.slot`
     2. `/proc/bootconfig` (wajib untuk GKI Android 12–17)
     3. `/proc/cmdline`
     4. Device Tree (`/proc/device-tree/firmware/android/slot_suffix`)
     5. Validasi CRC32 `bootloader_control` langsung dari partisi `/dev/block/by-name/misc`
   - Mengekspor `ro.boot.slot_suffix` dan `ro.boot.slot` agar installer seperti Magisk / KernelSU / AnyKernel3 dan flasher ROM tidak salah slot.
3. **Gabungan Fitur `sh1nr4i/android_device_infinix_Infinix-X678B` + `transsion-mt6789-common`**:
   - Menyertakan `snapuserd.rc` (penting untuk Virtual A/B & snapshot merge Android 14–17).
   - Menyertakan `mtk-plpath-utils.rc` & `init.recovery.mt6789.rc` (beserta symlink UFS `/dev/block/platform/soc/11270000.ufshci` -> `/dev/block/bootdevice` dan USB `cmode 2`).
   - Menyertakan `recovery/root/system/etc/task_profiles.json` (wajib di source OrangeFox 14.1 agar tidak crash saat init).
4. **Prebuilt Kernel Modules & DTB Terbaru (`akifakif32` 17.0 / `sixteen-mck`) + Touchscreen Firmware**:
   - Semua 199 modul `.ko` ramdisk (`ft3681_common.ko`, `nt37703_fhdp_dsi_vdo_tm_panda_120hz_x678b.ko`, dll.) dan `dtb.img` sudah dimasukkan di `recovery/root/lib/modules/` dan `prebuilt/dtb.img`.
   - Firmware touchscreen (`focaltech_ts_fw.bin`, `novatek_ts_fw*.bin`) dan haptic (`aw862xx*.bin`) lengkap di `recovery/root/vendor/firmware/`.

---

## 🚀 Cara Pakai (GitHub Actions)

Folder ini (`fox_14.1_Infinix-X678B`) bisa langsung kamu **push ke repo GitHub** kamu (misal ke `just_test` atau repo baru):
1. Push seluruh isi folder `fox_14.1_Infinix-X678B` ke repository GitHub kamu.
2. Buka tab **Actions** -> pilih workflow **OrangeFox 14.1 - Infinix X678B (Universal Android 12-17 / Infinity-X 17.0)**.
3. Klik **Run workflow** (biarkan default `FOX_BRANCH: 14.1` dan `COMMON_BRANCH: fox_12.1-tranos15`).
4. Setelah selesai, unduh `vendor_boot.img` atau `OrangeFox-*.zip` dari bagian **Artifacts**.

### Cara Flash:
```bash
fastboot flash vendor_boot vendor_boot.img
fastboot reboot recovery
```

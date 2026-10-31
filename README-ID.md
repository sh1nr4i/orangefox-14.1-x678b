# OrangeFox 14.1 untuk Infinix X678B — tree 17.0 (eksperimental)

Paket ini menambahkan workflow GitHub Actions khusus untuk mencoba build `vendor_boot.img` dari tree `akifakif32/android_device_infinix_X678B` branch `17.0`. Workflow membuat produk recovery-only `omni_X678B`; ia **tidak** mewarisi `device.mk` ROM Infinity-X karena file itu menarik banyak paket sistem operasi yang bukan kebutuhan build recovery.

## Cara pakai

1. Unduh dan ekstrak ZIP ini.
2. Di fork builder kamu, buka branch `OrangeFox` → upload file `.github/workflows/OrangeFox-X678B-17.0-VendorBoot.yml` ke path yang sama. Pastikan folder `.github/workflows/` ada.
3. Commit perubahan tersebut.
4. Buka tab **Actions** → workflow **OrangeFox 14.1 - Infinix X678B (17.0 tree, vendor_boot)** → **Run workflow**.
5. Tunggu hasilnya. Kalau gagal, buka langkah yang merah dan periksa `build-vendorboot.log` pada artifact jika artifact berhasil dibuat.
6. Kalau sukses, ambil `vendor_boot.img` dari artifact `OrangeFox-14.1-X678B-17.0-vendor_boot`.

Tidak perlu mengubah input workflow; parameter target dan branch sudah diisi di file.

## Kenapa ada dependency dari branch lain?

- Device tree: `akifakif32/android_device_infinix_X678B`, branch `17.0`.
- Vendor blobs/config: `akifakif32/android_vendor_infinix_X678B`, branch `17.0`.
- Common GKI image: `MillenniumOSS/android_device_millennium_common-kernel`, branch `seventeen`.
- Kernel module files: `akifakif32/android_device_infinix_X678B-kernel`, branch `sixteen-mck`. Repository kernel module yang bisa diverifikasi saat ini belum menyediakan branch `17.0`, sehingga modul dari branch ini perlu dicocokkan dengan kernel GKI yang dipakai.
- MediaTek SEPolicy: `LineageOS/android_device_mediatek_sepolicy_vndr`, branch `lineage-21` agar lebih dekat dengan source recovery Android 14.

Workflow menonaktifkan `vendorsetup.sh` milik tree 17.0 karena isinya menerapkan patch ROM Infinity-X, bukan konfigurasi recovery. Ia juga memeriksa file, image kernel, module list, dan flag `vendor_boot` **sebelum** kompilasi.

## Catatan penting

Ini adalah port eksperimental: tree ROM Android 17 bukan tree recovery OrangeFox 14.1. Perubahan ini memperbaiki format perintah `lunch` (menggunakan `lunch omni_X678B trunk_staging eng`) dan membuat produk recovery minimal, tetapi tidak bisa menjamin source OrangeFox 14.1 kompatibel dengan seluruh konfigurasi di tree Android 17. Workflow akan memeriksa dependency yang jelas dan berhenti lebih awal jika ada file wajib yang hilang; error kompilasi berikutnya tetap harus dinilai dari log.

Jangan flash `vendor_boot.img` hanya karena build sukses. Verifikasi bahwa partisi `vendor_boot` pada perangkat memang 64 MiB atau lebih besar, periksa image dengan `unpack_bootimg`, lalu uji metode boot/flashing yang sesuai untuk device dan bootloader kamu. Jangan menulis image ke partisi lain.

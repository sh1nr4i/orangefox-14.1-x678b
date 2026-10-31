#
# Copyright (C) 2024 The LineageOS Project
# Copyright (C) 2024 SebaUbuntu's TWRP device tree generator
#
# SPDX-License-Identifier: Apache-2.0
#

DEVICE_PATH := device/infinix/Infinix-X678B

# Inherit from mt6789-common
$(call inherit-product, device/transsion/mt6789-common/common.mk)

# A/B Boot Slot Detection Helper & Boot Control CLI (required by OrangeFox 14.1 AB devices)
PRODUCT_PACKAGES += \
    unified_slot_detect \
    bootctl

# Fast Charging & USB properties
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.mtk_fast_charging_support=1

# Recovery Ramdisk files & directories required by OrangeFox 14.1 (A14+)
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/recovery/root/sbin/.keep:recovery/root/sbin/.keep \
    $(DEVICE_PATH)/recovery/root/linkerconfig/.keep:recovery/root/linkerconfig/.keep \
    $(DEVICE_PATH)/recovery/root/system/etc/task_profiles.json:recovery/root/system/etc/task_profiles.json \
    $(DEVICE_PATH)/recovery/root/mtk-plpath-utils.rc:recovery/root/mtk-plpath-utils.rc \
    $(DEVICE_PATH)/recovery/root/snapuserd.rc:recovery/root/snapuserd.rc

# Platform Vendor Ramdisk (Ramdisk 0 in vendor_boot.img)
# Required so Normal Android OS boot (Android 12-17) has /first_stage_ramdisk/fstab.mt6789
# and /lib/modules/* even when vendor_boot.img is flashed directly via fastboot!
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/recovery/root/first_stage_ramdisk/fstab.mt6789:$(TARGET_COPY_OUT_VENDOR_RAMDISK)/first_stage_ramdisk/fstab.mt6789 \
    $(call find-copy-subdir-files,*,$(DEVICE_PATH)/recovery/root/lib/modules,$(TARGET_COPY_OUT_VENDOR_RAMDISK)/lib/modules)

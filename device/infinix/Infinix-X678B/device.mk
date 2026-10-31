#
# Copyright (C) 2024 The LineageOS Project
# Copyright (C) 2024 SebaUbuntu's TWRP device tree generator
#
# SPDX-License-Identifier: Apache-2.0
#

LOCAL_PATH := device/infinix/Infinix-X678B

# Inherit from mt6789-common
$(call inherit-product, device/transsion/mt6789-common/common.mk)

# Fast Charging & USB properties
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.mtk_fast_charging_support=1

# Recovery Ramdisk files & directories required by OrangeFox 14.1 (A14+)
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/recovery/root/sbin/.keep:$(TARGET_COPY_OUT_RECOVERY)/root/sbin/.keep \
    $(LOCAL_PATH)/recovery/root/linkerconfig/.keep:$(TARGET_COPY_OUT_RECOVERY)/root/linkerconfig/.keep \
    $(LOCAL_PATH)/recovery/root/system/etc/task_profiles.json:$(TARGET_COPY_OUT_RECOVERY)/root/system/etc/task_profiles.json \
    $(LOCAL_PATH)/recovery/root/mtk-plpath-utils.rc:$(TARGET_COPY_OUT_RECOVERY)/root/mtk-plpath-utils.rc \
    $(LOCAL_PATH)/recovery/root/snapuserd.rc:$(TARGET_COPY_OUT_RECOVERY)/root/snapuserd.rc

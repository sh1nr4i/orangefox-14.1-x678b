#
# Copyright (C) 2024 The LineageOS Project
# Copyright (C) 2024 SebaUbuntu's TWRP device tree generator
#
# SPDX-License-Identifier: Apache-2.0
#

LOCAL_PATH := device/infinix/Infinix-X678B

# Inherit from mt6789-common
$(call inherit-product, device/transsion/mt6789-common/common.mk)

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    $(LOCAL_PATH)

# Recovery Ramdisk directories required by OrangeFox 14.1 (A14+)
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/recovery/root/system/etc/task_profiles.json:$(TARGET_COPY_OUT_RECOVERY)/root/system/etc/task_profiles.json \
    $(LOCAL_PATH)/recovery/root/mtk-plpath-utils.rc:$(TARGET_COPY_OUT_RECOVERY)/root/mtk-plpath-utils.rc \
    $(LOCAL_PATH)/recovery/root/snapuserd.rc:$(TARGET_COPY_OUT_RECOVERY)/root/snapuserd.rc

#
# Copyright (C) 2024 The LineageOS Project
# Copyright (C) 2024 SebaUbuntu's TWRP device tree generator
#
# SPDX-License-Identifier: Apache-2.0
#

DEVICE_PATH := device/infinix/Infinix-X678B
COMMON_PATH := device/transsion/mt6789-common

# Inherit from mt6789-common
include $(COMMON_PATH)/BoardConfigCommon.mk

# Architecture (Updated for Cortex-A76/A55 on MT6789 / Android 14-17)
TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv8-2a-dotprod
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_ABI2 :=
TARGET_CPU_VARIANT := generic
TARGET_CPU_VARIANT_RUNTIME := cortex-a76

# Assert (Supports both stock/recovery Infinix-X678B and Infinity-X 17.0 X678B/satire)
TARGET_OTA_ASSERT_DEVICE := Infinix-X678B,X678B,satire

# Init (Slot suffix detection from bootconfig/cmdline/dt/misc + product props)
TARGET_INIT_VENDOR_LIB := libinit_Infinix-X678B
TARGET_RECOVERY_DEVICE_MODULES += libinit_Infinix-X678B

# Display
TARGET_SCREEN_DENSITY := 480

# Dynamic Partitions (Supports both Stock main and Infinity-X mtk_dynamic_partitions)
BOARD_SUPER_PARTITION_SIZE := 9126805504
BOARD_SUPER_PARTITION_GROUPS := main
BOARD_MAIN_SIZE := 9122611200
BOARD_MAIN_PARTITION_LIST := \
    odm_dlkm \
    product \
    system \
    system_ext \
    vendor \
    vendor_dlkm

# TWRP / OrangeFox 14.1 Configs
TW_DEVICE_VERSION := X678B_OFRP_14.1
TW_LOAD_VENDOR_BOOT_MODULES := true
TW_INCLUDE_FASTBOOTD := true
TW_INCLUDE_REPACKTOOLS := true
TW_INCLUDE_RESETPROP := true
TW_EXCLUDE_LPTOOLS := true
TW_USE_TOOLBOX := true
TW_INPUT_BLACKLIST := "hbtp_vm"

# OrangeFox 14.1 Specific Flags (Required when TW_FORCE_KEYMASTER_VER := true)
OF_DEFAULT_KEYMASTER_VERSION := 4.1
FOX_AB_DEVICE := 1
FOX_VIRTUAL_AB_DEVICE := 1
FOX_VANILLA_BUILD := 1
FOX_RECOVERY_VENDOR_BOOT_PARTITION := /dev/block/by-name/vendor_boot

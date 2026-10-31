#!/bin/bash
if [ ! -d "device/transsion/mt6789-common" ]; then
    git clone https://github.com/transsion-mt6789-recovery/twrp-device_transsion_mt6789-common.git -b fox_12.1-tranos15 device/transsion/mt6789-common
fi

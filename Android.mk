#
# Copyright (C) 2015 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_DEVICE),oneplus2)

include $(call all-makefiles-under,$(LOCAL_PATH))

include $(CLEAR_VARS)

$(shell mkdir -p $(TARGET_OUT_ETC)/firmware/wcd9320)

WCD9320 := \
    wcd9320_anc.bin wcd9320_mad_audio.bin wcd9320_mbhc.bin

WCD9320_SYMLINKS := $(addprefix $(TARGET_OUT_ETC)/firmware/wcd9320/,$(notdir $(WCD9320)))
$(WCD9320_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "WCD9320 firmware link: $@"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf /data/misc/audio/$(notdir $@) $@

ALL_DEFAULT_INSTALLED_MODULES += $(WCD9320_SYMLINKS)

$(shell mkdir -p $(TARGET_OUT_VENDOR)/firmware/keymaster; \
    ln -sf /firmware/image/keymaste.b00 \
        $(TARGET_OUT_VENDOR)/firmware/keymaster/keymaster.b00; \
    ln -sf /firmware/image/keymaste.b01 \
        $(TARGET_OUT_VENDOR)/firmware/keymaster/keymaster.b01; \
    ln -sf /firmware/image/keymaste.b02 \
        $(TARGET_OUT_VENDOR)/firmware/keymaster/keymaster.b02; \
    ln -sf /firmware/image/keymaste.b03 \
        $(TARGET_OUT_VENDOR)/firmware/keymaster/keymaster.b03; \
    ln -sf /firmware/image/keymaste.mdt \
        $(TARGET_OUT_VENDOR)/firmware/keymaster/keymastermdt)

$(shell mkdir -p $(TARGET_OUT_ETC)/firmware; \
    ln -sf /dev/block/bootdevice/by-name/msadp \
        $(TARGET_OUT_ETC)/firmware/msadp)

# Create a link for the WCNSS config file, which ends up as a writable
# version in /data/misc/wifi
$(shell mkdir -p $(TARGET_OUT_ETC)/firmware/wlan/qca_cld; \
    ln -sf /system/etc/wifi/WCNSS_qcom_cfg.ini \
        $(TARGET_OUT_ETC)/firmware/wlan/qca_cld/WCNSS_qcom_cfg.ini)

endif

#
# Copyright 2014 The Android Open Source Project
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

# This contains the module build definitions for the hardware-specific
# components for this device.
#
# As much as possible, those components should be built unconditionally,
# with device-specific names to avoid collisions, to avoid device-specific
# bitrot and build breakages. Building a component unconditionally does
# *not* include it on all devices, so it is safe even with hardware-specific
# components.

LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_DEVICE),oneplus2)

include $(call all-subdir-makefiles,$(LOCAL_PATH))

include $(CLEAR_VARS)

LOCAL_MODULE := wifi_symlinks
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := FAKE
LOCAL_MODULE_SUFFIX := -timestamp

include $(BUILD_SYSTEM)/base_rules.mk

$(LOCAL_BUILT_MODULE): ACTUAL_INI_FILE := /system/etc/wifi/WCNSS_qcom_cfg.ini
$(LOCAL_BUILT_MODULE): WCNSS_INI_SYMLINK := $(TARGET_OUT)/etc/firmware/wlan/qca_cld/WCNSS_qcom_cfg.ini

$(LOCAL_BUILT_MODULE): ACTUAL_BIN_FILE := /system/etc/wifi/WCNSS_qcom_wlan_nv.bin
$(LOCAL_BUILT_MODULE): WCNSS_BIN_SYMLINK := $(TARGET_OUT)/etc/firmware/wlan/qca_cld/WCNSS_qcom_wlan_nv.bin

$(LOCAL_BUILT_MODULE): ACTUAL_DAT_FILE := /system/etc/wifi/WCNSS_cfg.dat
$(LOCAL_BUILT_MODULE): WCNSS_DAT_SYMLINK := $(TARGET_OUT)/etc/firmware/wlan/qca_cld/WCNSS_cfg.dat

$(LOCAL_BUILT_MODULE): ACTUAL_MAC_FILE := /persist/wlan_mac.bin
$(LOCAL_BUILT_MODULE): WCNSS_MAC_SYMLINK := $(TARGET_OUT)/etc/firmware/wlan/qca_cld/wlan_mac.bin


$(LOCAL_BUILT_MODULE): $(LOCAL_PATH)/Android.mk
$(LOCAL_BUILT_MODULE):
	$(hide) echo "Making symlinks for wifi"
	$(hide) mkdir -p $(dir $@)
	$(hide) mkdir -p $(dir $(WCNSS_BIN_SYMLINK))
	$(hide) rm -rf $@
	$(hide) rm -rf $(WCNSS_INI_SYMLINK)
	$(hide) ln -sf $(ACTUAL_INI_FILE) $(WCNSS_INI_SYMLINK)
	$(hide) rm -rf $(WCNSS_BIN_SYMLINK)
	$(hide) ln -sf $(ACTUAL_BIN_FILE) $(WCNSS_BIN_SYMLINK)
	$(hide) rm -rf $(WCNSS_DAT_SYMLINK)
	$(hide) ln -sf $(ACTUAL_DAT_FILE) $(WCNSS_DAT_SYMLINK)
	$(hide) rm -rf $(WCNSS_MAC_SYMLINK)
	$(hide) ln -sf $(ACTUAL_MAC_FILE) $(WCNSS_MAC_SYMLINK)
	$(hide) touch $@

include $(call all-makefiles-under,$(LOCAL_PATH))

IMS_LIBS := libimscamera_jni.so libimsmedia_jni.so

IMS_SYMLINKS := $(addprefix $(TARGET_OUT)/app/ims/lib/arm64/,$(notdir $(IMS_LIBS)))
$(IMS_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "IMS lib link: $@"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf /system/vendor/lib64/$(notdir $@) $@

ALL_DEFAULT_INSTALLED_MODULES += $(IMS_SYMLINKS)

include device/oneplus/oneplus2/tftp.mk

endif

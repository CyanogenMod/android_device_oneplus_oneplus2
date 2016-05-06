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

$(call inherit-product, device/oneplus/oneplus2/full_oneplus2.mk)

# Inherit some common CM stuff.
$(call inherit-product, vendor/cm/config/common_full_phone.mk)

PRODUCT_NAME := cm_oneplus2
PRODUCT_DEVICE := oneplus2
PRODUCT_MANUFACTURER := OnePlus
PRODUCT_MODEL := A2001
RELEASE_TYPE := cm_UNOFFICIAL

PRODUCT_GMS_CLIENTID_BASE := android-oneplus

PRODUCT_BRAND := oneplus
TARGET_VENDOR := oneplus
TARGET_VENDOR_PRODUCT_NAME := oneplus2
TARGET_VENDOR_DEVICE_NAME := A2001
PRODUCT_BUILD_PROP_OVERRIDES += TARGET_DEVICE=oneplus2 PRODUCT_NAME=OnePlus2

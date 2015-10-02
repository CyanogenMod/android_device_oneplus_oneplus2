LOCAL_PATH:= $(call my-dir)

ifeq ($(TARGET_DEVICE),oneplus2)
include $(CLEAR_VARS)

LOCAL_MODULE := com.cyanogenmod.keyhandler
LOCAL_SRC_FILES := $(call all-java-files-under,src)
LOCAL_MODULE_TAGS := optional
LOCAL_DEX_PREOPT := false

include $(BUILD_JAVA_LIBRARY)
endif


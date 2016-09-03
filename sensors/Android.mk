LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sensors.ssc.wrapper
LOCAL_PROPRIETARY_MODULE := true

LOCAL_SRC_FILES := \
    SensorsWrapper.c

LOCAL_SHARED_LIBRARIES := libdl liblog

include $(BUILD_SHARED_LIBRARY)

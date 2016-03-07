LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	fpc1020_hal.cpp \
	Fpc1020Sensor.cpp \
	QSEEComApp.cpp

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libdl \
	libutils

LOCAL_MODULE := fingerprint.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

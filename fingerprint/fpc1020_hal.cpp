/*
 *  Copyright (C) 2015 The CyanogenMod Project
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you
 *  may not use this file except in compliance with the License.  You may
 *  obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 *  implied.  See the License for the specific language governing
 *  permissions and limitations under the License.
 */

#define LOG_TAG "FingerprintHAL"
// #define LOG_NDEBUG 0

#include <cutils/log.h>
#include <hardware/hardware.h>
#include <hardware/fingerprint.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include "Fpc1020Sensor.h"

typedef struct {
    fingerprint_device_t device;
    Fpc1020Sensor *impl;
    android::Mutex notify_lock;
} fpc1020_device_t;

#define to_impl(dev) (((fpc1020_device_t *) dev)->impl)

static fingerprint_notify_t fingerprint_get_notify(struct fingerprint_device *dev)
{
    fpc1020_device_t *fpc1020_dev = (fpc1020_device_t *) dev;
    android::Mutex::Autolock l(fpc1020_dev->notify_lock);
    return dev->notify;
}

static int fingerprint_close(hw_device_t *dev)
{
    fpc1020_device_t *fpc1020_dev = (fpc1020_device_t *) dev;
    if (fpc1020_dev) {
        delete fpc1020_dev->impl;
        delete fpc1020_dev;
        return 0;
    }

    return -1;
}

static int fingerprint_authenticate(struct fingerprint_device *dev)
{
    ALOGV("fingerprint_authenticate");
    int ret = to_impl(dev)->startAuthentication();
    if (ret != 0) {
        ALOGE("Starting authentication mode failed: %d", ret);
        return FINGERPRINT_ERROR;
    }

    return 0;
}

static int fingerprint_cancel(struct fingerprint_device *dev)
{
    ALOGV("fingerprint_cancel");
    int ret = to_impl(dev)->goToIdleState();
    if (ret != 0) {
        ALOGE("Transitioning state machine to idle state failed: %d", ret);
        return FINGERPRINT_ERROR;
    }

    return 0;
}

static int fingerprint_enroll(struct fingerprint_device *dev,
                              uint32_t timeout_sec)
{
    ALOGV("fingerprint_enroll, timeout %d sec", timeout_sec);
    int ret = to_impl(dev)->startEnrollment(timeout_sec);
    if (ret != 0) {
        ALOGE("Starting enrollment mode failed: %d", ret);
        return FINGERPRINT_ERROR;
    }

    return 0;
}

static int fingerprint_remove(struct fingerprint_device *dev,
                              uint32_t fingerprint_id)
{
    ALOGV("fingerprint_remove, id 0x%08x", fingerprint_id);
    int ret = to_impl(dev)->removeId(fingerprint_id);
    if (ret != 0) {
        ALOGE("Removing enrolled fingerprint failed: %d", ret);
        return FINGERPRINT_ERROR;
    }

    fingerprint_notify_t notify = fingerprint_get_notify(dev);
    if (notify) {
        fingerprint_msg_t msg;
        msg.type = FINGERPRINT_TEMPLATE_REMOVED;
        msg.data.removed.id = fingerprint_id;
        notify(msg);
    }

    return 0;
}

static int set_notify_callback(struct fingerprint_device *dev,
                               fingerprint_notify_t notify)
{
    fpc1020_device_t *fpc1020_dev = (fpc1020_device_t *) dev;
    android::Mutex::Autolock l(fpc1020_dev->notify_lock);
    dev->notify = notify;

    return 0;
}

static int fingerprint_get_enrollment_info(struct fingerprint_device *dev,
                                           enrollment_info_t **enrollmentInfo)
{
    ALOGV("fingerprint_get_enrollment_info");
    android::Vector<uint32_t> ids;
    int ret = to_impl(dev)->getEnrolledIds(ids);

    if (ret != 0) {
        ALOGE("Getting enrolled fingerprints failed: %d", ret);
        return FINGERPRINT_ERROR;
    }

    enrollment_info_t *info = new enrollment_info_t;
    if (!info) {
        ALOGE("Allocating enrollment info failed");
        return FINGERPRINT_ERROR;
    }
    info->num_fingers = ids.size();
    info->fpinfo = new fingerprint_t[ids.size()];
    if (!info->fpinfo) {
        ALOGE("Allocating fingerprint info failed");
        delete info;
        return FINGERPRINT_ERROR;
    }
    for (size_t i = 0; i < ids.size(); i++) {
        info->fpinfo[i].index = ids[i];
    }

    *enrollmentInfo = info;
    return 0;
}

static int fingerprint_release_enrollment_info(struct fingerprint_device __unused *dev,
                                               enrollment_info_t *enrollmentInfo)
{
    ALOGV("fingerprint_release_enrollment_info");
    if (!enrollmentInfo) {
        return FINGERPRINT_ERROR;
    }

    delete [] enrollmentInfo->fpinfo;
    delete enrollmentInfo;
    return 0;
}

static int fingerprint_get_num_enrollment_steps(struct fingerprint_device __unused *dev)
{
    return Fpc1020Sensor::EnrollmentStepCount;
}

static int fingerprint_set_parameters(struct fingerprint_device *dev, const char *kvpairs)
{
    ALOGV("fingerprint_set_parameters %s", kvpairs);

    android::KeyedVector<android::String8, android::String8> paramMap;
    // from CameraParameters.cpp
    const char *a = kvpairs, *b;

    for (;;) {
        // Find the bounds of the key name.
        b = strchr(a, '=');
        if (b == 0)
            break;

        // Create the key string.
        android::String8 k(a, (size_t)(b-a));

        // Find the value.
        a = b+1;
        b = strchr(a, ';');
        if (b == 0) {
            // If there's no semicolon, this is the last item.
            android::String8 v(a);
            paramMap.add(k, v);
            break;
        }

        android::String8 v(a, (size_t)(b-a));
        paramMap.add(k, v);
        a = b+1;
    }

    bool wakeup = paramMap.valueFor(android::String8("wakeup")) == "1";
    int ret = to_impl(dev)->setWakeupMode(wakeup);

    if (ret != 0) {
        ALOGE("Setting wakeup mode failed: %d", ret);
        return FINGERPRINT_ERROR;
    }

    return 0;
}

static void fingerprint_cb_acquired(void *data)
{
    struct fingerprint_device *dev = (struct fingerprint_device *) data;
    fingerprint_notify_t notify = fingerprint_get_notify(dev);
    if (notify) {
        fingerprint_msg_t msg;
        msg.type = FINGERPRINT_ACQUIRED;
        msg.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_GOOD;
        notify(msg);
    }
}

static void fingerprint_cb_enrollment_progress(uint32_t id,
                                               int steps_remaining,
                                               void *data)
{
    struct fingerprint_device *dev = (struct fingerprint_device *) data;
    fingerprint_notify_t notify = fingerprint_get_notify(dev);
    if (notify) {
        fingerprint_msg_t msg;
        msg.type = FINGERPRINT_TEMPLATE_ENROLLING;
        msg.data.enroll.id = id;
        msg.data.enroll.samples_remaining = steps_remaining;
        notify(msg);
    }
}

static void fingerprint_cb_authenticate(bool success,
                                        uint32_t id,
                                        void *data)
{
    struct fingerprint_device *dev = (struct fingerprint_device *) data;
    fingerprint_notify_t notify = fingerprint_get_notify(dev);
    if (notify) {
        fingerprint_msg_t msg;
        msg.type = FINGERPRINT_PROCESSED;
        msg.data.processed.id = success ? id : 0;
        notify(msg);
    }
}

static void fingerprint_cb_error(int result, void *data)
{
    struct fingerprint_device *dev = (struct fingerprint_device *) data;
    fingerprint_notify_t notify = fingerprint_get_notify(dev);
    if (notify) {
        fingerprint_msg_t msg;
        msg.type = FINGERPRINT_ERROR;
        switch (result) {
            case -EINTR: msg.data.error = FINGERPRINT_ERROR_CANCELED; break;
            case -ETIMEDOUT: msg.data.error = FINGERPRINT_ERROR_TIMEOUT; break;
            default: msg.data.error = FINGERPRINT_ERROR_HW_UNAVAILABLE; break;
        }
        notify(msg);
    }
}

static int fingerprint_open(const hw_module_t* module,
                            const char __unused *id,
                            hw_device_t** device)
{
    if (device == NULL) {
        ALOGE("NULL device on open");
        return -EINVAL;
    }

    fpc1020_device_t *dev = new fpc1020_device_t;
    if (!dev) {
        return -ENOMEM;
    }

    dev->impl = new Fpc1020Sensor(fingerprint_cb_acquired,
            fingerprint_cb_enrollment_progress,
            fingerprint_cb_authenticate,
            fingerprint_cb_error, dev);
    if (!dev->impl) {
        delete dev;
        return -ENOMEM;
    }

    dev->device.common.tag = HARDWARE_DEVICE_TAG;
    dev->device.common.version = HARDWARE_MODULE_API_VERSION(1, 2);
    dev->device.common.module = (struct hw_module_t*) module;
    dev->device.common.close = fingerprint_close;

    dev->device.authenticate = fingerprint_authenticate;
    dev->device.cancel = fingerprint_cancel;
    dev->device.enroll = fingerprint_enroll;
    dev->device.remove = fingerprint_remove;
    dev->device.set_notify = set_notify_callback;
    dev->device.notify = NULL;
    dev->device.get_enrollment_info = fingerprint_get_enrollment_info;
    dev->device.release_enrollment_info = fingerprint_release_enrollment_info;
    dev->device.get_num_enrollment_steps = fingerprint_get_num_enrollment_steps;
    dev->device.set_parameters = fingerprint_set_parameters;

    *device = (hw_device_t *) &dev->device;

    return 0;
}

static struct hw_module_methods_t fingerprint_module_methods = {
    .open = fingerprint_open,
};

fingerprint_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag                = HARDWARE_MODULE_TAG,
        .module_api_version = FINGERPRINT_MODULE_API_VERSION_1_2,
        .hal_api_version    = HARDWARE_HAL_API_VERSION,
        .id                 = FINGERPRINT_HARDWARE_MODULE_ID,
        .name               = "FPC1020 Fingerprint HAL",
        .author             = "The CyanogenMod Project",
        .methods            = &fingerprint_module_methods,
        .dso                = NULL,
        .reserved           = { 0 }
    }
};

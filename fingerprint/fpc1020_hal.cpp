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
    uint64_t operation_id;
    uint32_t gid;
    uint64_t challenge;
} fpc1020_device_t;

#define to_impl(dev) (((fpc1020_device_t *) dev)->impl)

static fingerprint_notify_t fingerprint_get_notify(struct fingerprint_device *dev)
{
    fpc1020_device_t *fpc1020_dev = (fpc1020_device_t *) dev;
    android::Mutex::Autolock l(fpc1020_dev->notify_lock);
    return dev->notify;
}

static uint64_t get_64bit_rand() {
    return (((uint64_t) rand()) << 32) | ((uint64_t) rand());
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

static int fingerprint_authenticate(struct fingerprint_device *dev,
                                    uint64_t operation_id,
                                    uint32_t gid)
{
    fpc1020_device_t *device = (fpc1020_device_t *) dev;
    ALOGV("fingerprint_authenticate");
    int ret = to_impl(dev)->startAuthentication();
    if (ret != 0) {
        ALOGE("Starting authentication mode failed: %d", ret);
        return ret;
    }

    device->operation_id = operation_id;
    device->gid = gid;

    return 0;
}

static int fingerprint_cancel(struct fingerprint_device *dev)
{
    ALOGV("fingerprint_cancel");
    int ret = to_impl(dev)->goToIdleState();
    if (ret != 0) {
        ALOGE("Transitioning state machine to idle state failed: %d", ret);
    }
    return ret;
}

static uint64_t fingerprint_pre_enroll(struct fingerprint_device *dev) {
    ALOGV("fingerprint_pre_enroll");
    fpc1020_device_t *device = (fpc1020_device_t *) dev;
    device->challenge = get_64bit_rand();
    return device->challenge;
}

static int fingerprint_post_enroll(struct fingerprint_device *dev) {
    ALOGV("fingerprint_post_enroll");
    fpc1020_device_t *device = (fpc1020_device_t *) dev;
    device->challenge = 0;
    return 0;
}

static int fingerprint_enroll(struct fingerprint_device *dev,
                              const hw_auth_token_t *hat,
                              uint32_t gid,
                              uint32_t timeout_sec)
{
    ALOGV("fingerprint_enroll, timeout %d sec", timeout_sec);

    fpc1020_device_t *device = (fpc1020_device_t *) dev;
    if (!hat) {
        ALOGW("Null auth token");
        return -EINVAL;
    }

    if (hat->version != HW_AUTH_TOKEN_VERSION) {
        return -EPROTONOSUPPORT;
    }
    if (hat->challenge != device->challenge && !(hat->authenticator_type & HW_AUTH_FINGERPRINT)) {
        return -EPERM;
    }

    if (gid == 0) {
        gid = rand();
    }

    int ret = to_impl(dev)->startEnrollment(timeout_sec, hat->user_id, gid);
    if (ret != 0) {
        ALOGE("Starting enrollment mode failed: %d", ret);
    }

    return ret;
}

static uint64_t fingerprint_get_auth_id(struct fingerprint_device *dev)
{
    return to_impl(dev)->getAuthenticatorId();
}

static int fingerprint_set_active_group(struct fingerprint_device *dev,
                                        uint32_t gid,
                                        const char * __unused path)
{
    fpc1020_device_t *device = (fpc1020_device_t *) dev;
    device->gid = gid;
    return 0;
}

static int fingerprint_enumerate(struct fingerprint_device *dev,
                                 fingerprint_finger_id_t *results,
                                 uint32_t *max_size)
{
    android::Vector<Fpc1020Sensor::EnrolledFingerprint> fps;
    android::Vector<Fpc1020Sensor::EnrolledFingerprint>::iterator iter;
    fpc1020_device_t *device = (fpc1020_device_t *) dev;
    int ret = to_impl(dev)->getEnrolledFingerprints(fps);

    if (ret != 0) {
        ALOGE("Getting enrolled fingerprints failed: %d", ret);
        return ret;
    }

    for (iter = fps.begin(); iter != fps.end(); ) {
        if (device->gid != 0 && device->gid != iter->gid) {
            iter = fps.erase(iter);
        } else {
            ++iter;
        }
    }

    if (*max_size == 0) {
        *max_size = fps.size();
    } else {
        for (size_t i = 0; i < *max_size && i < fps.size(); i++) {
            results[i].fid = fps[i].fid;
            results[i].gid = fps[i].gid;
        }
    }

    return 0;
}

static int fingerprint_remove(struct fingerprint_device *dev,
                              uint32_t gid,
                              uint32_t fid)
{
    ALOGV("fingerprint_remove, id 0x%08x gid 0x%08x", fid, gid);
    fpc1020_device_t *device = (fpc1020_device_t *) dev;
    if (device->gid != gid) {
        return -EINVAL;
    }
    Fpc1020Sensor::EnrolledFingerprint fp(fid, gid);
    int ret = device->impl->removeId(fp);
    if (ret != 0) {
        ALOGE("Removing enrolled fingerprint failed: %d", ret);
        return ret;
    }

    device->impl->goToIdleState();

    fingerprint_notify_t notify = fingerprint_get_notify(dev);
    if (notify) {
        fingerprint_msg_t msg;
        msg.type = FINGERPRINT_TEMPLATE_REMOVED;
        msg.data.removed.finger.fid = fp.fid;
        msg.data.removed.finger.gid = fp.gid;
        notify(&msg);
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

static void fingerprint_cb_acquired(void *data)
{
    struct fingerprint_device *dev = (struct fingerprint_device *) data;
    fingerprint_notify_t notify = fingerprint_get_notify(dev);
    if (notify) {
        fingerprint_msg_t msg;
        msg.type = FINGERPRINT_ACQUIRED;
        msg.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_GOOD;
        notify(&msg);
    }
}

static void fingerprint_cb_enrollment_progress(const Fpc1020Sensor::EnrolledFingerprint *fp,
                                               int steps_remaining,
                                               void *data)
{
    struct fingerprint_device *dev = (struct fingerprint_device *) data;
    fingerprint_notify_t notify = fingerprint_get_notify(dev);
    if (notify) {
        fingerprint_msg_t msg;
        msg.type = FINGERPRINT_TEMPLATE_ENROLLING;
        msg.data.enroll.finger.fid = fp ? fp->fid : 0;
        msg.data.enroll.finger.gid = fp ? fp->gid : 0;
        msg.data.enroll.samples_remaining = steps_remaining;
        msg.data.enroll.msg = 0;
        notify(&msg);
    }
}

static void fingerprint_cb_authenticate(const Fpc1020Sensor::EnrolledFingerprint *fp,
                                        uint32_t user_id,
                                        void *data)
{
    fpc1020_device_t *device = (fpc1020_device_t *) data;
    fingerprint_notify_t notify = fingerprint_get_notify(&device->device);
    if (notify && (!fp || device->gid == 0 || device->gid == fp->gid)) {
        fingerprint_msg_t msg;
        msg.type = FINGERPRINT_AUTHENTICATED;
        msg.data.authenticated.finger.gid = fp ? fp->gid : 0;
        msg.data.authenticated.finger.fid = fp ? fp->fid : 0;
        msg.data.authenticated.hat.version = HW_AUTH_TOKEN_VERSION;
        msg.data.authenticated.hat.authenticator_type = htobe32(HW_AUTH_FINGERPRINT);
        msg.data.authenticated.hat.challenge = device->operation_id;
        msg.data.authenticated.hat.authenticator_id = device->impl->getAuthenticatorId();
        msg.data.authenticated.hat.user_id = user_id;

        notify(&msg);
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
        notify(&msg);
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
    dev->device.common.version = HARDWARE_MODULE_API_VERSION(2, 0);
    dev->device.common.module = (struct hw_module_t*) module;
    dev->device.common.close = fingerprint_close;

    dev->device.authenticate = fingerprint_authenticate;
    dev->device.cancel = fingerprint_cancel;
    dev->device.pre_enroll = fingerprint_pre_enroll;
    dev->device.enroll = fingerprint_enroll;
    dev->device.post_enroll = fingerprint_post_enroll;
    dev->device.get_authenticator_id = fingerprint_get_auth_id;
    dev->device.set_active_group = fingerprint_set_active_group;
    dev->device.enumerate = fingerprint_enumerate;
    dev->device.remove = fingerprint_remove;
    dev->device.set_notify = set_notify_callback;
    dev->device.notify = NULL;

    dev->gid = 0;
    dev->operation_id = 0;
    dev->challenge = get_64bit_rand();

    *device = (hw_device_t *) &dev->device;

    return 0;
}

static struct hw_module_methods_t fingerprint_module_methods = {
    .open = fingerprint_open,
};

fingerprint_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag                = HARDWARE_MODULE_TAG,
        .module_api_version = FINGERPRINT_MODULE_API_VERSION_2_0,
        .hal_api_version    = HARDWARE_HAL_API_VERSION,
        .id                 = FINGERPRINT_HARDWARE_MODULE_ID,
        .name               = "FPC1020 Fingerprint HAL",
        .author             = "The CyanogenMod Project",
        .methods            = &fingerprint_module_methods,
        .dso                = NULL,
        .reserved           = { 0 }
    }
};

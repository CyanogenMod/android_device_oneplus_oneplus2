/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_HARDWARE_ICAMERASERVICEPROXY_H
#define ANDROID_HARDWARE_ICAMERASERVICEPROXY_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace android {

/**
 * Interface from native camera service to managed-side camera service proxy.
 *
 * Keep in sync with frameworks/base/core/java/android/hardware/ICameraServiceProxy.aidl
 *
 */
class ICameraServiceProxy : public IInterface {
public:
    enum {
        PING_FOR_USER_UPDATE = IBinder::FIRST_CALL_TRANSACTION,
        NOTIFY_CAMERA_STATE
    };

    enum CameraState {
        CAMERA_STATE_OPEN,
        CAMERA_STATE_ACTIVE,
        CAMERA_STATE_IDLE,
        CAMERA_STATE_CLOSED
    };

    DECLARE_META_INTERFACE(CameraServiceProxy);

    virtual void pingForUserUpdate() = 0;
    virtual void notifyCameraState(String16 cameraId, CameraState newCameraState) = 0;
};

class BnCameraServiceProxy: public BnInterface<ICameraServiceProxy>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};



}; // namespace android

#endif // ANDROID_HARDWARE_ICAMERASERVICEPROXY_H

/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_PHOTOGRAPHY_CALLBACKS_H
#define ANDROID_HARDWARE_PHOTOGRAPHY_CALLBACKS_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include <utils/Timers.h>
#include <system/camera.h>

#include <camera/CaptureResult.h>

namespace android {
class CameraMetadata;


class ICameraDeviceCallbacks : public IInterface
{
    /**
     * Keep up-to-date with ICameraDeviceCallbacks.aidl in frameworks/base
     */
public:
    DECLARE_META_INTERFACE(CameraDeviceCallbacks);

    /**
     * Error codes for CAMERA_MSG_ERROR
     */
    enum CameraErrorCode {
        ERROR_CAMERA_INVALID_ERROR = -1, // To indicate all invalid error codes
        ERROR_CAMERA_DISCONNECTED = 0,
        ERROR_CAMERA_DEVICE = 1,
        ERROR_CAMERA_SERVICE = 2,
        ERROR_CAMERA_REQUEST = 3,
        ERROR_CAMERA_RESULT = 4,
        ERROR_CAMERA_BUFFER = 5,
    };

    // One way
    virtual void            onDeviceError(CameraErrorCode errorCode,
                                          const CaptureResultExtras& resultExtras) = 0;

    // One way
    virtual void            onDeviceIdle() = 0;

    // One way
    virtual void            onCaptureStarted(const CaptureResultExtras& resultExtras,
                                             int64_t timestamp) = 0;

    // One way
    virtual void            onResultReceived(const CameraMetadata& metadata,
                                             const CaptureResultExtras& resultExtras) = 0;

    // One way
    virtual void            onPrepared(int streamId) = 0;
};

// ----------------------------------------------------------------------------

class BnCameraDeviceCallbacks : public BnInterface<ICameraDeviceCallbacks>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace android

#endif

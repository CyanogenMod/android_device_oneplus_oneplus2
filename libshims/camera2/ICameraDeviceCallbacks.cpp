/*
**
** Copyright 2013, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

//#define LOG_NDEBUG 0
#define LOG_TAG "ICameraDeviceCallbacks"
#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <gui/IGraphicBufferProducer.h>
#include <gui/Surface.h>
#include <utils/Mutex.h>

#include <camera/camera2/ICameraDeviceCallbacks.h>
#include "camera/CameraMetadata.h"
#include "camera/CaptureResult.h"

namespace android {

enum {
    CAMERA_ERROR = IBinder::FIRST_CALL_TRANSACTION,
    CAMERA_IDLE,
    CAPTURE_STARTED,
    RESULT_RECEIVED,
    PREPARED
};

class BpCameraDeviceCallbacks: public BpInterface<ICameraDeviceCallbacks>
{
public:
    BpCameraDeviceCallbacks(const sp<IBinder>& impl)
        : BpInterface<ICameraDeviceCallbacks>(impl)
    {
    }

    void onDeviceError(CameraErrorCode errorCode, const CaptureResultExtras& resultExtras)
    {
        ALOGV("onDeviceError");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceCallbacks::getInterfaceDescriptor());
        data.writeInt32(static_cast<int32_t>(errorCode));
        data.writeInt32(1); // to mark presence of CaptureResultExtras object
        resultExtras.writeToParcel(&data);
        remote()->transact(CAMERA_ERROR, data, &reply, IBinder::FLAG_ONEWAY);
        data.writeNoException();
    }

    void onDeviceIdle()
    {
        ALOGV("onDeviceIdle");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceCallbacks::getInterfaceDescriptor());
        remote()->transact(CAMERA_IDLE, data, &reply, IBinder::FLAG_ONEWAY);
        data.writeNoException();
    }

    void onCaptureStarted(const CaptureResultExtras& result, int64_t timestamp)
    {
        ALOGV("onCaptureStarted");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceCallbacks::getInterfaceDescriptor());
        data.writeInt32(1); // to mark presence of CaptureResultExtras object
        result.writeToParcel(&data);
        data.writeInt64(timestamp);
        remote()->transact(CAPTURE_STARTED, data, &reply, IBinder::FLAG_ONEWAY);
        data.writeNoException();
    }

    void onResultReceived(const CameraMetadata& metadata,
            const CaptureResultExtras& resultExtras) {
        ALOGV("onResultReceived");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceCallbacks::getInterfaceDescriptor());
        data.writeInt32(1); // to mark presence of metadata object
        metadata.writeToParcel(&data);
        data.writeInt32(1); // to mark presence of CaptureResult object
        resultExtras.writeToParcel(&data);
        remote()->transact(RESULT_RECEIVED, data, &reply, IBinder::FLAG_ONEWAY);
        data.writeNoException();
    }

    void onPrepared(int streamId)
    {
        ALOGV("onPrepared");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceCallbacks::getInterfaceDescriptor());
        data.writeInt32(streamId);
        remote()->transact(PREPARED, data, &reply, IBinder::FLAG_ONEWAY);
        data.writeNoException();
    }

};

IMPLEMENT_META_INTERFACE(CameraDeviceCallbacks,
                         "android.hardware.camera2.ICameraDeviceCallbacks");

// ----------------------------------------------------------------------

status_t BnCameraDeviceCallbacks::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    ALOGV("onTransact - code = %d", code);
    switch(code) {
        case CAMERA_ERROR: {
            ALOGV("onDeviceError");
            CHECK_INTERFACE(ICameraDeviceCallbacks, data, reply);
            CameraErrorCode errorCode =
                    static_cast<CameraErrorCode>(data.readInt32());
            CaptureResultExtras resultExtras;
            if (data.readInt32() != 0) {
                resultExtras.readFromParcel(const_cast<Parcel*>(&data));
            } else {
                ALOGE("No CaptureResultExtras object is present!");
            }
            onDeviceError(errorCode, resultExtras);
            data.readExceptionCode();
            return NO_ERROR;
        } break;
        case CAMERA_IDLE: {
            ALOGV("onDeviceIdle");
            CHECK_INTERFACE(ICameraDeviceCallbacks, data, reply);
            onDeviceIdle();
            data.readExceptionCode();
            return NO_ERROR;
        } break;
        case CAPTURE_STARTED: {
            ALOGV("onCaptureStarted");
            CHECK_INTERFACE(ICameraDeviceCallbacks, data, reply);
            CaptureResultExtras result;
            if (data.readInt32() != 0) {
                result.readFromParcel(const_cast<Parcel*>(&data));
            } else {
                ALOGE("No CaptureResultExtras object is present in result!");
            }
            int64_t timestamp = data.readInt64();
            onCaptureStarted(result, timestamp);
            data.readExceptionCode();
            return NO_ERROR;
        } break;
        case RESULT_RECEIVED: {
            ALOGV("onResultReceived");
            CHECK_INTERFACE(ICameraDeviceCallbacks, data, reply);
            CameraMetadata metadata;
            if (data.readInt32() != 0) {
                metadata.readFromParcel(const_cast<Parcel*>(&data));
            } else {
                ALOGW("No metadata object is present in result");
            }
            CaptureResultExtras resultExtras;
            if (data.readInt32() != 0) {
                resultExtras.readFromParcel(const_cast<Parcel*>(&data));
            } else {
                ALOGW("No capture result extras object is present in result");
            }
            onResultReceived(metadata, resultExtras);
            data.readExceptionCode();
            return NO_ERROR;
        } break;
        case PREPARED: {
            ALOGV("onPrepared");
            CHECK_INTERFACE(ICameraDeviceCallbacks, data, reply);
            CaptureResultExtras result;
            int streamId = data.readInt32();
            onPrepared(streamId);
            data.readExceptionCode();
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace android

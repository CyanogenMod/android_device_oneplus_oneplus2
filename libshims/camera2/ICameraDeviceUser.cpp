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

// #define LOG_NDEBUG 0
#define LOG_TAG "ICameraDeviceUser"
#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>
#include <binder/Parcel.h>
#include <camera/camera2/ICameraDeviceUser.h>
#include <gui/IGraphicBufferProducer.h>
#include <gui/Surface.h>
#include <camera/CameraMetadata.h>
#include <camera/camera2/CaptureRequest.h>
#include <camera/camera2/OutputConfiguration.h>

namespace android {

typedef Parcel::WritableBlob WritableBlob;
typedef Parcel::ReadableBlob ReadableBlob;

enum {
    DISCONNECT = IBinder::FIRST_CALL_TRANSACTION,
    SUBMIT_REQUEST,
    SUBMIT_REQUEST_LIST,
    CANCEL_REQUEST,
    BEGIN_CONFIGURE,
    END_CONFIGURE,
    DELETE_STREAM,
    CREATE_STREAM,
    CREATE_INPUT_STREAM,
    GET_INPUT_SURFACE,
    CREATE_DEFAULT_REQUEST,
    GET_CAMERA_INFO,
    WAIT_UNTIL_IDLE,
    FLUSH,
    PREPARE,
    TEAR_DOWN,
    PREPARE2
};

namespace {
    // Read empty strings without printing a false error message.
    String16 readMaybeEmptyString16(const Parcel& parcel) {
        size_t len;
        const char16_t* str = parcel.readString16Inplace(&len);
        if (str != NULL) {
            return String16(str, len);
        } else {
            return String16();
        }
    }
};

class BpCameraDeviceUser : public BpInterface<ICameraDeviceUser>
{
public:
    BpCameraDeviceUser(const sp<IBinder>& impl)
        : BpInterface<ICameraDeviceUser>(impl)
    {
    }

    // disconnect from camera service
    void disconnect()
    {
        ALOGV("disconnect");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceUser::getInterfaceDescriptor());
        remote()->transact(DISCONNECT, data, &reply);
        reply.readExceptionCode();
    }

    virtual int submitRequest(sp<CaptureRequest> request, bool repeating,
                              int64_t *lastFrameNumber)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceUser::getInterfaceDescriptor());

        // arg0 = CaptureRequest
        if (request != 0) {
            data.writeInt32(1);
            request->writeToParcel(&data);
        } else {
            data.writeInt32(0);
        }

        // arg1 = streaming (bool)
        data.writeInt32(repeating);

        remote()->transact(SUBMIT_REQUEST, data, &reply);

        reply.readExceptionCode();
        status_t res = reply.readInt32();

        status_t resFrameNumber = BAD_VALUE;
        if (reply.readInt32() != 0) {
            if (lastFrameNumber != NULL) {
                resFrameNumber = reply.readInt64(lastFrameNumber);
            }
        }

        if (res < 0 || (resFrameNumber != NO_ERROR)) {
            res = FAILED_TRANSACTION;
        }
        return res;
    }

    virtual int submitRequestList(List<sp<CaptureRequest> > requestList, bool repeating,
                                  int64_t *lastFrameNumber)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceUser::getInterfaceDescriptor());

        data.writeInt32(requestList.size());

        for (List<sp<CaptureRequest> >::iterator it = requestList.begin();
                it != requestList.end(); ++it) {
            sp<CaptureRequest> request = *it;
            if (request != 0) {
                data.writeInt32(1);
                if (request->writeToParcel(&data) != OK) {
                    return BAD_VALUE;
                }
            } else {
                data.writeInt32(0);
            }
        }

        data.writeInt32(repeating);

        remote()->transact(SUBMIT_REQUEST_LIST, data, &reply);

        reply.readExceptionCode();
        status_t res = reply.readInt32();

        status_t resFrameNumber = BAD_VALUE;
        if (reply.readInt32() != 0) {
            if (lastFrameNumber != NULL) {
                resFrameNumber = reply.readInt64(lastFrameNumber);
            }
        }
        if (res < 0 || (resFrameNumber != NO_ERROR)) {
            res = FAILED_TRANSACTION;
        }
        return res;
    }

    virtual status_t cancelRequest(int requestId, int64_t *lastFrameNumber)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceUser::getInterfaceDescriptor());
        data.writeInt32(requestId);

        remote()->transact(CANCEL_REQUEST, data, &reply);

        reply.readExceptionCode();
        status_t res = reply.readInt32();

        status_t resFrameNumber = BAD_VALUE;
        if (reply.readInt32() != 0) {
            if (lastFrameNumber != NULL) {
                resFrameNumber = reply.readInt64(lastFrameNumber);
            }
        }
        if ((res != NO_ERROR) || (resFrameNumber != NO_ERROR)) {
            res = FAILED_TRANSACTION;
        }
        return res;
    }

    virtual status_t beginConfigure()
    {
        ALOGV("beginConfigure");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceUser::getInterfaceDescriptor());
        remote()->transact(BEGIN_CONFIGURE, data, &reply);
        reply.readExceptionCode();
        return reply.readInt32();
    }

    virtual status_t endConfigure(bool isConstrainedHighSpeed)
    {
        ALOGV("endConfigure");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceUser::getInterfaceDescriptor());
        data.writeInt32(isConstrainedHighSpeed);

        remote()->transact(END_CONFIGURE, data, &reply);
        reply.readExceptionCode();
        return reply.readInt32();
    }

    virtual status_t deleteStream(int streamId)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceUser::getInterfaceDescriptor());
        data.writeInt32(streamId);

        remote()->transact(DELETE_STREAM, data, &reply);

        reply.readExceptionCode();
        return reply.readInt32();
    }

    virtual status_t createStream(const OutputConfiguration& outputConfiguration)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceUser::getInterfaceDescriptor());
        if (outputConfiguration.getGraphicBufferProducer() != NULL) {
            data.writeInt32(1); // marker that OutputConfiguration is not null. Mimic aidl behavior
            outputConfiguration.writeToParcel(data);
        } else {
            data.writeInt32(0);
        }
        remote()->transact(CREATE_STREAM, data, &reply);

        reply.readExceptionCode();
        return reply.readInt32();
    }

    virtual status_t createInputStream(int width, int height, int format)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceUser::getInterfaceDescriptor());
        data.writeInt32(width);
        data.writeInt32(height);
        data.writeInt32(format);

        remote()->transact(CREATE_INPUT_STREAM, data, &reply);

        reply.readExceptionCode();
        return reply.readInt32();
    }

    // get the buffer producer of the input stream
    virtual status_t getInputBufferProducer(
            sp<IGraphicBufferProducer> *producer) {
        if (producer == NULL) {
            return BAD_VALUE;
        }

        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceUser::getInterfaceDescriptor());

        remote()->transact(GET_INPUT_SURFACE, data, &reply);

        reply.readExceptionCode();
        status_t result = reply.readInt32() ;
        if (result != OK) {
            return result;
        }

        sp<IGraphicBufferProducer> bp = NULL;
        if (reply.readInt32() != 0) {
            String16 name = readMaybeEmptyString16(reply);
            bp = interface_cast<IGraphicBufferProducer>(
                    reply.readStrongBinder());
        }

        *producer = bp;

        return *producer == NULL ? INVALID_OPERATION : OK;
    }

    // Create a request object from a template.
    virtual status_t createDefaultRequest(int templateId,
                                          /*out*/
                                          CameraMetadata* request)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceUser::getInterfaceDescriptor());
        data.writeInt32(templateId);
        remote()->transact(CREATE_DEFAULT_REQUEST, data, &reply);

        reply.readExceptionCode();
        status_t result = reply.readInt32();

        CameraMetadata out;
        if (reply.readInt32() != 0) {
            out.readFromParcel(&reply);
        }

        if (request != NULL) {
            request->swap(out);
        }
        return result;
    }


    virtual status_t getCameraInfo(CameraMetadata* info)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceUser::getInterfaceDescriptor());
        remote()->transact(GET_CAMERA_INFO, data, &reply);

        reply.readExceptionCode();
        status_t result = reply.readInt32();

        CameraMetadata out;
        if (reply.readInt32() != 0) {
            out.readFromParcel(&reply);
        }

        if (info != NULL) {
            info->swap(out);
        }

        return result;
    }

    virtual status_t waitUntilIdle()
    {
        ALOGV("waitUntilIdle");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceUser::getInterfaceDescriptor());
        remote()->transact(WAIT_UNTIL_IDLE, data, &reply);
        reply.readExceptionCode();
        return reply.readInt32();
    }

    virtual status_t flush(int64_t *lastFrameNumber)
    {
        ALOGV("flush");
        Parcel data, reply;
        data.writeInterfaceToken(ICameraDeviceUser::getInterfaceDescriptor());
        remote()->transact(FLUSH, data, &reply);
        reply.readExceptionCode();
        status_t res = reply.readInt32();

        status_t resFrameNumber = BAD_VALUE;
        if (reply.readInt32() != 0) {
            if (lastFrameNumber != NULL) {
                resFrameNumber = reply.readInt64(lastFrameNumber);
            }
        }
        if ((res != NO_ERROR) || (resFrameNumber != NO_ERROR)) {
            res = FAILED_TRANSACTION;
        }
        return res;
    }

    virtual status_t prepare(int streamId)
    {
        ALOGV("prepare");
        Parcel data, reply;

        data.writeInterfaceToken(ICameraDeviceUser::getInterfaceDescriptor());
        data.writeInt32(streamId);

        remote()->transact(PREPARE, data, &reply);

        reply.readExceptionCode();
        return reply.readInt32();
    }

    virtual status_t prepare2(int maxCount, int streamId)
    {
        ALOGV("prepare2");
        Parcel data, reply;

        data.writeInterfaceToken(ICameraDeviceUser::getInterfaceDescriptor());
        data.writeInt32(maxCount);
        data.writeInt32(streamId);

        remote()->transact(PREPARE2, data, &reply);

        reply.readExceptionCode();
        return reply.readInt32();
    }

    virtual status_t tearDown(int streamId)
    {
        ALOGV("tearDown");
        Parcel data, reply;

        data.writeInterfaceToken(ICameraDeviceUser::getInterfaceDescriptor());
        data.writeInt32(streamId);

        remote()->transact(TEAR_DOWN, data, &reply);

        reply.readExceptionCode();
        return reply.readInt32();
    }

private:


};

IMPLEMENT_META_INTERFACE(CameraDeviceUser,
                         "android.hardware.camera2.ICameraDeviceUser");

// ----------------------------------------------------------------------

status_t BnCameraDeviceUser::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case DISCONNECT: {
            ALOGV("DISCONNECT");
            CHECK_INTERFACE(ICameraDeviceUser, data, reply);
            disconnect();
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case SUBMIT_REQUEST: {
            CHECK_INTERFACE(ICameraDeviceUser, data, reply);

            // arg0 = request
            sp<CaptureRequest> request;
            if (data.readInt32() != 0) {
                request = new CaptureRequest();
                request->readFromParcel(const_cast<Parcel*>(&data));
            }

            // arg1 = streaming (bool)
            bool repeating = data.readInt32();

            // return code: requestId (int32)
            reply->writeNoException();
            int64_t lastFrameNumber = -1;
            reply->writeInt32(submitRequest(request, repeating, &lastFrameNumber));
            reply->writeInt32(1);
            reply->writeInt64(lastFrameNumber);

            return NO_ERROR;
        } break;
        case SUBMIT_REQUEST_LIST: {
            CHECK_INTERFACE(ICameraDeviceUser, data, reply);

            List<sp<CaptureRequest> > requestList;
            int requestListSize = data.readInt32();
            for (int i = 0; i < requestListSize; i++) {
                if (data.readInt32() != 0) {
                    sp<CaptureRequest> request = new CaptureRequest();
                    if (request->readFromParcel(const_cast<Parcel*>(&data)) != OK) {
                        return BAD_VALUE;
                    }
                    requestList.push_back(request);
                } else {
                    sp<CaptureRequest> request = 0;
                    requestList.push_back(request);
                    ALOGE("A request is missing. Sending in null request.");
                }
            }

            bool repeating = data.readInt32();

            reply->writeNoException();
            int64_t lastFrameNumber = -1;
            reply->writeInt32(submitRequestList(requestList, repeating, &lastFrameNumber));
            reply->writeInt32(1);
            reply->writeInt64(lastFrameNumber);

            return NO_ERROR;
        } break;
        case CANCEL_REQUEST: {
            CHECK_INTERFACE(ICameraDeviceUser, data, reply);
            int requestId = data.readInt32();
            reply->writeNoException();
            int64_t lastFrameNumber = -1;
            reply->writeInt32(cancelRequest(requestId, &lastFrameNumber));
            reply->writeInt32(1);
            reply->writeInt64(lastFrameNumber);
            return NO_ERROR;
        } break;
        case DELETE_STREAM: {
            CHECK_INTERFACE(ICameraDeviceUser, data, reply);
            int streamId = data.readInt32();
            reply->writeNoException();
            reply->writeInt32(deleteStream(streamId));
            return NO_ERROR;
        } break;
        case CREATE_STREAM: {
            CHECK_INTERFACE(ICameraDeviceUser, data, reply);

            status_t ret = BAD_VALUE;
            if (data.readInt32() != 0) {
                OutputConfiguration outputConfiguration(data);
                ret = createStream(outputConfiguration);
            } else {
                ALOGE("%s: cannot take an empty OutputConfiguration", __FUNCTION__);
            }

            reply->writeNoException();
            ALOGV("%s: CREATE_STREAM: write noException", __FUNCTION__);
            reply->writeInt32(ret);
            ALOGV("%s: CREATE_STREAM: write ret = %d", __FUNCTION__, ret);

            return NO_ERROR;
        } break;
        case CREATE_INPUT_STREAM: {
            CHECK_INTERFACE(ICameraDeviceUser, data, reply);
            int width, height, format;

            width = data.readInt32();
            height = data.readInt32();
            format = data.readInt32();
            status_t ret = createInputStream(width, height, format);

            reply->writeNoException();
            reply->writeInt32(ret);
            return NO_ERROR;

        } break;
        case GET_INPUT_SURFACE: {
            CHECK_INTERFACE(ICameraDeviceUser, data, reply);

            sp<IGraphicBufferProducer> bp;
            status_t ret = getInputBufferProducer(&bp);
            sp<IBinder> b(IInterface::asBinder(ret == OK ? bp : NULL));

            reply->writeNoException();
            reply->writeInt32(ret);
            reply->writeInt32(1);
            reply->writeString16(String16("camera input")); // name of surface
            reply->writeStrongBinder(b);

            return NO_ERROR;
        } break;
        case CREATE_DEFAULT_REQUEST: {
            CHECK_INTERFACE(ICameraDeviceUser, data, reply);

            int templateId = data.readInt32();

            CameraMetadata request;
            status_t ret;
            ret = createDefaultRequest(templateId, &request);

            reply->writeNoException();
            reply->writeInt32(ret);

            // out-variables are after exception and return value
            reply->writeInt32(1); // to mark presence of metadata object
            request.writeToParcel(const_cast<Parcel*>(reply));

            return NO_ERROR;
        } break;
        case GET_CAMERA_INFO: {
            CHECK_INTERFACE(ICameraDeviceUser, data, reply);

            CameraMetadata info;
            status_t ret;
            ret = getCameraInfo(&info);

            reply->writeNoException();
            reply->writeInt32(ret);

            // out-variables are after exception and return value
            reply->writeInt32(1); // to mark presence of metadata object
            info.writeToParcel(reply);

            return NO_ERROR;
        } break;
        case WAIT_UNTIL_IDLE: {
            CHECK_INTERFACE(ICameraDeviceUser, data, reply);
            reply->writeNoException();
            reply->writeInt32(waitUntilIdle());
            return NO_ERROR;
        } break;
        case FLUSH: {
            CHECK_INTERFACE(ICameraDeviceUser, data, reply);
            reply->writeNoException();
            int64_t lastFrameNumber = -1;
            reply->writeInt32(flush(&lastFrameNumber));
            reply->writeInt32(1);
            reply->writeInt64(lastFrameNumber);
            return NO_ERROR;
        }
        case BEGIN_CONFIGURE: {
            CHECK_INTERFACE(ICameraDeviceUser, data, reply);
            reply->writeNoException();
            reply->writeInt32(beginConfigure());
            return NO_ERROR;
        } break;
        case END_CONFIGURE: {
            CHECK_INTERFACE(ICameraDeviceUser, data, reply);
            bool isConstrainedHighSpeed = data.readInt32();
            reply->writeNoException();
            reply->writeInt32(endConfigure(isConstrainedHighSpeed));
            return NO_ERROR;
        } break;
        case PREPARE: {
            CHECK_INTERFACE(ICameraDeviceUser, data, reply);
            int streamId = data.readInt32();
            reply->writeNoException();
            reply->writeInt32(prepare(streamId));
            return NO_ERROR;
        } break;
        case TEAR_DOWN: {
            CHECK_INTERFACE(ICameraDeviceUser, data, reply);
            int streamId = data.readInt32();
            reply->writeNoException();
            reply->writeInt32(tearDown(streamId));
            return NO_ERROR;
        } break;
        case PREPARE2: {
            CHECK_INTERFACE(ICameraDeviceUser, data, reply);
            int maxCount = data.readInt32();
            int streamId = data.readInt32();
            reply->writeNoException();
            reply->writeInt32(prepare2(maxCount, streamId));
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace android

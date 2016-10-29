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

#ifndef ANDROID_HARDWARE_PHOTOGRAPHY_ICAMERADEVICEUSER_H
#define ANDROID_HARDWARE_PHOTOGRAPHY_ICAMERADEVICEUSER_H

#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <utils/List.h>

struct camera_metadata;

namespace android {

class ICameraDeviceUserClient;
class IGraphicBufferProducer;
class CaptureRequest;
class CameraMetadata;
class OutputConfiguration;

enum {
    NO_IN_FLIGHT_REPEATING_FRAMES = -1,
};

class ICameraDeviceUser : public IInterface
{
    /**
     * Keep up-to-date with ICameraDeviceUser.aidl in frameworks/base
     */
public:
    DECLARE_META_INTERFACE(CameraDeviceUser);

    virtual void            disconnect() = 0;

    /**
     * Request Handling
     **/

    /**
     * For streaming requests, output lastFrameNumber is the last frame number
     * of the previous repeating request.
     * For non-streaming requests, output lastFrameNumber is the expected last
     * frame number of the current request.
     */
    virtual int             submitRequest(sp<CaptureRequest> request,
                                          bool streaming = false,
                                          /*out*/
                                          int64_t* lastFrameNumber = NULL) = 0;

    /**
     * For streaming requests, output lastFrameNumber is the last frame number
     * of the previous repeating request.
     * For non-streaming requests, output lastFrameNumber is the expected last
     * frame number of the current request.
     */
    virtual int             submitRequestList(List<sp<CaptureRequest> > requestList,
                                              bool streaming = false,
                                              /*out*/
                                              int64_t* lastFrameNumber = NULL) = 0;

    /**
     * Output lastFrameNumber is the last frame number of the previous repeating request.
     */
    virtual status_t        cancelRequest(int requestId,
                                          /*out*/
                                          int64_t* lastFrameNumber = NULL) = 0;

    /**
     * Begin the device configuration.
     *
     * <p>
     * beginConfigure must be called before any call to deleteStream, createStream,
     * or endConfigure.  It is not valid to call this when the device is not idle.
     * <p>
     */
    virtual status_t        beginConfigure() = 0;

    /**
     * End the device configuration.
     *
     * <p>
     * endConfigure must be called after stream configuration is complete (i.e. after
     * a call to beginConfigure and subsequent createStream/deleteStream calls).  This
     * must be called before any requests can be submitted.
     * <p>
     */
    virtual status_t        endConfigure(bool isConstrainedHighSpeed = false) = 0;

    virtual status_t        deleteStream(int streamId) = 0;

    virtual status_t        createStream(const OutputConfiguration& outputConfiguration) = 0;

    /**
     * Create an input stream of width, height, and format (one of
     * HAL_PIXEL_FORMAT_*)
     *
     * Return stream ID if it's a non-negative value. status_t if it's a
     * negative value.
     */
    virtual status_t        createInputStream(int width, int height, int format) = 0;

    // get the buffer producer of the input stream
    virtual status_t        getInputBufferProducer(
            sp<IGraphicBufferProducer> *producer) = 0;

    // Create a request object from a template.
    virtual status_t        createDefaultRequest(int templateId,
                                                 /*out*/
                                                 CameraMetadata* request) = 0;
    // Get static camera metadata
    virtual status_t        getCameraInfo(/*out*/
                                          CameraMetadata* info) = 0;

    // Wait until all the submitted requests have finished processing
    virtual status_t        waitUntilIdle() =  0;

    /**
     * Flush all pending and in-progress work as quickly as possible.
     * Output lastFrameNumber is the last frame number of the previous repeating request.
     */
    virtual status_t        flush(/*out*/
                                  int64_t* lastFrameNumber = NULL) = 0;

    /**
     * Preallocate buffers for a given output stream asynchronously.
     */
    virtual status_t        prepare(int streamId) = 0;

    /**
     * Preallocate up to maxCount buffers for a given output stream asynchronously.
     */
    virtual status_t        prepare2(int maxCount, int streamId) = 0;

    /**
     * Free all unused buffers for a given output stream.
     */
    virtual status_t        tearDown(int streamId) = 0;

};

// ----------------------------------------------------------------------------

class BnCameraDeviceUser: public BnInterface<ICameraDeviceUser>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace android

#endif

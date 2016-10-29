/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_ICAMERASERVICE_H
#define ANDROID_HARDWARE_ICAMERASERVICE_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace android {

class ICamera;
class ICameraClient;
class ICameraServiceListener;
class ICameraDeviceUser;
class ICameraDeviceCallbacks;
class CameraMetadata;
class VendorTagDescriptor;
class String16;

class ICameraService : public IInterface
{
public:
    /**
     * Keep up-to-date with ICameraService.aidl in frameworks/base
     */
    enum {
        GET_NUMBER_OF_CAMERAS = IBinder::FIRST_CALL_TRANSACTION,
        GET_CAMERA_INFO,
        CONNECT,
        CONNECT_DEVICE,
        ADD_LISTENER,
        REMOVE_LISTENER,
        GET_CAMERA_CHARACTERISTICS,
        GET_CAMERA_VENDOR_TAG_DESCRIPTOR,
        GET_LEGACY_PARAMETERS,
        SUPPORTS_CAMERA_API,
        CONNECT_LEGACY,
        SET_TORCH_MODE,
        NOTIFY_SYSTEM_EVENT,
    };

    enum {
        USE_CALLING_UID = -1
    };

    enum {
        API_VERSION_1 = 1,
        API_VERSION_2 = 2,
    };

    enum {
        CAMERA_TYPE_BACKWARD_COMPATIBLE = 0,
        CAMERA_TYPE_ALL = 1,
    };

    enum {
        CAMERA_HAL_API_VERSION_UNSPECIFIED = -1
    };

    /**
     * Keep up-to-date with declarations in
     * frameworks/base/services/core/java/com/android/server/camera/CameraService.java
     *
     * These event codes are intended to be used with the notifySystemEvent call.
     */
    enum {
        NO_EVENT = 0,
        USER_SWITCHED,
    };

public:
    DECLARE_META_INTERFACE(CameraService);

    // Get the number of cameras that support basic color camera operation
    // (type CAMERA_TYPE_BACKWARD_COMPATIBLE)
    virtual int32_t  getNumberOfCameras() = 0;
    // Get the number of cameras of the specified type, one of CAMERA_TYPE_*
    // enums
    virtual int32_t  getNumberOfCameras(int cameraType) = 0;
    virtual status_t getCameraInfo(int cameraId,
            /*out*/
            struct CameraInfo* cameraInfo) = 0;

    virtual status_t getCameraCharacteristics(int cameraId,
            /*out*/
            CameraMetadata* cameraInfo) = 0;

    virtual status_t getCameraVendorTagDescriptor(
            /*out*/
            sp<VendorTagDescriptor>& desc) = 0;

    // Returns 'OK' if operation succeeded
    // - Errors: ALREADY_EXISTS if the listener was already added
    virtual status_t addListener(const sp<ICameraServiceListener>& listener)
                                                                            = 0;
    // Returns 'OK' if operation succeeded
    // - Errors: BAD_VALUE if specified listener was not in the listener list
    virtual status_t removeListener(const sp<ICameraServiceListener>& listener)
                                                                            = 0;
    /**
     * clientPackageName and clientUid are used for permissions checking.  if
     * clientUid == USE_CALLING_UID, then the calling UID is used instead. Only
     * trusted callers can set a clientUid other than USE_CALLING_UID.
     */
    virtual status_t connect(const sp<ICameraClient>& cameraClient,
            int cameraId,
            const String16& clientPackageName,
            int clientUid,
            /*out*/
            sp<ICamera>& device) = 0;

    virtual status_t connectDevice(
            const sp<ICameraDeviceCallbacks>& cameraCb,
            int cameraId,
            const String16& clientPackageName,
            int clientUid,
            /*out*/
            sp<ICameraDeviceUser>& device) = 0;

    virtual status_t getLegacyParameters(
            int cameraId,
            /*out*/
            String16* parameters) = 0;

    /**
     * Returns OK if device supports camera2 api,
     * returns -EOPNOTSUPP if it doesn't.
     */
    virtual status_t supportsCameraApi(
            int cameraId, int apiVersion) = 0;

    /**
     * Connect the device as a legacy device for a given HAL version.
     * For halVersion, use CAMERA_API_DEVICE_VERSION_* for a particular
     * version, or CAMERA_HAL_API_VERSION_UNSPECIFIED for a service-selected version.
     */
    virtual status_t connectLegacy(const sp<ICameraClient>& cameraClient,
            int cameraId, int halVersion,
            const String16& clientPackageName,
            int clientUid,
            /*out*/
            sp<ICamera>& device) = 0;

    /**
     * Turn on or off a camera's torch mode. Torch mode will be turned off by
     * camera service if the lastest client binder that turns it on dies.
     *
     * return values:
     * 0:       on a successful operation.
     * -ENOSYS: the camera device doesn't support this operation. It it returned
     *          if and only if android.flash.into.available is false.
     * -EBUSY:  the camera device is opened.
     * -EINVAL: camera_id is invalid or clientBinder is NULL when enabling a
     *          torch mode.
     */
    virtual status_t setTorchMode(const String16& cameraId, bool enabled,
            const sp<IBinder>& clientBinder) = 0;

    /**
     * Notify the camera service of a system event.  Should only be called from system_server.
     */
    virtual void notifySystemEvent(int32_t eventId, const int32_t* args, size_t length) = 0;
};

// ----------------------------------------------------------------------------

class BnCameraService: public BnInterface<ICameraService>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace android

#endif

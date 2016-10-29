/*
 * Copyright (C) 2014 The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_CAPTURERESULT_H
#define ANDROID_HARDWARE_CAPTURERESULT_H

#include <utils/RefBase.h>
#include <camera/CameraMetadata.h>

namespace android {

/**
 * CaptureResultExtras is a structure to encapsulate various indices for a capture result.
 * These indices are framework-internal and not sent to the HAL.
 */
struct CaptureResultExtras {
    /**
     * An integer to index the request sequence that this result belongs to.
     */
    int32_t requestId;

    /**
     * An integer to index this result inside a request sequence, starting from 0.
     */
    int32_t burstId;

    /**
     * TODO: Add documentation for this field.
     */
    int32_t afTriggerId;

    /**
     * TODO: Add documentation for this field.
     */
    int32_t precaptureTriggerId;

    /**
     * A 64bit integer to index the frame number associated with this result.
     */
    int64_t frameNumber;

    /**
     * The partial result count (index) for this capture result.
     */
    int32_t partialResultCount;

    /**
     * Constructor initializes object as invalid by setting requestId to be -1.
     */
    CaptureResultExtras()
        : requestId(-1),
          burstId(0),
          afTriggerId(0),
          precaptureTriggerId(0),
          frameNumber(0),
          partialResultCount(0) {
    }

    /**
     * This function returns true if it's a valid CaptureResultExtras object.
     * Otherwise, returns false. It is valid only when requestId is non-negative.
     */
    bool isValid();

    status_t                readFromParcel(Parcel* parcel);
    status_t                writeToParcel(Parcel* parcel) const;
};

struct CaptureResult : public virtual LightRefBase<CaptureResult> {
    CameraMetadata          mMetadata;
    CaptureResultExtras     mResultExtras;

    CaptureResult();

    CaptureResult(const CaptureResult& otherResult);

    status_t                readFromParcel(Parcel* parcel);
    status_t                writeToParcel(Parcel* parcel) const;
};

}

#endif /* ANDROID_HARDWARE_CAPTURERESULT_H */

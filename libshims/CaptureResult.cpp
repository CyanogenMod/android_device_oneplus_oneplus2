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

#define LOG_TAG "Camera-CaptureResult"
#include <utils/Log.h>

#include <camera/CaptureResult.h>
#include <binder/Parcel.h>

namespace android {

bool CaptureResultExtras::isValid() {
    return requestId >= 0;
}

status_t CaptureResultExtras::readFromParcel(Parcel *parcel) {
    if (parcel == NULL) {
        ALOGE("%s: Null parcel", __FUNCTION__);
        return BAD_VALUE;
    }

    parcel->readInt32(&requestId);
    parcel->readInt32(&burstId);
    parcel->readInt32(&afTriggerId);
    parcel->readInt32(&precaptureTriggerId);
    parcel->readInt64(&frameNumber);
    parcel->readInt32(&partialResultCount);

    return OK;
}

status_t CaptureResultExtras::writeToParcel(Parcel *parcel) const {
    if (parcel == NULL) {
        ALOGE("%s: Null parcel", __FUNCTION__);
        return BAD_VALUE;
    }

    parcel->writeInt32(requestId);
    parcel->writeInt32(burstId);
    parcel->writeInt32(afTriggerId);
    parcel->writeInt32(precaptureTriggerId);
    parcel->writeInt64(frameNumber);
    parcel->writeInt32(partialResultCount);

    return OK;
}

CaptureResult::CaptureResult() :
        mMetadata(), mResultExtras() {
}

CaptureResult::CaptureResult(const CaptureResult &otherResult) {
    mResultExtras = otherResult.mResultExtras;
    mMetadata = otherResult.mMetadata;
}

status_t CaptureResult::readFromParcel(Parcel *parcel) {

    ALOGV("%s: parcel = %p", __FUNCTION__, parcel);

    if (parcel == NULL) {
        ALOGE("%s: parcel is null", __FUNCTION__);
        return BAD_VALUE;
    }

    mMetadata.clear();

    status_t res = OK;
    res = mMetadata.readFromParcel(parcel);
    if (res != OK) {
        ALOGE("%s: Failed to read metadata from parcel.",
              __FUNCTION__);
        return res;
    }
    ALOGV("%s: Read metadata from parcel", __FUNCTION__);

    res = mResultExtras.readFromParcel(parcel);
    if (res != OK) {
        ALOGE("%s: Failed to read result extras from parcel.",
                __FUNCTION__);
        return res;
    }
    ALOGV("%s: Read result extras from parcel", __FUNCTION__);

    return OK;
}

status_t CaptureResult::writeToParcel(Parcel *parcel) const {

    ALOGV("%s: parcel = %p", __FUNCTION__, parcel);

    if (parcel == NULL) {
        ALOGE("%s: parcel is null", __FUNCTION__);
        return BAD_VALUE;
    }

    status_t res;

    res = mMetadata.writeToParcel(parcel);
    if (res != OK) {
        ALOGE("%s: Failed to write metadata to parcel", __FUNCTION__);
        return res;
    }
    ALOGV("%s: Wrote metadata to parcel", __FUNCTION__);

    res = mResultExtras.writeToParcel(parcel);
    if (res != OK) {
        ALOGE("%s: Failed to write result extras to parcel", __FUNCTION__);
        return res;
    }
    ALOGV("%s: Wrote result extras to parcel", __FUNCTION__);

    return OK;
}

}

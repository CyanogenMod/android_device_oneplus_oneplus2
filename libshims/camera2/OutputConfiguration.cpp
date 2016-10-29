/*
**
** Copyright 2015, The Android Open Source Project
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

#define LOG_TAG "OutputConfiguration"
#include <utils/Log.h>

#include <camera/camera2/OutputConfiguration.h>
#include <binder/Parcel.h>

namespace android {


const int OutputConfiguration::INVALID_ROTATION = -1;

// Read empty strings without printing a false error message.
String16 OutputConfiguration::readMaybeEmptyString16(const Parcel& parcel) {
    size_t len;
    const char16_t* str = parcel.readString16Inplace(&len);
    if (str != NULL) {
        return String16(str, len);
    } else {
        return String16();
    }
}

sp<IGraphicBufferProducer> OutputConfiguration::getGraphicBufferProducer() const {
    return mGbp;
}

int OutputConfiguration::getRotation() const {
    return mRotation;
}

OutputConfiguration::OutputConfiguration(const Parcel& parcel) {
    status_t err;
    int rotation = 0;
    if ((err = parcel.readInt32(&rotation)) != OK) {
        ALOGE("%s: Failed to read rotation from parcel", __FUNCTION__);
        mGbp = NULL;
        mRotation = INVALID_ROTATION;
        return;
    }

    String16 name = readMaybeEmptyString16(parcel);
    const sp<IGraphicBufferProducer>& gbp =
            interface_cast<IGraphicBufferProducer>(parcel.readStrongBinder());
    mGbp = gbp;
    mRotation = rotation;

    ALOGV("%s: OutputConfiguration: bp = %p, name = %s", __FUNCTION__,
          gbp.get(), String8(name).string());
}

OutputConfiguration::OutputConfiguration(sp<IGraphicBufferProducer>& gbp, int rotation) {
    mGbp = gbp;
    mRotation = rotation;
}

status_t OutputConfiguration::writeToParcel(Parcel& parcel) const {

    parcel.writeInt32(mRotation);
    parcel.writeString16(String16("unknown_name")); // name of surface
    sp<IBinder> b(IInterface::asBinder(mGbp));
    parcel.writeStrongBinder(b);

    return OK;
}

}; // namespace android


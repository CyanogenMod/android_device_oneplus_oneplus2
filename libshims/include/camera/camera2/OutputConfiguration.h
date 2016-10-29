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

#ifndef ANDROID_HARDWARE_CAMERA2_OUTPUTCONFIGURATION_H
#define ANDROID_HARDWARE_CAMERA2_OUTPUTCONFIGURATION_H

#include <utils/RefBase.h>
#include <gui/IGraphicBufferProducer.h>

namespace android {

class Surface;

class OutputConfiguration : public virtual RefBase {
public:

    static const int INVALID_ROTATION;
    sp<IGraphicBufferProducer> getGraphicBufferProducer() const;
    int                        getRotation() const;

    /**
     * Keep impl up-to-date with OutputConfiguration.java in frameworks/base
     */
    status_t                   writeToParcel(Parcel& parcel) const;
    // getGraphicBufferProducer will be NULL if error occurred
    // getRotation will be INVALID_ROTATION if error occurred
    OutputConfiguration(const Parcel& parcel);

    OutputConfiguration(sp<IGraphicBufferProducer>& gbp, int rotation);

private:
    sp<IGraphicBufferProducer> mGbp;
    int                        mRotation;

    // helper function
    static String16 readMaybeEmptyString16(const Parcel& parcel);
};
}; // namespace android

#endif

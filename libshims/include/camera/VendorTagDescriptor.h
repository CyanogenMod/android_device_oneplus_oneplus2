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

#ifndef VENDOR_TAG_DESCRIPTOR_H

#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include <utils/RefBase.h>
#include <system/camera_vendor_tags.h>

#include <stdint.h>

namespace android {

class Parcel;

/**
 * VendorTagDescriptor objects are parcelable containers for the vendor tag
 * definitions provided, and are typically used to pass the vendor tag
 * information enumerated by the HAL to clients of the camera service.
 */
class VendorTagDescriptor
        : public LightRefBase<VendorTagDescriptor> {
    public:
        virtual ~VendorTagDescriptor();

        /**
         * The following 'get*' methods implement the corresponding
         * functions defined in
         * system/media/camera/include/system/camera_vendor_tags.h
         */

        // Returns the number of vendor tags defined.
        int getTagCount() const;

        // Returns an array containing the id's of vendor tags defined.
        void getTagArray(uint32_t* tagArray) const;

        // Returns the section name string for a given vendor tag id.
        const char* getSectionName(uint32_t tag) const;

        // Returns the tag name string for a given vendor tag id.
        const char* getTagName(uint32_t tag) const;

        // Returns the tag type for a given vendor tag id.
        int getTagType(uint32_t tag) const;

        /**
         * Write the VendorTagDescriptor object into the given parcel.
         *
         * Returns OK on success, or a negative error code.
         */
        status_t writeToParcel(
                /*out*/
                Parcel* parcel) const;

        /**
         * Convenience method to get a vector containing all vendor tag
         * sections, or an empty vector if none are defined.
         */
        SortedVector<String8> getAllSectionNames() const;

        /**
         * Lookup the tag id for a given tag name and section.
         *
         * Returns OK on success, or a negative error code.
         */
        status_t lookupTag(String8 name, String8 section, /*out*/uint32_t* tag) const;

        /**
         * Dump the currently configured vendor tags to a file descriptor.
         */
        void dump(int fd, int verbosity, int indentation) const;

        // Static methods:

        /**
         * Create a VendorTagDescriptor object from the given parcel.
         *
         * Returns OK on success, or a negative error code.
         */
        static status_t createFromParcel(const Parcel* parcel,
                /*out*/
                sp<VendorTagDescriptor>& descriptor);

        /**
         * Create a VendorTagDescriptor object from the given vendor_tag_ops_t
         * struct.
         *
         * Returns OK on success, or a negative error code.
         */
        static status_t createDescriptorFromOps(const vendor_tag_ops_t* vOps,
                /*out*/
                sp<VendorTagDescriptor>& descriptor);

        /**
         * Sets the global vendor tag descriptor to use for this process.
         * Camera metadata operations that access vendor tags will use the
         * vendor tag definitions set this way.
         *
         * Returns OK on success, or a negative error code.
         */
        static status_t setAsGlobalVendorTagDescriptor(const sp<VendorTagDescriptor>& desc);

        /**
         * Clears the global vendor tag descriptor used by this process.
         */
        static void clearGlobalVendorTagDescriptor();

        /**
         * Returns the global vendor tag descriptor used by this process.
         * This will contain NULL if no vendor tags are defined.
         */
        static sp<VendorTagDescriptor> getGlobalVendorTagDescriptor();
    protected:
        VendorTagDescriptor();
        KeyedVector<String8, KeyedVector<String8, uint32_t>*> mReverseMapping;
        KeyedVector<uint32_t, String8> mTagToNameMap;
        KeyedVector<uint32_t, uint32_t> mTagToSectionMap; // Value is offset in mSections
        KeyedVector<uint32_t, int32_t> mTagToTypeMap;
        SortedVector<String8> mSections;
        // must be int32_t to be compatible with Parcel::writeInt32
        int32_t mTagCount;
    private:
        vendor_tag_ops mVendorOps;
};

} /* namespace android */

#define VENDOR_TAG_DESCRIPTOR_H
#endif /* VENDOR_TAG_DESCRIPTOR_H */

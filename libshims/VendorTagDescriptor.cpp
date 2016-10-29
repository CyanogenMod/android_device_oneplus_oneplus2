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

#define LOG_TAG "VendorTagDescriptor"

#include <binder/Parcel.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/Mutex.h>
#include <utils/Vector.h>
#include <utils/SortedVector.h>
#include <system/camera_metadata.h>
#include <camera_metadata_hidden.h>

#include "camera/VendorTagDescriptor.h"

#include <stdio.h>
#include <string.h>

namespace android {

extern "C" {

static int vendor_tag_descriptor_get_tag_count(const vendor_tag_ops_t* v);
static void vendor_tag_descriptor_get_all_tags(const vendor_tag_ops_t* v, uint32_t* tagArray);
static const char* vendor_tag_descriptor_get_section_name(const vendor_tag_ops_t* v, uint32_t tag);
static const char* vendor_tag_descriptor_get_tag_name(const vendor_tag_ops_t* v, uint32_t tag);
static int vendor_tag_descriptor_get_tag_type(const vendor_tag_ops_t* v, uint32_t tag);

} /* extern "C" */


static Mutex sLock;
static sp<VendorTagDescriptor> sGlobalVendorTagDescriptor;

VendorTagDescriptor::VendorTagDescriptor() {}

VendorTagDescriptor::~VendorTagDescriptor() {
    size_t len = mReverseMapping.size();
    for (size_t i = 0; i < len; ++i)  {
        delete mReverseMapping[i];
    }
}

status_t VendorTagDescriptor::createDescriptorFromOps(const vendor_tag_ops_t* vOps,
            /*out*/
            sp<VendorTagDescriptor>& descriptor) {
    if (vOps == NULL) {
        ALOGE("%s: vendor_tag_ops argument was NULL.", __FUNCTION__);
        return BAD_VALUE;
    }

    int tagCount = vOps->get_tag_count(vOps);
    if (tagCount < 0 || tagCount > INT32_MAX) {
        ALOGE("%s: tag count %d from vendor ops is invalid.", __FUNCTION__, tagCount);
        return BAD_VALUE;
    }

    Vector<uint32_t> tagArray;
    LOG_ALWAYS_FATAL_IF(tagArray.resize(tagCount) != tagCount,
            "%s: too many (%u) vendor tags defined.", __FUNCTION__, tagCount);

    vOps->get_all_tags(vOps, /*out*/tagArray.editArray());

    sp<VendorTagDescriptor> desc = new VendorTagDescriptor();
    desc->mTagCount = tagCount;

    SortedVector<String8> sections;
    KeyedVector<uint32_t, String8> tagToSectionMap;

    for (size_t i = 0; i < static_cast<size_t>(tagCount); ++i) {
        uint32_t tag = tagArray[i];
        if (tag < CAMERA_METADATA_VENDOR_TAG_BOUNDARY) {
            ALOGE("%s: vendor tag %d not in vendor tag section.", __FUNCTION__, tag);
            return BAD_VALUE;
        }
        const char *tagName = vOps->get_tag_name(vOps, tag);
        if (tagName == NULL) {
            ALOGE("%s: no tag name defined for vendor tag %d.", __FUNCTION__, tag);
            return BAD_VALUE;
        }
        desc->mTagToNameMap.add(tag, String8(tagName));
        const char *sectionName = vOps->get_section_name(vOps, tag);
        if (sectionName == NULL) {
            ALOGE("%s: no section name defined for vendor tag %d.", __FUNCTION__, tag);
            return BAD_VALUE;
        }

        String8 sectionString(sectionName);

        sections.add(sectionString);
        tagToSectionMap.add(tag, sectionString);

        int tagType = vOps->get_tag_type(vOps, tag);
        if (tagType < 0 || tagType >= NUM_TYPES) {
            ALOGE("%s: tag type %d from vendor ops does not exist.", __FUNCTION__, tagType);
            return BAD_VALUE;
        }
        desc->mTagToTypeMap.add(tag, tagType);
    }

    desc->mSections = sections;

    for (size_t i = 0; i < static_cast<size_t>(tagCount); ++i) {
        uint32_t tag = tagArray[i];
        String8 sectionString = tagToSectionMap.valueFor(tag);

        // Set up tag to section index map
        ssize_t index = sections.indexOf(sectionString);
        LOG_ALWAYS_FATAL_IF(index < 0, "index %zd must be non-negative", index);
        desc->mTagToSectionMap.add(tag, static_cast<uint32_t>(index));

        // Set up reverse mapping
        ssize_t reverseIndex = -1;
        if ((reverseIndex = desc->mReverseMapping.indexOfKey(sectionString)) < 0) {
            KeyedVector<String8, uint32_t>* nameMapper = new KeyedVector<String8, uint32_t>();
            reverseIndex = desc->mReverseMapping.add(sectionString, nameMapper);
        }
        desc->mReverseMapping[reverseIndex]->add(desc->mTagToNameMap.valueFor(tag), tag);
    }

    descriptor = desc;
    return OK;
}

status_t VendorTagDescriptor::createFromParcel(const Parcel* parcel,
            /*out*/
            sp<VendorTagDescriptor>& descriptor) {
    status_t res = OK;
    if (parcel == NULL) {
        ALOGE("%s: parcel argument was NULL.", __FUNCTION__);
        return BAD_VALUE;
    }

    int32_t tagCount = 0;
    if ((res = parcel->readInt32(&tagCount)) != OK) {
        ALOGE("%s: could not read tag count from parcel", __FUNCTION__);
        return res;
    }

    if (tagCount < 0 || tagCount > INT32_MAX) {
        ALOGE("%s: tag count %d from vendor ops is invalid.", __FUNCTION__, tagCount);
        return BAD_VALUE;
    }

    sp<VendorTagDescriptor> desc = new VendorTagDescriptor();
    desc->mTagCount = tagCount;

    uint32_t tag, sectionIndex;
    uint32_t maxSectionIndex = 0;
    int32_t tagType;
    Vector<uint32_t> allTags;
    for (int32_t i = 0; i < tagCount; ++i) {
        if ((res = parcel->readInt32(reinterpret_cast<int32_t*>(&tag))) != OK) {
            ALOGE("%s: could not read tag id from parcel for index %d", __FUNCTION__, i);
            break;
        }
        if (tag < CAMERA_METADATA_VENDOR_TAG_BOUNDARY) {
            ALOGE("%s: vendor tag %d not in vendor tag section.", __FUNCTION__, tag);
            res = BAD_VALUE;
            break;
        }
        if ((res = parcel->readInt32(&tagType)) != OK) {
            ALOGE("%s: could not read tag type from parcel for tag %d", __FUNCTION__, tag);
            break;
        }
        if (tagType < 0 || tagType >= NUM_TYPES) {
            ALOGE("%s: tag type %d from vendor ops does not exist.", __FUNCTION__, tagType);
            res = BAD_VALUE;
            break;
        }
        String8 tagName = parcel->readString8();
        if (tagName.isEmpty()) {
            ALOGE("%s: parcel tag name was NULL for tag %d.", __FUNCTION__, tag);
            res = NOT_ENOUGH_DATA;
            break;
        }

        if ((res = parcel->readInt32(reinterpret_cast<int32_t*>(&sectionIndex))) != OK) {
            ALOGE("%s: could not read section index for tag %d.", __FUNCTION__, tag);
            break;
        }

        maxSectionIndex = (maxSectionIndex >= sectionIndex) ? maxSectionIndex : sectionIndex;

        allTags.add(tag);
        desc->mTagToNameMap.add(tag, tagName);
        desc->mTagToSectionMap.add(tag, sectionIndex);
        desc->mTagToTypeMap.add(tag, tagType);
    }

    if (res != OK) {
        return res;
    }

    size_t sectionCount = 0;
    if (tagCount > 0) {
        if ((res = parcel->readInt32(reinterpret_cast<int32_t*>(&sectionCount))) != OK) {
            ALOGE("%s: could not read section count for.", __FUNCTION__);
            return res;
        }
        if (sectionCount < (maxSectionIndex + 1)) {
            ALOGE("%s: Incorrect number of sections defined, received %zu, needs %d.",
                    __FUNCTION__, sectionCount, (maxSectionIndex + 1));
            return BAD_VALUE;
        }
        LOG_ALWAYS_FATAL_IF(desc->mSections.setCapacity(sectionCount) <= 0,
                "Vector capacity must be positive");
        for (size_t i = 0; i < sectionCount; ++i) {
            String8 sectionName = parcel->readString8();
            if (sectionName.isEmpty()) {
                ALOGE("%s: parcel section name was NULL for section %zu.",
                      __FUNCTION__, i);
                return NOT_ENOUGH_DATA;
            }
            desc->mSections.add(sectionName);
        }
    }

    LOG_ALWAYS_FATAL_IF(static_cast<size_t>(tagCount) != allTags.size(),
                        "tagCount must be the same as allTags size");
    // Set up reverse mapping
    for (size_t i = 0; i < static_cast<size_t>(tagCount); ++i) {
        uint32_t tag = allTags[i];
        String8 sectionString = desc->mSections[desc->mTagToSectionMap.valueFor(tag)];

        ssize_t reverseIndex = -1;
        if ((reverseIndex = desc->mReverseMapping.indexOfKey(sectionString)) < 0) {
            KeyedVector<String8, uint32_t>* nameMapper = new KeyedVector<String8, uint32_t>();
            reverseIndex = desc->mReverseMapping.add(sectionString, nameMapper);
        }
        desc->mReverseMapping[reverseIndex]->add(desc->mTagToNameMap.valueFor(tag), tag);
    }

    descriptor = desc;
    return res;
}

int VendorTagDescriptor::getTagCount() const {
    size_t size = mTagToNameMap.size();
    if (size == 0) {
        return VENDOR_TAG_COUNT_ERR;
    }
    return size;
}

void VendorTagDescriptor::getTagArray(uint32_t* tagArray) const {
    size_t size = mTagToNameMap.size();
    for (size_t i = 0; i < size; ++i) {
        tagArray[i] = mTagToNameMap.keyAt(i);
    }
}

const char* VendorTagDescriptor::getSectionName(uint32_t tag) const {
    ssize_t index = mTagToSectionMap.indexOfKey(tag);
    if (index < 0) {
        return VENDOR_SECTION_NAME_ERR;
    }
    return mSections[mTagToSectionMap.valueAt(index)].string();
}

const char* VendorTagDescriptor::getTagName(uint32_t tag) const {
    ssize_t index = mTagToNameMap.indexOfKey(tag);
    if (index < 0) {
        return VENDOR_TAG_NAME_ERR;
    }
    return mTagToNameMap.valueAt(index).string();
}

int VendorTagDescriptor::getTagType(uint32_t tag) const {
    ssize_t index = mTagToNameMap.indexOfKey(tag);
    if (index < 0) {
        return VENDOR_TAG_TYPE_ERR;
    }
    return mTagToTypeMap.valueFor(tag);
}

status_t VendorTagDescriptor::writeToParcel(Parcel* parcel) const {
    status_t res = OK;
    if (parcel == NULL) {
        ALOGE("%s: parcel argument was NULL.", __FUNCTION__);
        return BAD_VALUE;
    }

    if ((res = parcel->writeInt32(mTagCount)) != OK) {
        return res;
    }

    size_t size = mTagToNameMap.size();
    uint32_t tag, sectionIndex;
    int32_t tagType;
    for (size_t i = 0; i < size; ++i) {
        tag = mTagToNameMap.keyAt(i);
        String8 tagName = mTagToNameMap[i];
        sectionIndex = mTagToSectionMap.valueFor(tag);
        tagType = mTagToTypeMap.valueFor(tag);
        if ((res = parcel->writeInt32(tag)) != OK) break;
        if ((res = parcel->writeInt32(tagType)) != OK) break;
        if ((res = parcel->writeString8(tagName)) != OK) break;
        if ((res = parcel->writeInt32(sectionIndex)) != OK) break;
    }

    size_t numSections = mSections.size();
    if (numSections > 0) {
        if ((res = parcel->writeInt32(numSections)) != OK) return res;
        for (size_t i = 0; i < numSections; ++i) {
            if ((res = parcel->writeString8(mSections[i])) != OK) return res;
        }
    }

    return res;
}

SortedVector<String8> VendorTagDescriptor::getAllSectionNames() const {
    return mSections;
}

status_t VendorTagDescriptor::lookupTag(String8 name, String8 section, /*out*/uint32_t* tag) const {
    ssize_t index = mReverseMapping.indexOfKey(section);
    if (index < 0) {
        ALOGE("%s: Section '%s' does not exist.", __FUNCTION__, section.string());
        return BAD_VALUE;
    }

    ssize_t nameIndex = mReverseMapping[index]->indexOfKey(name);
    if (nameIndex < 0) {
        ALOGE("%s: Tag name '%s' does not exist.", __FUNCTION__, name.string());
        return BAD_VALUE;
    }

    if (tag != NULL) {
        *tag = mReverseMapping[index]->valueAt(nameIndex);
    }
    return OK;
}

void VendorTagDescriptor::dump(int fd, int verbosity, int indentation) const {

    size_t size = mTagToNameMap.size();
    if (size == 0) {
        dprintf(fd, "%*sDumping configured vendor tag descriptors: None set\n",
                indentation, "");
        return;
    }

    dprintf(fd, "%*sDumping configured vendor tag descriptors: %zu entries\n",
            indentation, "", size);
    for (size_t i = 0; i < size; ++i) {
        uint32_t tag =  mTagToNameMap.keyAt(i);

        if (verbosity < 1) {
            dprintf(fd, "%*s0x%x\n", indentation + 2, "", tag);
            continue;
        }
        String8 name = mTagToNameMap.valueAt(i);
        uint32_t sectionId = mTagToSectionMap.valueFor(tag);
        String8 sectionName = mSections[sectionId];
        int type = mTagToTypeMap.valueFor(tag);
        const char* typeName = (type >= 0 && type < NUM_TYPES) ?
                camera_metadata_type_names[type] : "UNKNOWN";
        dprintf(fd, "%*s0x%x (%s) with type %d (%s) defined in section %s\n", indentation + 2,
            "", tag, name.string(), type, typeName, sectionName.string());
    }

}

status_t VendorTagDescriptor::setAsGlobalVendorTagDescriptor(const sp<VendorTagDescriptor>& desc) {
    status_t res = OK;
    Mutex::Autolock al(sLock);
    sGlobalVendorTagDescriptor = desc;

    vendor_tag_ops_t* opsPtr = NULL;
    if (desc != NULL) {
        opsPtr = &(desc->mVendorOps);
        opsPtr->get_tag_count = vendor_tag_descriptor_get_tag_count;
        opsPtr->get_all_tags = vendor_tag_descriptor_get_all_tags;
        opsPtr->get_section_name = vendor_tag_descriptor_get_section_name;
        opsPtr->get_tag_name = vendor_tag_descriptor_get_tag_name;
        opsPtr->get_tag_type = vendor_tag_descriptor_get_tag_type;
    }
    if((res = set_camera_metadata_vendor_ops(opsPtr)) != OK) {
        ALOGE("%s: Could not set vendor tag descriptor, received error %s (%d)."
                , __FUNCTION__, strerror(-res), res);
    }
    return res;
}

void VendorTagDescriptor::clearGlobalVendorTagDescriptor() {
    Mutex::Autolock al(sLock);
    set_camera_metadata_vendor_ops(NULL);
    sGlobalVendorTagDescriptor.clear();
}

sp<VendorTagDescriptor> VendorTagDescriptor::getGlobalVendorTagDescriptor() {
    Mutex::Autolock al(sLock);
    return sGlobalVendorTagDescriptor;
}

extern "C" {

int vendor_tag_descriptor_get_tag_count(const vendor_tag_ops_t* /*v*/) {
    Mutex::Autolock al(sLock);
    if (sGlobalVendorTagDescriptor == NULL) {
        ALOGE("%s: Vendor tag descriptor not initialized.", __FUNCTION__);
        return VENDOR_TAG_COUNT_ERR;
    }
    return sGlobalVendorTagDescriptor->getTagCount();
}

void vendor_tag_descriptor_get_all_tags(const vendor_tag_ops_t* /*v*/, uint32_t* tagArray) {
    Mutex::Autolock al(sLock);
    if (sGlobalVendorTagDescriptor == NULL) {
        ALOGE("%s: Vendor tag descriptor not initialized.", __FUNCTION__);
        return;
    }
    sGlobalVendorTagDescriptor->getTagArray(tagArray);
}

const char* vendor_tag_descriptor_get_section_name(const vendor_tag_ops_t* /*v*/, uint32_t tag) {
    Mutex::Autolock al(sLock);
    if (sGlobalVendorTagDescriptor == NULL) {
        ALOGE("%s: Vendor tag descriptor not initialized.", __FUNCTION__);
        return VENDOR_SECTION_NAME_ERR;
    }
    return sGlobalVendorTagDescriptor->getSectionName(tag);
}

const char* vendor_tag_descriptor_get_tag_name(const vendor_tag_ops_t* /*v*/, uint32_t tag) {
    Mutex::Autolock al(sLock);
    if (sGlobalVendorTagDescriptor == NULL) {
        ALOGE("%s: Vendor tag descriptor not initialized.", __FUNCTION__);
        return VENDOR_TAG_NAME_ERR;
    }
    return sGlobalVendorTagDescriptor->getTagName(tag);
}

int vendor_tag_descriptor_get_tag_type(const vendor_tag_ops_t* /*v*/, uint32_t tag) {
    Mutex::Autolock al(sLock);
    if (sGlobalVendorTagDescriptor == NULL) {
        ALOGE("%s: Vendor tag descriptor not initialized.", __FUNCTION__);
        return VENDOR_TAG_TYPE_ERR;
    }
    return sGlobalVendorTagDescriptor->getTagType(tag);
}

} /* extern "C" */
} /* namespace android */

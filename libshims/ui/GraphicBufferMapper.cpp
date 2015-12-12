/*
 * Copyright (C) 2007 The Android Open Source Project
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

#define LOG_TAG "GraphicBufferMapper"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <stdint.h>
#include <errno.h>

#include <sync/sync.h>

#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/Trace.h>

#include <ui/GraphicBufferMapper.h>
#include <ui/Rect.h>

#include <hardware/gralloc.h>


namespace android {
// ---------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE( GraphicBufferMapper )

GraphicBufferMapper::GraphicBufferMapper()
    : mAllocMod(0)
{
    hw_module_t const* module;
    int err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);
    ALOGE_IF(err, "FATAL: can't find the %s module", GRALLOC_HARDWARE_MODULE_ID);
    if (err == 0) {
        mAllocMod = (gralloc_module_t const *)module;
    }
}

status_t GraphicBufferMapper::registerBuffer(buffer_handle_t handle)
{
    ATRACE_CALL();
    status_t err;

    err = mAllocMod->registerBuffer(mAllocMod, handle);

    ALOGW_IF(err, "registerBuffer(%p) failed %d (%s)",
            handle, err, strerror(-err));
    return err;
}

status_t GraphicBufferMapper::unregisterBuffer(buffer_handle_t handle)
{
    ATRACE_CALL();
    status_t err;

    err = mAllocMod->unregisterBuffer(mAllocMod, handle);

    ALOGW_IF(err, "unregisterBuffer(%p) failed %d (%s)",
            handle, err, strerror(-err));
    return err;
}

status_t GraphicBufferMapper::lock(buffer_handle_t handle, 
        int usage, const Rect& bounds, void** vaddr)
{
    ATRACE_CALL();
    status_t err;

    err = mAllocMod->lock(mAllocMod, handle, usage,
            bounds.left, bounds.top, bounds.width(), bounds.height(),
            vaddr);

    ALOGW_IF(err, "lock(...) failed %d (%s)", err, strerror(-err));
    return err;
}

status_t GraphicBufferMapper::lockYCbCr(buffer_handle_t handle,
        int usage, const Rect& bounds, android_ycbcr *ycbcr)
{
    ATRACE_CALL();
    status_t err;

    if (mAllocMod->lock_ycbcr == NULL) {
        return -EINVAL; // do not log failure
    }

    err = mAllocMod->lock_ycbcr(mAllocMod, handle, usage,
            bounds.left, bounds.top, bounds.width(), bounds.height(),
            ycbcr);

    ALOGW_IF(err, "lock(...) failed %d (%s)", err, strerror(-err));
    return err;
}

status_t GraphicBufferMapper::unlock(buffer_handle_t handle)
{
    ATRACE_CALL();
    status_t err;

    err = mAllocMod->unlock(mAllocMod, handle);

    ALOGW_IF(err, "unlock(...) failed %d (%s)", err, strerror(-err));
    return err;
}

status_t GraphicBufferMapper::lockAsync(buffer_handle_t handle,
        int usage, const Rect& bounds, void** vaddr, int fenceFd)
{
    ATRACE_CALL();
    status_t err;

    if (mAllocMod->common.module_api_version >= GRALLOC_MODULE_API_VERSION_0_3) {
        err = mAllocMod->lockAsync(mAllocMod, handle, usage,
                bounds.left, bounds.top, bounds.width(), bounds.height(),
                vaddr, fenceFd);
    } else {
        sync_wait(fenceFd, -1);
        close(fenceFd);
        err = mAllocMod->lock(mAllocMod, handle, usage,
                bounds.left, bounds.top, bounds.width(), bounds.height(),
                vaddr);
    }

    ALOGW_IF(err, "lockAsync(...) failed %d (%s)", err, strerror(-err));
    return err;
}

status_t GraphicBufferMapper::lockAsyncYCbCr(buffer_handle_t handle,
        int usage, const Rect& bounds, android_ycbcr *ycbcr, int fenceFd)
{
    ATRACE_CALL();
    status_t err;

    if (mAllocMod->common.module_api_version >= GRALLOC_MODULE_API_VERSION_0_3
            && mAllocMod->lockAsync_ycbcr != NULL) {
        err = mAllocMod->lockAsync_ycbcr(mAllocMod, handle, usage,
                bounds.left, bounds.top, bounds.width(), bounds.height(),
                ycbcr, fenceFd);
    } else if (mAllocMod->lock_ycbcr != NULL) {
        sync_wait(fenceFd, -1);
        close(fenceFd);
        err = mAllocMod->lock_ycbcr(mAllocMod, handle, usage,
                bounds.left, bounds.top, bounds.width(), bounds.height(),
                ycbcr);
    } else {
        return -EINVAL; // do not log failure
    }

    ALOGW_IF(err, "lock(...) failed %d (%s)", err, strerror(-err));
    return err;
}

status_t GraphicBufferMapper::unlockAsync(buffer_handle_t handle, int *fenceFd)
{
    ATRACE_CALL();
    status_t err;

    if (mAllocMod->common.module_api_version >= GRALLOC_MODULE_API_VERSION_0_3) {
        err = mAllocMod->unlockAsync(mAllocMod, handle, fenceFd);
    } else {
        *fenceFd = -1;
        err = mAllocMod->unlock(mAllocMod, handle);
    }

    ALOGW_IF(err, "unlockAsync(...) failed %d (%s)", err, strerror(-err));
    return err;
}

// ---------------------------------------------------------------------------
}; // namespace android

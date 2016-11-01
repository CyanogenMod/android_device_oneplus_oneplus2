/* Copyright (c) 2015, The Linux Foundataion. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __QCAMERA_FLASH_H__
#define __QCAMERA_FLASH_H__

#include <hardware/camera_common.h>

extern "C" {
#include <mm_camera_interface.h>
}

namespace qcamera {

#define QCAMERA_TORCH_CURRENT_VALUE 200

class QCameraFlash {
public:
    static QCameraFlash& getInstance();

    int32_t registerCallbacks(const camera_module_callbacks_t* callbacks);
    int32_t initFlash(const int camera_id);
    int32_t setFlashMode(const int camera_id, const bool on);
    int32_t deinitFlash(const int camera_id);
    int32_t reserveFlashForCamera(const int camera_id);
    int32_t releaseFlashFromCamera(const int camera_id);

private:
    QCameraFlash();
    virtual ~QCameraFlash();
    QCameraFlash(const QCameraFlash&);
    QCameraFlash& operator=(const QCameraFlash&);

    const camera_module_callbacks_t *m_callbacks;
    int32_t m_flashFds[MM_CAMERA_MAX_NUM_SENSORS];
    bool m_flashOn[MM_CAMERA_MAX_NUM_SENSORS];
    bool m_cameraOpen[MM_CAMERA_MAX_NUM_SENSORS];
};

}; // namespace qcamera

#endif /* __QCAMERA_FLASH_H__ */

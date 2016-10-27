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

#include <stdio.h>
#include <fcntl.h>
#include <linux/media.h>
#include <media/msmb_camera.h>
#include <media/msm_cam_sensor.h>
#include <utils/Log.h>

#include "HAL3/QCamera3HWI.h"
#include "QCameraFlash.h"

#define STRING_LENGTH_OF_64_BIT_NUMBER 21

volatile uint32_t gCamHal3LogLevel = 1;

namespace qcamera {

/*===========================================================================
 * FUNCTION   : getInstance
 *
 * DESCRIPTION: Get and create the QCameraFlash singleton.
 *
 * PARAMETERS : None
 *
 * RETURN     : None
 *==========================================================================*/
QCameraFlash& QCameraFlash::getInstance()
{
    static QCameraFlash flashInstance;
    return flashInstance;
}

/*===========================================================================
 * FUNCTION   : QCameraFlash
 *
 * DESCRIPTION: default constructor of QCameraFlash
 *
 * PARAMETERS : None
 *
 * RETURN     : None
 *==========================================================================*/
QCameraFlash::QCameraFlash() : m_callbacks(NULL)
{
    memset(&m_flashOn, 0, sizeof(m_flashOn));
    memset(&m_cameraOpen, 0, sizeof(m_cameraOpen));
    for (int pos = 0; pos < MM_CAMERA_MAX_NUM_SENSORS; pos++) {
        m_flashFds[pos] = -1;
    }
}

/*===========================================================================
 * FUNCTION   : ~QCameraFlash
 *
 * DESCRIPTION: deconstructor of QCameraFlash
 *
 * PARAMETERS : None
 *
 * RETURN     : None
 *==========================================================================*/
QCameraFlash::~QCameraFlash()
{
    for (int pos = 0; pos < MM_CAMERA_MAX_NUM_SENSORS; pos++) {
        if (m_flashFds[pos] >= 0)
            {
                setFlashMode(pos, false);
                close(m_flashFds[pos]);
                m_flashFds[pos] = -1;
            }
    }
}

/*===========================================================================
 * FUNCTION   : registerCallbacks
 *
 * DESCRIPTION: provide flash module with reference to callbacks to framework
 *
 * PARAMETERS : None
 *
 * RETURN     : None
 *==========================================================================*/
int32_t QCameraFlash::registerCallbacks(
        const camera_module_callbacks_t* callbacks)
{
    int32_t retVal = 0;
    m_callbacks = callbacks;
    return retVal;
}

/*===========================================================================
 * FUNCTION   : initFlash
 *
 * DESCRIPTION: Reserve and initialize the flash unit associated with a
 *              given camera id. This function is blocking until the
 *              operation completes or fails. Each flash unit can be "inited"
 *              by only one process at a time.
 *
 * PARAMETERS :
 *   @camera_id : Camera id of the flash.
 *
 * RETURN     :
 *   0        : success
 *   -EBUSY   : The flash unit or the resource needed to turn on the
 *              the flash is busy, typically because the flash is
 *              already in use.
 *   -EINVAL  : No flash present at camera_id.
 *==========================================================================*/
int32_t QCameraFlash::initFlash(const int camera_id)
{
    int32_t retVal = 0;
    bool hasFlash = false;
    char flashNode[QCAMERA_MAX_FILEPATH_LENGTH];
    char flashPath[QCAMERA_MAX_FILEPATH_LENGTH] = "/dev/";

    if (camera_id < 0 || camera_id >= MM_CAMERA_MAX_NUM_SENSORS) {
        ALOGE("%s: Invalid camera id: %d", __func__, camera_id);
        return -EINVAL;
    }

    QCamera3HardwareInterface::getFlashInfo(camera_id,
            hasFlash,
            flashNode);

    strlcat(flashPath,
            flashNode,
            sizeof(flashPath));

    if (!hasFlash) {
        ALOGE("%s: No flash available for camera id: %d",
                __func__,
                camera_id);
        retVal = -EINVAL;
    } else if (m_cameraOpen[camera_id]) {
        ALOGE("%s: Camera in use for camera id: %d",
                __func__,
                camera_id);
        retVal = -EBUSY;
    } else if (m_flashFds[camera_id] >= 0) {
        CDBG("%s: Flash is already inited for camera id: %d",
                __func__,
                camera_id);
    } else {
        m_flashFds[camera_id] = open(flashPath, O_RDWR | O_NONBLOCK);

        if (m_flashFds[camera_id] < 0) {
            ALOGE("%s: Unable to open node '%s'",
                    __func__,
                    flashPath);
            retVal = -EBUSY;
        } else {
            struct msm_flash_cfg_data_t cfg;
            struct msm_flash_init_info_t init_info;
            memset(&cfg, 0, sizeof(struct msm_flash_cfg_data_t));
            memset(&init_info, 0, sizeof(struct msm_flash_init_info_t));
            init_info.flash_driver_type = FLASH_DRIVER_DEFAULT;
            cfg.cfg.flash_init_info = &init_info;
            cfg.cfg_type = CFG_FLASH_INIT;
            retVal = ioctl(m_flashFds[camera_id],
                    VIDIOC_MSM_FLASH_CFG,
                    &cfg);
            if (retVal < 0) {
                ALOGE("%s: Unable to init flash for camera id: %d",
                        __func__,
                        camera_id);
                close(m_flashFds[camera_id]);
                m_flashFds[camera_id] = -1;
            }
        }
    }

    CDBG("%s: X, retVal = %d", __func__, retVal);
    return retVal;
}

/*===========================================================================
 * FUNCTION   : setFlashMode
 *
 * DESCRIPTION: Turn on or off the flash associated with a given handle.
 *              This function is blocking until the operation completes or
 *              fails.
 *
 * PARAMETERS :
 *   @camera_id  : Camera id of the flash
 *   @on         : Whether to turn flash on (true) or off (false)
 *
 * RETURN     :
 *   0        : success
 *   -EINVAL  : No camera present at camera_id, or it is not inited.
 *   -EALREADY: Flash is already in requested state
 *==========================================================================*/
int32_t QCameraFlash::setFlashMode(const int camera_id, const bool mode)
{
    int32_t retVal = 0;
    struct msm_flash_cfg_data_t cfg;

    if (camera_id < 0 || camera_id >= MM_CAMERA_MAX_NUM_SENSORS) {
        ALOGE("%s: Invalid camera id: %d", __func__, camera_id);
        retVal = -EINVAL;
    } else if (mode == m_flashOn[camera_id]) {
        CDBG("%s: flash %d is already in requested state: %d",
                __func__,
                camera_id,
                mode);
        retVal = -EALREADY;
    } else if (m_flashFds[camera_id] < 0) {
        ALOGE("%s: called for uninited flash: %d", __func__, camera_id);
        retVal = -EINVAL;
    }  else {
        memset(&cfg, 0, sizeof(struct msm_flash_cfg_data_t));
        for (int i = 0; i < MAX_LED_TRIGGERS; i++)
            cfg.flash_current[i] = QCAMERA_TORCH_CURRENT_VALUE;
        cfg.cfg_type = mode ? CFG_FLASH_LOW: CFG_FLASH_OFF;

        retVal = ioctl(m_flashFds[camera_id],
                        VIDIOC_MSM_FLASH_CFG,
                        &cfg);
        if (retVal < 0)
            ALOGE("%s: Unable to change flash mode to %d for camera id: %d",
                    __func__, mode, camera_id);
        else
            m_flashOn[camera_id] = mode;
    }
    return retVal;
}

/*===========================================================================
 * FUNCTION   : deinitFlash
 *
 * DESCRIPTION: Release the flash unit associated with a given camera
 *              position. This function is blocking until the operation
 *              completes or fails.
 *
 * PARAMETERS :
 *   @camera_id : Camera id of the flash.
 *
 * RETURN     :
 *   0        : success
 *   -EINVAL  : No camera present at camera_id or not inited.
 *==========================================================================*/
int32_t QCameraFlash::deinitFlash(const int camera_id)
{
    int32_t retVal = 0;

    if (camera_id < 0 || camera_id >= MM_CAMERA_MAX_NUM_SENSORS) {
        ALOGE("%s: Invalid camera id: %d", __func__, camera_id);
        retVal = -EINVAL;
    } else if (m_flashFds[camera_id] < 0) {
        ALOGE("%s: called deinitFlash for uninited flash", __func__);
        retVal = -EINVAL;
    } else {
        setFlashMode(camera_id, false);

        struct msm_flash_cfg_data_t cfg;
        cfg.cfg_type = CFG_FLASH_RELEASE;
        retVal = ioctl(m_flashFds[camera_id],
                VIDIOC_MSM_FLASH_CFG,
                &cfg);
        if (retVal < 0) {
            ALOGE("%s: Failed to release flash for camera id: %d",
                    __func__,
                    camera_id);
        }

        close(m_flashFds[camera_id]);
        m_flashFds[camera_id] = -1;
    }

    return retVal;
}

/*===========================================================================
 * FUNCTION   : reserveFlashForCamera
 *
 * DESCRIPTION: Give control of the flash to the camera, and notify
 *              framework that the flash has become unavailable.
 *
 * PARAMETERS :
 *   @camera_id : Camera id of the flash.
 *
 * RETURN     :
 *   0        : success
 *   -EINVAL  : No camera present at camera_id or not inited.
 *   -ENOSYS  : No callback available for torch_mode_status_change.
 *==========================================================================*/
int32_t QCameraFlash::reserveFlashForCamera(const int camera_id)
{
    int32_t retVal = 0;

    if (camera_id < 0 || camera_id >= MM_CAMERA_MAX_NUM_SENSORS) {
        ALOGE("%s: Invalid camera id: %d", __func__, camera_id);
        retVal = -EINVAL;
    } else if (m_cameraOpen[camera_id]) {
        CDBG("%s: Flash already reserved for camera id: %d",
                __func__,
                camera_id);
    } else {
        if (m_flashOn[camera_id]) {
            setFlashMode(camera_id, false);
            deinitFlash(camera_id);
        }
        m_cameraOpen[camera_id] = true;

        bool hasFlash = false;
        char flashNode[QCAMERA_MAX_FILEPATH_LENGTH];

        QCamera3HardwareInterface::getFlashInfo(camera_id,
                hasFlash,
                flashNode);

        if (m_callbacks == NULL ||
                m_callbacks->torch_mode_status_change == NULL) {
            ALOGE("%s: Callback is not defined!", __func__);
            retVal = -ENOSYS;
        } else if (!hasFlash) {
            CDBG("%s: Suppressing callback "
                    "because no flash exists for camera id: %d",
                    __func__,
                    camera_id);
        } else {
            char cameraIdStr[STRING_LENGTH_OF_64_BIT_NUMBER];
            snprintf(cameraIdStr, STRING_LENGTH_OF_64_BIT_NUMBER,
                    "%d", camera_id);
            m_callbacks->torch_mode_status_change(m_callbacks,
                    cameraIdStr,
                    TORCH_MODE_STATUS_NOT_AVAILABLE);
        }
    }

    return retVal;
}

/*===========================================================================
 * FUNCTION   : releaseFlashFromCamera
 *
 * DESCRIPTION: Release control of the flash from the camera, and notify
 *              framework that the flash has become available.
 *
 * PARAMETERS :
 *   @camera_id : Camera id of the flash.
 *
 * RETURN     :
 *   0        : success
 *   -EINVAL  : No camera present at camera_id or not inited.
 *   -ENOSYS  : No callback available for torch_mode_status_change.
 *==========================================================================*/
int32_t QCameraFlash::releaseFlashFromCamera(const int camera_id)
{
    int32_t retVal = 0;

    if (camera_id < 0 || camera_id >= MM_CAMERA_MAX_NUM_SENSORS) {
        ALOGE("%s: Invalid camera id: %d", __func__, camera_id);
        retVal = -EINVAL;
    } else if (!m_cameraOpen[camera_id]) {
        CDBG("%s: Flash not reserved for camera id: %d",
                __func__,
                camera_id);
    } else {
        m_cameraOpen[camera_id] = false;

        bool hasFlash = false;
        char flashNode[QCAMERA_MAX_FILEPATH_LENGTH];

        QCamera3HardwareInterface::getFlashInfo(camera_id,
                hasFlash,
                flashNode);

        if (m_callbacks == NULL ||
                m_callbacks->torch_mode_status_change == NULL) {
            ALOGE("%s: Callback is not defined!", __func__);
            retVal = -ENOSYS;
        } else if (!hasFlash) {
            CDBG("%s: Suppressing callback "
                    "because no flash exists for camera id: %d",
                    __func__,
                    camera_id);
        } else {
            char cameraIdStr[STRING_LENGTH_OF_64_BIT_NUMBER];
            snprintf(cameraIdStr, STRING_LENGTH_OF_64_BIT_NUMBER,
                    "%d", camera_id);
            m_callbacks->torch_mode_status_change(m_callbacks,
                    cameraIdStr,
                    TORCH_MODE_STATUS_AVAILABLE_OFF);
        }
    }

    return retVal;
}

}; // namespace qcamera

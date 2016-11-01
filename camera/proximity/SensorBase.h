/*
 * Copyright (C) 2014 STMicroelectronics, Inc.
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

#ifndef ANDROID_SENSOR_BASE_H
#define ANDROID_SENSOR_BASE_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <string.h>

//#if defined ANDROID_JELLYBEAN
//build for Jellybean
#define LOGV_IF ALOGV_IF
#define LOGE_IF ALOGE_IF
#define LOGI_IF ALOGI_IF
#define LOGI    ALOGI
#define LOGE    ALOGE
#define LOGV    ALOGV
#define LOGW    ALOGW
//#else
//build for ICS or earlier version
//#endif
/*****************************************************************************/

struct sensors_event_t;

class SensorBase {
protected:
    const char* dev_name;
    const char* data_name;
    char        input_name[PATH_MAX];
    int         dev_fd;
    int         data_fd;

    int openInput(const char* inputName);
    static int64_t getTimestamp();


    static int64_t timevalToNano(timeval const& t) {
        return t.tv_sec*1000000000LL + t.tv_usec*1000;
    }

    int open_device();
    int close_device();

public:
    SensorBase(const char* dev_name, const char* data_name);

    virtual ~SensorBase();

    virtual int readEvents(sensors_event_t* data, int count) = 0;
    virtual bool hasPendingEvents() const;
    virtual int getFd() const;
    virtual int setDelay(int32_t handle, int64_t ns);
    virtual int enable(int32_t handle, int enabled) = 0;
    virtual int query(int what, int* value);
    virtual int batch(int handle, int flags, int64_t period_ns, int64_t timeout);
};

/*****************************************************************************/

#endif  // ANDROID_SENSOR_BASE_H

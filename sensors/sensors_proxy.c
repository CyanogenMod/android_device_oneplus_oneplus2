/* -*-mode:c-*- */
/* Time-stamp: <2016-09-03 16:15:06 dky> */
/*
 * Copyright (c) 2016, The CyanogenMod Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *  * Neither the name of The Linux Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
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
 */

#include <hardware/sensors.h>
#include <dlfcn.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

/* Define LOG_NDEBUG to 0 to enable logging */
#define LOG_NDEBUG 1
#include <cutils/log.h>

/* Base path of the hal modules */
#define HAL_VENDOR_PATH "/vendor/lib64"
#define HAL_SYSTEM_PATH "/system/lib64/hw"

int global_sensors_count = -1;
struct sensor_t* global_sensors_list = 0;
struct hw_module_t* global_sensor_module = 0;
pthread_once_t init_sensors_once = PTHREAD_ONCE_INIT;

static int open_sensors(__unused const struct hw_module_t* hw_module,
                        __unused const char* name,
                        struct hw_device_t** hw_device_out);
static int module__get_sensors_list(__unused struct sensors_module_t* module,
                                    struct sensor_t const** list);

static struct hw_module_methods_t sensors_module_methods = {
    open : open_sensors
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
    common : {
        tag : HARDWARE_MODULE_TAG,
        version_major : 1,
        version_minor : 1,
        id : SENSORS_HARDWARE_MODULE_ID,
        name : "Sensor proxy",
        author : "CyanogenMod",
        methods : &sensors_module_methods,
        dso : NULL,
        reserved : {0}
    },
    get_sensors_list : module__get_sensors_list,
    set_operation_mode : 0
};

/* Initialize the sensor by loading the underlying sensor
 * Called only once in a thread safe manner */
void init_sensors(void) {
    const struct sensor_t* sensors_list = 0;
    int sensors_count = 0;
    void* handle = 0;

    /* Try both paths */
    handle = dlopen(HAL_VENDOR_PATH"/"SENSORS_HARDWARE_MODULE_ID".ssc.so",
                    RTLD_NOW);
    if (!handle) {
        handle = dlopen(HAL_SYSTEM_PATH"/"SENSORS_HARDWARE_MODULE_ID".ssc.so",
                        RTLD_NOW);
    }

    if (!handle) {
        ALOGE("init_sensors: dlopen() failed to load \"sensors.ssc.so\"");
        goto fail;
    }

    global_sensor_module = (struct hw_module_t*)
        dlsym(handle, HAL_MODULE_INFO_SYM_AS_STR);
    if (!global_sensor_module) {
        ALOGE("init_sensors: dlsym() failed to load driver details");
        goto fail;
    }

    if (strcmp(SENSORS_HARDWARE_MODULE_ID, global_sensor_module->id)) {
        ALOGE("init_sensors: Driver ID mismatch, expecting %s but got %s",
              SENSORS_HARDWARE_MODULE_ID, global_sensor_module->id);
        goto fail;
    }

    /* Lazy initialize based on driver providing data */
    HAL_MODULE_INFO_SYM.set_operation_mode =
        ((struct sensors_module_t*)global_sensor_module)->set_operation_mode;

    sensors_count =
        ((struct sensors_module_t*)global_sensor_module)->get_sensors_list(
            (struct sensors_module_t*)global_sensor_module, &sensors_list);

    /* Check if there are any underlying sensors provided */
    if (!(sensors_count > 0)) {
        ALOGE("get_sensors_list() did not return any sensors");
        return;
    }

    global_sensors_list = calloc(sensors_count, sizeof(struct sensor_t));
    if (!global_sensors_list) {
        ALOGE("calloc() failed to allocate memory");
        goto fail;
    }

    /* Let us make sure it is set to 0 to avoid overruns */
    global_sensors_count = 0;
    for (int idx = 0; idx < sensors_count; ++idx) {
        if (SENSOR_TYPE_ROTATION_VECTOR == sensors_list[idx].type) {
            ALOGV("Skipping sensor: SENSOR_TYPE_ROTATION_VECTOR");
            continue;
        }

        memcpy(&global_sensors_list[global_sensors_count],
               &sensors_list[idx], sizeof(struct sensor_t));
        ++global_sensors_count;
    }

    /* Looks good: Update the handle in the module */
    global_sensor_module->dso = handle;

    return;

fail:
    global_sensors_count = 0;
    global_sensor_module = 0;

    if (handle) {
        dlclose(handle);
    }

    if (global_sensors_list) {
        free(global_sensors_list);
        global_sensors_list = 0;
    }

    return;
}

int module__get_sensors_list(__unused struct sensors_module_t* module,
                             struct sensor_t const** list) {
    ALOGV("module__get_sensors_list start");
    pthread_once(&init_sensors_once, init_sensors);

    *list = global_sensors_list;

    ALOGV("global_sensors_count: %d", global_sensors_count);
    for (int i = 0; i < global_sensors_count; ++i) {
        ALOGV("sensor type: %d", global_sensors_list[i].type);
    }

    return (global_sensors_count > 0) ? global_sensors_count : 0;
}

int open_sensors(__unused const struct hw_module_t* hw_module,
                 __unused const char* name,
                 struct hw_device_t** hw_device_out) {
    ALOGV("open_sensors begin...");
    pthread_once(&init_sensors_once, init_sensors);

    if (global_sensor_module) {
        sensors_open(global_sensor_module,
                     (struct sensors_poll_device_t**)hw_device_out);

        /* Hook the current driver to intercept function calls */
        if (*hw_device_out) {
            (*hw_device_out)->module = global_sensor_module;
        }
    }

    ALOGV("...open_sensors end");
    return (*hw_device_out) ? 0 : -EINVAL;
}

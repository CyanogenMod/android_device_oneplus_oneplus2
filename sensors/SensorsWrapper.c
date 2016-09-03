/* Copyright (C) 2016, The CyanogenMod Project
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

#include <hardware/sensors.h>
#include <dlfcn.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include <cutils/log.h>

#ifdef __aarch64__
#define HAL_VENDOR_PATH "/vendor/lib64"
#else
#define HAL_VENDOR_PATH "/vendor/lib"
#endif

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
    .open = open_sensors
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = SENSORS_HARDWARE_MODULE_ID,
        .name = "Sensors wrapper",
        .author = "The CyanogenMod Project",
        .methods = &sensors_module_methods,
        .dso = NULL,
        .reserved = {0}
    },
    .get_sensors_list = module__get_sensors_list,
    .set_operation_mode = 0
};

/* Initialize the sensor by loading the underlying sensor
 * Called only once in a thread safe manner */
void init_sensors(void) {
    void* handle;
    int idx;
    int sensors_count;
    const struct sensor_t* sensors_list = 0;

    handle = dlopen(HAL_VENDOR_PATH"/sensors.ssc.so", RTLD_NOW);

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

    /* Lazy initialize based on driver providing data */
    HAL_MODULE_INFO_SYM.set_operation_mode =
        ((struct sensors_module_t*)global_sensor_module)->set_operation_mode;

    sensors_count =
        ((struct sensors_module_t*)global_sensor_module)->get_sensors_list(
            (struct sensors_module_t*)global_sensor_module, &sensors_list);

    /* Check if there are any underlying sensors provided */
    if (sensors_count <= 0) {
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
    for (idx = 0; idx < sensors_count; ++idx) {
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
    int idx;
    ALOGV("module__get_sensors_list start");
    pthread_once(&init_sensors_once, init_sensors);

    *list = global_sensors_list;

    ALOGV("global_sensors_count: %d", global_sensors_count);
    for (idx = 0; idx < global_sensors_count; ++idx) {
        ALOGV("sensor type: %d", global_sensors_list[idx].type);
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

/*
 *  Copyright (C) 2015 The CyanogenMod Project
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you
 *  may not use this file except in compliance with the License.  You may
 *  obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 *  implied.  See the License for the specific language governing
 *  permissions and limitations under the License.
 */

#define LOG_TAG "FingerprintHAL-QSEECom"
// #define LOG_NDEBUG 0

#include <cutils/log.h>
#include <dlfcn.h>
#include <errno.h>
#include "QSEEComApp.h"
#include "QSEEComAPI.h"

QSEEComApp::~QSEEComApp()
{
    stop();
    if (mLibHandle) {
        dlclose(mLibHandle);
    }
}

int QSEEComApp::start()
{
    if (mAppRunning) {
        return 0;
    }

    if (!mLibInitialized) {
        mLibHandle = dlopen("libQSEEComAPI.so", RTLD_NOW);
        if (mLibHandle) {
            mStartApp = (start_app_fn) dlsym(mLibHandle, "QSEECom_start_app");
            mShutdownApp = (shutdown_app_fn) dlsym(mLibHandle, "QSEECom_shutdown_app");
            mSendCmd = (send_cmd_fn) dlsym(mLibHandle, "QSEECom_send_cmd");
            mSetBandwidth = (set_bandwidth_fn) dlsym(mLibHandle, "QSEECom_set_bandwidth");

            if (!mStartApp || !mShutdownApp || !mSendCmd || !mSetBandwidth) {
                dlclose(mLibHandle);
                mLibHandle = NULL;
            }
        }
        mLibInitialized = true;
    }

    if (!mLibHandle) {
        ALOGE("Failed to initialize QSEECom API library");
        return -EIO;
    }

    ALOGV("Starting app %s", mAppName);
    if (mStartApp(&mHandle, "/system/etc/firmware", mAppName,
            QSEECOM_ALIGN(mBufferSize) * 2) < 0) {
        return -errno;
    }
    if (mSetBandwidth(mHandle, true) < 0) {
        return -errno;
    }

    ALOGD("QSEECom app %s loaded", mAppName);
    mAppRunning = true;

    return 0;
}

void QSEEComApp::stop()
{
    if (mAppRunning) {
        mSetBandwidth(mHandle, false);
    }
    if (mHandle) {
        mShutdownApp(&mHandle);
        mHandle = NULL;
    }

    ALOGD("QSEECom app %s unloaded", mAppName);
    mAppRunning = false;
}

void * QSEEComApp::getSendBuffer()
{
    if (!mHandle) {
        return NULL;
    }
    return mHandle->ion_sbuffer;
}

void * QSEEComApp::getReceiveBuffer()
{
    if (!mHandle) {
        return NULL;
    }
    return mHandle->ion_sbuffer + QSEECOM_ALIGN(mBufferSize);
}

int QSEEComApp::sendCmd()
{
    if (!mAppRunning) {
        return -EINVAL;
    }
    if (mSendCmd(mHandle, getSendBuffer(), QSEECOM_ALIGN(mBufferSize),
            getReceiveBuffer(), QSEECOM_ALIGN(mBufferSize)) < 0) {
        return -errno;
    }
    return 0;
}

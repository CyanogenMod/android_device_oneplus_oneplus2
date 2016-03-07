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

#ifndef FINGERPRINT_QSEECOMAPP_H
#define FINGERPRINT_QSEECOMAPP_H

// forward declaration to keep the usage of
// the API header to the implementation
struct QSEECom_handle;

class QSEEComApp {
    public:
        QSEEComApp(const char *appName, size_t bufferSize) :
            mAppName(appName), mBufferSize(bufferSize),
            mHandle(NULL), mLibHandle(NULL),
            mLibInitialized(false), mAppRunning(false)
        {}
        ~QSEEComApp();

        int start();
        void stop();
        bool isRunning() {
            return mAppRunning;
        }

        void * getSendBuffer();
        void * getReceiveBuffer();

        int sendCmd();

    private:
        typedef int (*start_app_fn)(struct QSEECom_handle **handle,
                const char *path, const char *appname, uint32_t size);
        typedef int (*shutdown_app_fn)(struct QSEECom_handle **handle);
        typedef int (*send_cmd_fn)(struct QSEECom_handle* handle,
                void *sbuf, uint32_t slen, void *rbuf, uint32_t rlen);
        typedef int (*set_bandwidth_fn)(struct QSEECom_handle* handle, bool high);

    private:
        const char * mAppName;
        size_t mBufferSize;

        start_app_fn mStartApp;
        shutdown_app_fn mShutdownApp;
        send_cmd_fn mSendCmd;
        set_bandwidth_fn mSetBandwidth;

        struct QSEECom_handle * mHandle;
        void * mLibHandle;
        bool mLibInitialized;
        bool mAppRunning;
};

#endif // FINGERPRINT_QSEECOMAPP_H

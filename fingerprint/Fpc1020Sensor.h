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

#ifndef FINGERPRINT_FPC1020_SENSOR_H
#define FINGERPRINT_FPC1020_SENSOR_H

#include <utils/KeyedVector.h>
#include <utils/Thread.h>
#include <utils/Vector.h>
#include "QSEEComApp.h"
#include "fingerprint_tz_api.h"

class Fpc1020Sensor {
    public:
        struct EnrolledFingerprint {
            uint32_t fid;
            uint32_t gid;
            EnrolledFingerprint() :
                fid(0), gid(0) {}
            EnrolledFingerprint(uint32_t f, uint32_t g) :
                fid(f), gid(g) {}
        };

        typedef void (*AcquiredCb) (void *data);
        typedef void (*EnrollmentProgressCb) (const EnrolledFingerprint *fp, int stepsRemaining, void *data);
        typedef void (*AuthenticateResultCb) (const EnrolledFingerprint *fp, uint32_t userId, void *data);
        typedef void (*ErrorCb) (int result, void *data);

        Fpc1020Sensor(AcquiredCb acquiredCb, EnrollmentProgressCb enrollmentCb,
                AuthenticateResultCb authenticateCb, ErrorCb errorCb, void *cbData) :
            mQseecom("fingerprints", 512),
            mFpClockFd(-1),
            mFpApTzFd(-1),
            mFpcFd(-1),
            mCancelledDueToTimeout(false),
            mAcquiredCb(acquiredCb),
            mEnrollmentCb(enrollmentCb),
            mAuthenticateCb(authenticateCb),
            mErrorCb(errorCb),
            mCbData(cbData),
            mAuthenticatorId(0)
        {
            if (!loadPersistedMetaData()) {
                clearEnrolledFingerprints();
            }
        }

        ~Fpc1020Sensor() {
            if (!isIdle()) {
                goToIdleState();
            }
        }

        int startAuthentication();
        int startEnrollment(unsigned int timeout, uint32_t userId, uint32_t gid);
        int goToIdleState();

        int getEnrolledFingerprints(android::Vector<EnrolledFingerprint>& fps);
        int removeId(EnrolledFingerprint& fp);
        uint64_t getAuthenticatorId() {
            return mAuthenticatorId;
        }

    private:
        static const unsigned int EnrollmentStepCount = 20;
        static const uint32_t MetadataFileVersion = 1;
        static constexpr const char * MetadataFileName = "/data/misc/fp/metadata";

    private:
        bool isIdle() {
            android::Mutex::Autolock l(mThreadStateLock);
            return isIdleLocked();
        }
        bool isIdleLocked() {
            android::Thread *thread = mThread.get();
            return !thread || !thread->isRunning();
        }

        bool loadPersistedMetaData();
        int clearEnrolledFingerprints();
        void persistMetaData();

        void stopWatchdogThread();
        int activate(bool connect);
        void deactivate();
        int sendCommand(fingerprint_cmd_t cmd);

        int scanForTouchDown();

        class FingerprintThread : public android::Thread {
            protected:
                FingerprintThread(Fpc1020Sensor *sensor) :
                    Thread(true), mSensor(sensor) {}
                int waitForTouchDown();
                void handleShutdown();
                int adjustReturnValueForCancel(int ret);
                Fpc1020Sensor * mSensor;
        };

        class EnrollmentThread : public FingerprintThread {
            public:
                EnrollmentThread(Fpc1020Sensor *sensor, uint32_t userId, uint32_t gid) :
                    FingerprintThread(sensor), mUserId(userId), mGid(gid) {}
            private:
                virtual bool threadLoop();
                uint32_t mUserId;
                uint32_t mGid;
        };

        class AuthenticationThread : public FingerprintThread {
            public:
                AuthenticationThread(Fpc1020Sensor *sensor) :
                    FingerprintThread(sensor) {}
            private:
                virtual bool threadLoop();
                int doSingleAuthentication();
        };

        class TimeoutWatchdogThread : public android::Thread {
            public:
                TimeoutWatchdogThread(Fpc1020Sensor *sensor, unsigned int timeout) :
                    Thread(false), mSensor(sensor), mTimeout(timeout) {}
            private:
                virtual bool threadLoop();
                Fpc1020Sensor * mSensor;
                unsigned int mTimeout;
        };

    private:
        QSEEComApp mQseecom;
        int mFpClockFd;
        int mFpApTzFd;
        int mFpcFd;

        android::Mutex mTzLock;

        android::Mutex mThreadStateLock;
        // all of these are protected by mThreadStateLock
        android::sp<android::Thread> mThread;
        bool mCancelledDueToTimeout;

        android::Mutex mTimeoutLock;
        // all of these are protected by mTimeoutLock
        android::sp<android::Thread> mTimeoutWatchdog;
        android::Condition mTimeoutCondition;

        AcquiredCb mAcquiredCb;
        EnrollmentProgressCb mEnrollmentCb;
        AuthenticateResultCb mAuthenticateCb;
        ErrorCb mErrorCb;
        void * mCbData;

        struct FingerprintMetadata {
            uint32_t gid;
            uint32_t userId;
            FingerprintMetadata() : gid(0), userId(0) {}
            FingerprintMetadata(uint32_t g, uint32_t u) : gid(g), userId(u) {}
        };

        uint64_t mAuthenticatorId;
        android::KeyedVector<uint32_t, FingerprintMetadata> mFpMetadata;

        friend class EnrollmentThread;
        friend class AuthenticationThread;
        friend class TimeoutWatchdogThread;
};

#endif // FINGERPRINT_FPC1020_SENSOR_H

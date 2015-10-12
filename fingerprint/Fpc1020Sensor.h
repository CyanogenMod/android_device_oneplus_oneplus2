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

#include <utils/Thread.h>
#include <utils/Vector.h>
#include "QSEEComApp.h"
#include "fingerprint_tz_api.h"

class Fpc1020Sensor {
    public:
        typedef void (*AcquiredCb) (void *data);
        typedef void (*EnrollmentProgressCb) (uint32_t id, int stepsRemaining, void *data);
        typedef void (*AuthenticateResultCb) (bool success, uint32_t id, void *data);
        typedef void (*ErrorCb) (int result, void *data);

        Fpc1020Sensor(AcquiredCb acquiredCb, EnrollmentProgressCb enrollmentCb,
                AuthenticateResultCb authenticateCb, ErrorCb errorCb, void *cbData) :
            mQseecom("fingerprints", 512),
            mFpcFd(-1),
            mWakeupEnabled(false),
            mWaitingForWakeup(false),
            mCancelledDueToTimeout(false),
            mAcquiredCb(acquiredCb),
            mEnrollmentCb(enrollmentCb),
            mAuthenticateCb(authenticateCb),
            mErrorCb(errorCb),
            mCbData(cbData)
        {}

        ~Fpc1020Sensor() {
            if (!isIdle()) {
                goToIdleState();
            }
        }

        int startAuthentication(bool inWakeupMode = false);
        int startEnrollment(unsigned int timeout);
        int setWakeupMode(bool enable);
        int goToIdleState();

        int getEnrolledIds(android::Vector<uint32_t>& ids);
        int removeId(uint32_t id);

    public:
        static const int EnrollmentStepCount = 20;

    private:
        bool isIdle() {
            android::Mutex::Autolock l(mThreadStateLock);
            return isIdleLocked();
        }
        bool isIdleLocked() {
            android::Thread *thread = mThread.get();
            return !thread || !thread->isRunning();
        }

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
                EnrollmentThread(Fpc1020Sensor *sensor) :
                    FingerprintThread(sensor) {}
            private:
                virtual bool threadLoop();
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
        int mFpcFd;

        android::Mutex mThreadStateLock;
        // all of these are protected by mThreadStateLock
        android::sp<android::Thread> mThread;
        bool mWakeupEnabled;
        bool mWaitingForWakeup;
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

        friend class EnrollmentThread;
        friend class AuthenticationThread;
        friend class TimeoutWatchdogThread;
};

#endif // FINGERPRINT_FPC1020_SENSOR_H

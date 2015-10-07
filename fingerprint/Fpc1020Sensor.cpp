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

#define LOG_TAG "FingerprintHAL"
// #define LOG_NDEBUG 0

#include <cutils/log.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Fpc1020Sensor.h"
#include "fpc1020_ioctl.h"

int Fpc1020Sensor::startAuthentication(bool inWakeupMode)
{
    if (!isIdle()) {
        return -EINVAL;
    }

    ALOGV("startAuthentication(%d)", inWakeupMode);
    int ret = activate(true);
    if (ret) {
        return ret;
    }

    mThread = new AuthenticationThread(this);
    mWaitingForWakeup = inWakeupMode;
    mThread->run("FingerprintAuthentication");

    ALOGD("State machine now in authentication state");

    return 0;
}

int Fpc1020Sensor::startEnrollment(unsigned int timeout)
{
    if (!isIdle()) {
        return -EINVAL;
    }

    ALOGV("startEnrollment(%d)", timeout);
    int ret = activate(true);
    if (ret) {
        return ret;
    }

    if (timeout) {
        mTimeoutWatchdog = new TimeoutWatchdogThread(this, timeout);
        mTimeoutWatchdog->run("FingerprintTimeout");
    }

    mThread = new EnrollmentThread(this);
    mThread->run("FingerprintEnrollment");

    ALOGD("State machine now in enrollment state");

    return 0;
}

int Fpc1020Sensor::setWakeupMode(bool enable)
{
    int ret = 0;
    bool idle, inWakeupMode;

    ALOGV("setWakeupMode(%d)", enable);
    {
        android::Mutex::Autolock l(mThreadStateLock);
        idle = isIdleLocked();
        inWakeupMode = mWaitingForWakeup;
    }

    ALOGD("Set wakeup mode: idle %d wait for wakeup %d enable %d", idle, inWakeupMode, enable);
    mWakeupEnabled = enable;

    if (enable && idle) {
        ret = startAuthentication(true);
    } else if (!enable && !idle && inWakeupMode) {
        ret = goToIdleState();
    }

    return ret;
}

int Fpc1020Sensor::goToIdleState()
{
    ALOGV("goToIdleState()");
    stopWatchdogThread();

    android::sp<android::Thread> thread;
    {
        android::Mutex::Autolock l(mThreadStateLock);
        thread = mThread;
    }

    if (thread.get()) {
        ALOGV("Requesting worker thread exit");
        thread->requestExit();

        if (mFpcFd >= 0) {
            if (ioctl(mFpcFd, FPC_ABORT) < 0) {
                ALOGE("Aborting the worker thread failed");
                return -errno;
            }
        }

        thread->join();
    }

    mCancelledDueToTimeout = false;

    if (mWakeupEnabled) {
        ALOGD("Starting idle wakeup mode");
        startAuthentication(true);
    } else {
        ALOGD("State machine now in idle state");
    }

    return 0;
}

void Fpc1020Sensor::stopWatchdogThread()
{
    android::sp<android::Thread> watchdogThread;

    ALOGV("stopWatchdogThread()");

    {
        android::Mutex::Autolock l(mTimeoutLock);
        watchdogThread = mTimeoutWatchdog;
        mTimeoutCondition.broadcast();
        mTimeoutWatchdog.clear();
    }

    if (watchdogThread.get()) {
        watchdogThread->join();
    }
}

int Fpc1020Sensor::getEnrolledIds(android::Vector<uint32_t>& ids)
{
    if (!isIdle()) {
        return -EINVAL;
    }

    ALOGV("getEnrolledIds()");
    int ret = activate(false);
    if (ret) {
        return ret;
    }

    ret = sendCommand(CLIENT_CMD_GET_IDS_LIST);
    if (ret == 0) {
        fingerprint_get_ids_list_rsp_t *resp =
                (fingerprint_get_ids_list_rsp_t *) mQseecom.getReceiveBuffer();
        ALOGD("getEnrolledIds(): result %d, count %d", resp->result, resp->count);

        if (resp->result == 0) {
            for (uint32_t i = 0; i < resp->count; i++) {
                ids.push_back(resp->ids[i]);
            }
        }
    }

    deactivate();

    return ret;
}

int Fpc1020Sensor::removeId(uint32_t id)
{
    if (!isIdle()) {
        return -EINVAL;
    }

    ALOGV("removeId(%d)", id);
    int ret = activate(false);
    if (ret) {
        return ret;
    }

    fingerprint_delete_cmd_t *req = (fingerprint_delete_cmd_t *) mQseecom.getSendBuffer();
    req->id = id;

    ret = sendCommand(CLIENT_CMD_REMOVE_ID);
    deactivate();

    return ret;
}

int Fpc1020Sensor::activate(bool connect)
{
    int ret;

    ret = mQseecom.start();
    if (ret) {
        ALOGE("Failed starting TZ app: %d", ret);
        return ret;
    }

    if (mFpcFd < 0) {
        mFpcFd = open("/dev/fpc1020", O_RDWR);
        if (mFpcFd < 0) {
            ret = -errno;
            ALOGE("Failed opening FPC1020: %d", ret);
            goto out;
        }
    }

    if (ioctl(mFpcFd, FPC_HW_RESET) < 0) {
        ret = -errno;
        ALOGE("Failed resetting FPC1020: %d", ret);
        goto out;
    }

    if (connect) {
        ret = sendCommand(CLIENT_CMD_FP_CONNECT);
        if (ret) {
            ALOGE("TZ connection failed: %d", ret);
            goto out;
        }
    }

    ret = sendCommand(CLIENT_CMD_FP_LOAD_IDS);
    if (ret) {
        ALOGE("TZ app initialization failed: %d", ret);
        goto out;
    }

out:
    if (ret) {
        deactivate();
    }
    return ret;
}

void Fpc1020Sensor::deactivate()
{
    if (mQseecom.isRunning()) {
        int ret = sendCommand(CLIENT_CMD_FP_RELEASE);
        if (ret != 0) {
            ALOGW("Failed to release TZ app: %d", ret);
        }
        mQseecom.stop();
    }
    if (mFpcFd >= 0) {
        close(mFpcFd);
        mFpcFd = -1;
    }
}

int Fpc1020Sensor::sendCommand(fingerprint_cmd_t cmd)
{
    fingerprint_base_cmd_t *req = (fingerprint_base_cmd_t *) mQseecom.getSendBuffer();
    req->cmd_id = cmd;

    return mQseecom.sendCmd();
}

int Fpc1020Sensor::scanForTouchDown()
{
    int ret, result;

    ret = sendCommand(CLIENT_CMD_WAIT_FOR_TOUCH_DOWN);
    if (ret) {
        ALOGE("Failed sending WAIT_FOR_TOUCH_DOWN command: %d", ret);
        return ret;
    }

    fingerprint_get_touch_rsp_t *resp =
            (fingerprint_get_touch_rsp_t *) mQseecom.getReceiveBuffer();
    if (resp->result == 0) {
        return 0;
    }

    if (ioctl(mFpcFd, FPC_GET_INTERRUPT, &result) < 0) {
        ret = -errno;
        ALOGE("Failed waiting for interrupt: %d", ret);
    }

    return ret;
}

int Fpc1020Sensor::FingerprintThread::waitForTouchDown()
{
    fingerprint_get_touch_rsp_t *resp =
            (fingerprint_get_touch_rsp_t *) mSensor->mQseecom.getReceiveBuffer();
    int ret;

    // First wait for the sensor to become free ...
    do {
        ret = mSensor->sendCommand(CLIENT_CMD_WAIT_FOR_TOUCH_UP);
        if (ret) {
            return ret;
        }
    } while (resp->result != 0);

    // ... then wait for the next touch
    do {
        ret = mSensor->scanForTouchDown();
        if (exitPending()) {
            return -EINTR;
        }
    } while (ret == 0 && resp->result != 0);

    return ret;
}

void Fpc1020Sensor::FingerprintThread::handleShutdown()
{
    ALOGV("Shutting down thread %d", getTid());
    mSensor->deactivate();
    mSensor->stopWatchdogThread();

    android::Mutex::Autolock l(mSensor->mThreadStateLock);
    mSensor->mWaitingForWakeup = false;
    mSensor->mThread.clear();

    ALOGV("Thread %d now shut down", getTid());
}

int Fpc1020Sensor::FingerprintThread::adjustReturnValueForCancel(int ret)
{
    android::Mutex::Autolock l(mSensor->mThreadStateLock);
    if (ret == -EINTR) {
        // operation was cancelled
        return mSensor->mCancelledDueToTimeout ? -ETIMEDOUT : 0;
    }
    return ret;
}

bool Fpc1020Sensor::EnrollmentThread::threadLoop()
{
    int ret, enrolledId, stepsRemaining = Fpc1020Sensor::EnrollmentStepCount;
    fingerprint_enroll_rsp_t *resp =
            (fingerprint_enroll_rsp_t *) mSensor->mQseecom.getReceiveBuffer();
    fingerprint_end_enroll_cmd_t *endReq =
            (fingerprint_end_enroll_cmd_t *) mSensor->mQseecom.getSendBuffer();
    fingerprint_end_enroll_rsp_t *endResp =
            (fingerprint_end_enroll_rsp_t *) mSensor->mQseecom.getReceiveBuffer();

    ALOGV("Started enrollment thread");
    while (!exitPending()) {
        ret = waitForTouchDown();
        if (ret) {
            goto out;
        }
        mSensor->mAcquiredCb(mSensor->mCbData);
        ret = mSensor->sendCommand(CLIENT_CMD_FP_GET_IMAGE_WITH_CAC);
        if (ret) {
            goto out;
        }
        ret = mSensor->sendCommand(CLIENT_CMD_FP_DO_ENROLL);
        if (ret) {
            goto out;
        }

        ALOGD("Enrollment step done: result %d progress %d", resp->result, resp->progress);

        if (resp->result == 3) {
            // The return value of 3 means 'enrollment done'
            break;
        } else if (resp->result == 0) {
            stepsRemaining--;
            // While empiric evidence shows the number of enrollment steps is
            // always 10, the protocol doesn't provide any guarantee for it.
            // Safeguard against that case.
            if (stepsRemaining <= 0) {
                stepsRemaining = 1;
            }
            mSensor->mEnrollmentCb(0xffffffff, stepsRemaining, mSensor->mCbData);
        } else {
            ALOGI("Enrollment step returned error (%d), ignoring.", resp->result);
        }
    }

    memset(endReq->unknown, 0, sizeof(endReq->unknown));
    endReq->identifier_len = 0; // identifier unused for now

    ret = mSensor->sendCommand(CLIENT_CMD_FP_END_ENROLL);
    if (ret) {
        goto out;
    }

    ALOGD("Enrollment thread finished: result %d id 0x%08x", endResp->result, endResp->id);
    enrolledId = endResp->id;
    if (endResp->result != 0) {
        ret = -EIO;
    }

out:
    ret = adjustReturnValueForCancel(ret);
    handleShutdown();

    if (ret == 0) {
        // Do the final enrollment callback after doing the cleanup, so
        // we're in idle state when doing this call. Upper layers query
        // the enrollment list more or less directly after this callback,
        // so if we aren't in idle state at this point, we're prone to
        // race conditions.
        mSensor->mEnrollmentCb(enrolledId, 0, mSensor->mCbData);
    } else {
        ALOGD("Enrollment failed: %d", ret);
        mSensor->mErrorCb(ret, mSensor->mCbData);
    }

    return false;
}

int Fpc1020Sensor::AuthenticationThread::doSingleAuthentication()
{
    int ret;
    fingerprint_verify_rsp_t *resp =
            (fingerprint_verify_rsp_t *) mSensor->mQseecom.getReceiveBuffer();

    ret = waitForTouchDown();
    if (ret) {
        goto out;
    }

    mSensor->mAcquiredCb(mSensor->mCbData);

    ret = mSensor->sendCommand(CLIENT_CMD_FP_GET_IMAGE_WITH_CAC);
    if (ret) {
        goto out;
    }

    ret = mSensor->sendCommand(CLIENT_CMD_VERIFY);
    if (ret) {
        goto out;
    }

    ALOGD("Authentication: got id 0x%08x certainty %d", resp->id, resp->certainty);
    mSensor->mAuthenticateCb(resp->id != 0xffffffff, resp->id, mSensor->mCbData);

out:
    ret = adjustReturnValueForCancel(ret);
    if (ret) {
        ALOGD("Authentication failed: %d", ret);
        mSensor->mErrorCb(ret, mSensor->mCbData);
    }
    return ret;
}

bool Fpc1020Sensor::AuthenticationThread::threadLoop()
{
    ALOGV("Started authentication thread");

    do {
        int ret = doSingleAuthentication();
        if (ret) {
            ALOGW("Authentication attempt failed: %d", ret);
            mSensor->mErrorCb(ret, mSensor->mCbData);
        }
    } while (!exitPending() && mSensor->mWaitingForWakeup);

    handleShutdown();
    return false;
}

bool Fpc1020Sensor::TimeoutWatchdogThread::threadLoop()
{
    ALOGV("Started timeout watchdog thread, timeout = %d sec", mTimeout);

    android::status_t status;
    {
        android::Mutex::Autolock l(mSensor->mTimeoutLock);
        status = mSensor->mTimeoutCondition.waitRelative(mSensor->mTimeoutLock,
                1000000000LL * mTimeout);
        ALOGV("Timeout watchdog wait returned, result = %d", status);
    }

    if (status == android::TIMED_OUT) {
        {
            android::Mutex::Autolock l(mSensor->mThreadStateLock);
            mSensor->mCancelledDueToTimeout = true;
        }
        mSensor->goToIdleState();
    }

    return false;
}

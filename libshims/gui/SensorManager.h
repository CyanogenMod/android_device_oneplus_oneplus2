/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef ANDROID_GUI_SENSOR_MANAGER_H
#define ANDROID_GUI_SENSOR_MANAGER_H

#include <stdint.h>
#include <sys/types.h>

#include <binder/IBinder.h>

#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/Singleton.h>
#include <utils/Vector.h>

#include <gui/SensorEventQueue.h>

// ----------------------------------------------------------------------------
// Concrete types for the NDK
struct ASensorManager { };

// ----------------------------------------------------------------------------
namespace android {
// ----------------------------------------------------------------------------

class ISensorServer;
class Sensor;
class SensorEventQueue;

// ----------------------------------------------------------------------------

class SensorManager :
    public ASensorManager,
    public Singleton<SensorManager>
{
public:
    SensorManager();
    ~SensorManager();

    ssize_t getSensorList(Sensor const* const** list) const;
    Sensor const* getDefaultSensor(int type);
    sp<SensorEventQueue> createEventQueue();

private:
    // DeathRecipient interface
    void sensorManagerDied();

    status_t assertStateLocked() const;

private:
    mutable Mutex mLock;
    mutable sp<ISensorServer> mSensorServer;
    mutable Sensor const** mSensorList;
    mutable Vector<Sensor> mSensors;
    mutable sp<IBinder::DeathRecipient> mDeathObserver;
};

// ----------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_GUI_SENSOR_MANAGER_H

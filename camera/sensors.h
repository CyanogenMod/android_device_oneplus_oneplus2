/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef ANDROID_SENSORS_H
#define ANDROID_SENSORS_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <string.h>

#include <linux/input.h>

#include <hardware/hardware.h>
#include <hardware/sensors.h>

__BEGIN_DECLS

/*****************************************************************************/

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define ID_A  (0)
#define ID_M  (1)
#define ID_O  (2)
#define ID_L  (3)
#define ID_P  (4)
#define ID_GY (5)
#define ID_PR (6)
#define ID_T  (7)

/*****************************************************************************/

/*
 * The SENSORS Module
 */

/* the GP2A is a binary proximity sensor that triggers around 5 cm on
 * this hardware */
#define PROXIMITY_THRESHOLD_GP2A  5.0f

/*****************************************************************************/

#define EVENT_TYPE_ACCEL_X          ABS_X
#define EVENT_TYPE_ACCEL_Y          ABS_Y
#define EVENT_TYPE_ACCEL_Z          ABS_Z

#define EVENT_TYPE_YAW              REL_RX
#define EVENT_TYPE_PITCH            REL_RY
#define EVENT_TYPE_ROLL             REL_RZ
#define EVENT_TYPE_ORIENT_STATUS    REL_WHEEL


#define EVENT_TYPE_MAG_X           ABS_X
#define EVENT_TYPE_MAG_Y           ABS_Y
#define EVENT_TYPE_MAG_Z           ABS_Z

#define EVENT_TYPE_PROXIMITY        ABS_DISTANCE
#define EVENT_TYPE_LIGHT            ABS_MISC

#define EVENT_TYPE_GYRO_X           ABS_X
#define EVENT_TYPE_GYRO_Y           ABS_Y
#define EVENT_TYPE_GYRO_Z           ABS_Z

#define EVENT_TYPE_PRESS            ABS_PRESSURE
#define EVENT_TYPE_TEMP             ABS_GAS


// under default setting in acc driver, Full Scale = 2G, 1000 LSG = 1G
#define LSG                 (1000.0f)

// conversion of acceleration data to SI units (m/s^2)
#define RANGE_A                     (2*GRAVITY_EARTH)
#define CONVERT_A                   (GRAVITY_EARTH / LSG)
#define CONVERT_A_X                 (CONVERT_A)
#define CONVERT_A_Y                 (CONVERT_A)
#define CONVERT_A_Z                 (CONVERT_A)

// conversion of magnetic data to uT units
// conversion of magnetic data to uT units. 1G = 100 uT.
#define CONVERT_M                   (1.0f/10.0f)
#define CONVERT_M_X                 (CONVERT_M)
#define CONVERT_M_Y                 (CONVERT_M)
#define CONVERT_M_Z                 (CONVERT_M)

/* conversion of orientation data to degree units */
#define CONVERT_O                   (1.0f/64.0f)
#define CONVERT_O_A                 (CONVERT_O)
#define CONVERT_O_P                 (CONVERT_O)
#define CONVERT_O_R                 (CONVERT_O)

/* Conversion of pressure and temperature */
#define CONVERT_P (1.0f/4096.0f)
#define CONVERT_T(x) ((x / 480.0f) + 42.5)

// conversion of gyro data to SI units (radian/sec)
//#define RANGE_GYRO                  (2000.0f*(float)M_PI/180.0f)
// #define CONVERT_GYRO                ((70.0f / 1000.0f) * ((float)M_PI / 180.0f))
// **************************
// under default setting in gyro driver, Full Scale = 250dps, sensitivity = 8.75 mdps/digit
#define GYRO_SSTVT          (8.75f/1000.0f)
#define CONVERT_GYRO                (GYRO_SSTVT * ((float)M_PI / 180.0f))
#define CONVERT_GYRO_X              (CONVERT_GYRO)
#define CONVERT_GYRO_Y              (CONVERT_GYRO)
#define CONVERT_GYRO_Z              (CONVERT_GYRO)

#define SENSOR_STATE_MASK           (0x7FFF)

/*****************************************************************************/

__END_DECLS

#endif  // ANDROID_SENSORS_H

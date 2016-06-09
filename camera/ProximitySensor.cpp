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

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <cutils/log.h>
#include <linux/input.h>

#include "configuration.h"
#include "ProximitySensor.h"

//#define FETCH_FULL_EVENT_BEFORE_RETURN 1

/*****************************************************************************/

ProximitySensor::ProximitySensor()
    : SensorBase(NULL, "STM VL6180 proximity sensor"),
      mEnabled(0),
	  mBatchEnabled(0),
      mInputReader(4),
      mHasPendingEvent(false)
{
    //Added for debug
    LOGV("ProximitySensor::ProximitySensor gets called!");
	
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_P;
    mPendingEvent.type = SENSOR_TYPE_TIME_OF_FLIGHT;
    memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));

    if (data_fd) {
        strcpy(input_sysfs_path, "/sys/class/input/");
        strcat(input_sysfs_path, "input");
        strcat(input_sysfs_path, input_name+5);
        strcat(input_sysfs_path, "/device/");
        input_sysfs_path_len = strlen(input_sysfs_path);
	 LOGV("ProximitySensor: input_name:%s, input_sysfs_path:%s",input_name,input_sysfs_path);
        enable(0, 1);
    }else
    	LOGV("%s:%d wrong fd", __func__, __LINE__);
}

ProximitySensor::~ProximitySensor() {
    if (mEnabled) {
        enable(0, 0);
    }
}

int ProximitySensor::setInitialState() {
    struct input_absinfo absinfo;
	int rc;

	rc = ioctl(data_fd, EVIOCGABS(EVENT_TYPE_PROXIMITY), &absinfo);
    if (!rc ) {
		LOGE("%s:%d Pend event successfully!", __func__, __LINE__);
        mHasPendingEvent = true;
		mPendingEvent.distance = absinfo.value;
		return 0;
    }
    else{
		LOGV("%s:%d Pend event failed!", __func__, __LINE__);		
    	return rc;
    }
}

int ProximitySensor::enable(int32_t, int en) {

    //Added for debug
    LOGE("ProximitySensor::enable function called for stmvl6180 %d!", en);

    char buf[2];
    int err;

    int flags = en ? 1 : 0;
    if (flags != mEnabled) {
        int fd;
        //strcpy(&input_sysfs_path[input_sysfs_path_len], "enable");
        //fd = open(input_sysfs_path, O_RDWR);
	fd = open(PROXIMITY_ENABLE_FILENAME_CCI, O_RDWR);
	if(fd < 0){
		LOGE("%s:%d open %s failed(%s), try to open %s", __func__, __LINE__, PROXIMITY_ENABLE_FILENAME_CCI, strerror(errno), PROXIMITY_ENABLE_FILENAME_I2C);
		fd = open(PROXIMITY_ENABLE_FILENAME_I2C, O_RDWR);
	}
	
        if (fd >= 0) {
            buf[1] = 0;
            if (flags) {
                buf[0] = '1';
            } else {
                buf[0] = '0';
        	}
	    LOGV("%s:%d fd = %d, flags = %d", __func__, __LINE__, fd, flags);

           err = write(fd, buf, sizeof(buf));
	    LOGV("%s:%d after write", __func__, __LINE__, fd, flags);            
	    if(err <= 0){
		LOGE("%s:%d write %d failed: %s", __func__, __LINE__, fd, strerror(errno));
	       close(fd);			
		return err;
	    }
	    else
		LOGV("%s:%d write successfully!", __func__, __LINE__, fd, flags);
		
            close(fd);
            mEnabled = flags;
            setInitialState();
	    	//Added for debug
	    	LOGV("ProximitySensor::enable enabled VL6180! Error code(2 should indicate success): %d", err);
            return 0;
        }
        LOGV("ProximitySensor::enable: Failed to open %s for enabling VL6180 %s", PROXIMITY_ENABLE_FILENAME_I2C, strerror(errno));
        LOGV("ProximitySensor::enable: Attempted to open file %s", input_sysfs_path);
        return -1;
    }
    return 0;
}

bool ProximitySensor::hasPendingEvents() const {
    return mHasPendingEvent;
}

int ProximitySensor::batch(int handle, int flags, int64_t period_ns, int64_t timeout)
{
   	int err;

 	if (period_ns > 0)
	{
        int fd;
	 	fd = open(PROXIMITY_SET_DELAY_FILENAME_CCI, O_RDWR);
	 	if(fd < 0){
			LOGE("%s:%d open %s failed(%s), try to open %s", __func__, __LINE__, PROXIMITY_SET_DELAY_FILENAME_CCI, strerror(errno), PROXIMITY_SET_DELAY_FILENAME_I2C);
			fd = open(PROXIMITY_SET_DELAY_FILENAME_I2C, O_RDWR);
	 	}	 
        if (fd >= 0) {
		   	char buf[80];
			sprintf(buf,"%lld", period_ns / 1000000);
            err = write(fd, buf, strlen(buf)+1);
            close(fd);
	    	//Added for debug
	    	LOGV("ProximitySensor::batch return: %d", err);
            return 0;
        }
        LOGV("ProximitySensor::batch: Failed to open %s for set delay time(%s)", PROXIMITY_SET_DELAY_FILENAME_I2C, strerror(errno));
        return -1;
    }
    return 0;

}

int ProximitySensor::setDelay(int32_t handle, int64_t delay_ns)
{
    LOGV("ProximitySensor::setDelay called!");
    int fd;
    strcpy(&input_sysfs_path[input_sysfs_path_len], "pollrate_ms");
    fd = open(input_sysfs_path, O_RDWR);
    if (fd >= 0) {
        char buf[80];
        sprintf(buf, "%lld", delay_ns / 1000000);
        write(fd, buf, strlen(buf)+1);
        close(fd);
		//Added for debug
		LOGV("ProximitySensor::setDelay is successful!");
        return 0;
    }
    LOGV("ProximitySensor::setDelay failed!");
    return -1;
}

int ProximitySensor::readEvents(sensors_event_t* data, int count)
{

    //Added for debug

    if (count < 1)
        return -EINVAL;

    if (mHasPendingEvent) {
        mHasPendingEvent = false;
        mPendingEvent.timestamp = getTimestamp();
        *data = mPendingEvent;
        return mEnabled ? 1 : 0;
    }

    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0){
        LOGE("ProximitySensor::readEvents: Did not successfully read an event, error: %d", n);
        return n;
    }

    int numEventReceived = 0;
    input_event const* event;

#if FETCH_FULL_EVENT_BEFORE_RETURN
again:
#endif
    LOGV("ProximitySensor::readEvents: At readEvent loop, probably was able to read at least one event successfully");
    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
        if (type == EV_ABS) {
            if (event->code == EVENT_TYPE_PROXIMITY) { //ABS_DISTANCE
               if (event->value != -1) {
                    // FIXME: not sure why we're getting -1 sometimes
                    //mPendingEvent.distance = indexToValue(event->value);
		    		//mPendingEvent.distance = event->value;
		    		mPendingEvent.data[0] = event->value;
		    		LOGE("%s:%d mPendingEvent.data[0] = %d",__func__, __LINE__, event->value);
                }
            } 
			/*
	    	else if (event->code == ABS_X)
				mPendingEvent.data[1] = event->value;
	    	else if (event->code == ABS_HAT0X)
				mPendingEvent.data[2] = event->value;
			*/

		    else if (event->code == ABS_HAT0X){
				mPendingEvent.data[1] = event->value;
				LOGV("%s:%d mPendingEvent.data[1] = %d",__func__, __LINE__, event->value);

			}
		    else if (event->code == ABS_HAT0Y){
				mPendingEvent.data[2] = event->value;
				LOGV("%s:%d mPendingEvent.data[2] = %d",__func__, __LINE__, event->value);

			}
		    else if (event->code == ABS_HAT1X){
				mPendingEvent.data[3] = event->value;
				LOGV("%s:%d mPendingEvent.data[3] = %d",__func__, __LINE__, event->value);

			}
		    else if (event->code == ABS_HAT1Y){
				mPendingEvent.data[4] = event->value;
				LOGV("%s:%d mPendingEvent.data[4] = %d",__func__, __LINE__, event->value);

			}
		    else if (event->code == ABS_HAT2X){
				mPendingEvent.data[5] = event->value;
				LOGV("%s:%d mPendingEvent.data[5] = %d",__func__, __LINE__, event->value);

			}
		    else if (event->code == ABS_HAT2Y){
				mPendingEvent.data[6] = event->value;
				LOGV("%s:%d mPendingEvent.data[6] = %d",__func__, __LINE__, event->value);

			}
		    else if (event->code == ABS_HAT3X){
				mPendingEvent.data[7] = event->value;
				LOGV("%s:%d mPendingEvent.data[7] = %d",__func__, __LINE__, event->value);

			}
		    else if (event->code == ABS_HAT3Y){
				mPendingEvent.data[8] = event->value;
				LOGV("%s:%d mPendingEvent.data[8] = %d",__func__, __LINE__, event->value);

			}

            LOGV("ProximitySensor: gets one ps data event!");
        } 
		else if (type == EV_SYN) {
            mPendingEvent.timestamp = timevalToNano(event->time);
            if (mEnabled) {
                *data++ = mPendingEvent;
                numEventReceived++;
                count--;
	        //Added for debug
		LOGV("ProximitySensor: gets one ps sync event!");
            }
        } else {
            LOGV("ProximitySensor: unknown event (type=%d, code=%d)",
                    type, event->code);
        }
        mInputReader.next();
    }
#if 0
	LOGV("HAL: distance: %f cm tv_sec: %f tv_usec: %f " 
      "distance: %fmm error_code: %x signalRate_mcps: %f " 
      "rtnAmbRate(kcps): %f rtnConvTime: %f dmax_sq: %f", 
      mPendingEvent.data[0], mPendingEvent.data[1], mPendingEvent.data[2], mPendingEvent.data[3], 
      mPendingEvent.data[4], mPendingEvent.data[5], mPendingEvent.data[6],mPendingEvent.data[7], 
      mPendingEvent.data[8]);
#endif

#if FETCH_FULL_EVENT_BEFORE_RETURN
    /* if we didn't read a complete event, see if we can fill and
       try again instead of returning with nothing and redoing poll. */
    if (numEventReceived == 0 && mEnabled == 1) {
        n = mInputReader.fill(data_fd);
        if (n)
            goto again;
    }
#endif

    return numEventReceived;
}



/*
 * FPC1020 Touch sensor driver ioctl definitions
 *
 * Copied from kernel - drivers/input/misc/fpc1020_common.h
 *
 * Copyright (c) 2013,2014 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License Version 2
 * as published by the Free Software Foundation.
 */

#ifndef ANDROID_HARDWARE_FPC1020_IOCTL_H
#define ANDROID_HARDWARE_FPC1020_IOCTL_H

#include <linux/ioctl.h>

#define FPC_HW_RESET			_IOW('K', 0, int)
#define FPC_GET_INTERRUPT		_IOR('K', 1, int)
#define FPC_ABORT			_IOR('K', 6, int)

#endif // ANDROID_HARDWARE_FPC1020_IOCTL_H

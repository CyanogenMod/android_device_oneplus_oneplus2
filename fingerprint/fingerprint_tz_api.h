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

#ifndef FINGERPRINT_TZ_API_H
#define FINGERPRINT_TZ_API_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

/**
 * Commands supported
 */
enum fingerprint_cmd_t {
    CLIENT_CMD_FP_RELEASE = 0x13,
    CLIENT_CMD_FP_LOAD_IDS = 0x14,
    CLIENT_CMD_FP_CONNECT = 0x15,
    CLIENT_CMD_WAIT_FOR_TOUCH_DOWN = 0x17,
    CLIENT_CMD_WAIT_FOR_TOUCH_UP = 0x19,
    CLIENT_CMD_FP_DO_ENROLL = 0x1a,
    CLIENT_CMD_FP_END_ENROLL = 0x1b,
    CLIENT_CMD_VERIFY = 0x1c,
    CLIENT_CMD_GET_IDS_LIST = 0x1e,
    CLIENT_CMD_GET_ID_NAME = 0x1f,
    CLIENT_CMD_REMOVE_ID = 0x20,
    CLIENT_CMD_SELF_TEST = 0x21,
    CLIENT_CMD_FP_GET_IMAGE = 0x22,
    CLIENT_CMD_FP_GET_IMAGE_WITH_CAC = 0x23
};

//
// FOR ALL COMMANDS UNLESS OTHERWISE NOTED
//
typedef struct {
    uint32_t cmd_id;    // one of the values of fingerprint_cmd_t
} fingerprint_base_cmd_t;

//
// FOR CLIENT_CMD_FP_DO_ENROLL
//
typedef struct {
    uint32_t result;    // 0 on success
                        // 3 on success and no further necessary steps
                        // 1007 on failure
    uint32_t progress;  // 0..100
} fingerprint_enroll_rsp_t;

//
// FOR CLIENT_CMD_FP_END_ENROLL
//
typedef struct {
    fingerprint_base_cmd_t base;
    uint32_t unknown[2]; // send as 0
    uint32_t identifier_len;
    char identifier[0];
} fingerprint_end_enroll_cmd_t;

typedef struct {
    uint32_t result;    // 0 on success, ? otherwise
    uint32_t id;        // ID of enrolled fingerprint
} fingerprint_end_enroll_rsp_t;

//
// FOR CLIENT_CMD_GET_IDS_LIST
//
typedef struct {
    uint32_t result;    // 0 on success, ? otherwise
    uint32_t count;     // number of enrolled fingerprints
    uint32_t unknown;   // seen as 0
    uint32_t data_len;  // count * sizeof(int32_t)
    uint32_t ids[0];    // enrolled fingerprint IDs
} fingerprint_get_ids_list_rsp_t;

//
// FOR CLIENT_CMD_REMOVE_ID
//
typedef struct {
    fingerprint_base_cmd_t base;
    uint32_t id;        // ID of fingerprint to be deleted
} fingerprint_delete_cmd_t;

//
// FOR CLIENT_CMD_VERIFY
//
typedef struct {
    uint32_t certainty; // 0..100? maybe flags
    uint32_t id;        // ID of recognized fingerprint
} fingerprint_verify_rsp_t;

//
// FOR CLIENT_CMD_WAIT_FOR_TOUCH_[UP|DOWN]
//
typedef struct {
    uint32_t result;// 0 if success (touch happened for down, no touch for up)
                    // 1001 otherwise
} fingerprint_get_touch_rsp_t;

#endif  // FINGERPRINT_TZ_API_H

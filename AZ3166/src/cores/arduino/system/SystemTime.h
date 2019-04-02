// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __SYSTEM_CLOCK_H__
#define __SYSTEM_CLOCK_H__

#include "mbed.h"

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

/**
 * @brief    Provide a new Time Server list.
 * 
 * @param    tsList - Time Server string list, split by comma.
 * 
 * @return   0 upon success or other value upon failure.
**/
int SetTimeServer(const char * tsList);

/**
 * @brief    Sync up the time from Time Server.
**/
void SyncTime(void);

/**
 * @brief    Check whether the local time is synced up with Time Server.
 * 
 * @return   0 upon synced or other value upon sync failed.
**/
int IsTimeSynced(void);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __SYSTEM_CLOCK_H__

// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __SYSTEM_FUNC_H__
#define __SYSTEM_FUNC_H__

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

/**
* @brief	Reboot the system.
*/
void SystemReboot(void);

/**
* @brief	           Enter sleep mode and restart after reset interval.
* @param    timeout    Timeout interval in second.
*/
void SystemStandby(int timeout);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __SYSTEM_FUNC_H__

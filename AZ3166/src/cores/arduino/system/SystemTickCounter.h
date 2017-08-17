// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __SYSTEM_TICKCOUNTER_H__
#define __SYSTEM_TICKCOUNTER_H__

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

void SystemTckCounterInit(void);
uint64_t SystemTckCounterRead(void);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __SYSTEM_TICKCOUNTER_H__

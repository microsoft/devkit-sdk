// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#ifndef __SYSTEM_TICKCOUNTER_H__
#define __SYSTEM_TICKCOUNTER_H__

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    void SystemTickCounterInit(void);
    uint64_t SystemTickCounterRead(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __SYSTEM_TICKCOUNTER_H__

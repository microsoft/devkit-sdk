// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __SYSTME_LOCK_H__
#define __SYSTME_LOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

void init_system_lock(void);

void lwip_lock(void);

void lwip_unlock(void);


#ifdef __cplusplus
}
#endif

#endif  // __SYSTME_LOCK_H__

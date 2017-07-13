// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 


#ifndef __VERSION_H__
#define __VERSION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
//Get the whole version string
const char* getDevkitVersion();

//Get major version
uint32_t getMajorVersion();

//Get minor version
uint32_t getMinorVersion();

//Get patch version
uint32_t getPatchVersion();

#ifdef __cplusplus
}
#endif

#endif
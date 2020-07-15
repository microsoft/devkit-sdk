// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 


#ifndef __VERSION_H__
#define __VERSION_H__

#define DEVKIT_MAJOR_VERSION 2
#define DEVKIT_MINOR_VERSION 0
#define DEVKIT_PATCH_VERSION 0
#define DEVKIT_SDK_VERSION (DEVKIT_MAJOR_VERSION * 10000 + DEVKIT_MINOR_VERSION * 100 + DEVKIT_PATCH_VERSION)

#ifdef __cplusplus
extern "C" {
#endif

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

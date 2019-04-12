// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 


#include <stdio.h>
#include "SystemVersion.h"

#ifdef __cplusplus
extern "C" {
#endif

static char version[12] = { "\0" };

const char* getDevkitVersion()
{
    snprintf(version, sizeof(version), "%d.%d.%d", DEVKIT_MAJOR_VERSION, DEVKIT_MINOR_VERSION, DEVKIT_PATCH_VERSION);
    return version;
}


uint32_t getMajorVersion()
{
    return DEVKIT_MAJOR_VERSION;
}


uint32_t getMinorVersion()
{
    return DEVKIT_MINOR_VERSION;
}


uint32_t getPatchVersion()
{
    return DEVKIT_PATCH_VERSION;
}

#ifdef __cplusplus
}
#endif
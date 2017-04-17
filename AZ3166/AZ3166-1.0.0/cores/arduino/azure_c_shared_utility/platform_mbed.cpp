// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/platform.h"
#include "EMW10xxInterface.h"
#include "EEPROMInterface.h"
#include "NTPClient.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio_mbedtls.h"

extern NetworkInterface *network;

static int SetupRealTime(void)
{
    int result;

    NTPClient ntp(*network);
    if (ntp.setTime("0.pool.ntp.org") != 0)
    {
        result = __LINE__;
    }
    else
    {
        result = 0;
    }

    return result;
}

int platform_init(void)
{
    int result = 0;
    
    if(SetupRealTime() != 0)
    {
        result = __LINE__;
    } 
    
    return result;
}

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
    return tlsio_mbedtls_get_interface_description();
}

void platform_deinit(void)
{
}

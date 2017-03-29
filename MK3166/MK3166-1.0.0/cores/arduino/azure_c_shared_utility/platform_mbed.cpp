// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/platform.h"
#include "easy-connect.h"
#include "NTPClient.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio_mbedtls.h"

extern NetworkInterface *network;

int setupRealTime(void)
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
    int result;

    while(true)
    {
        network = easy_connect(true); /* has 1 argument, enable_logging (pass in true to log to serial port) */
        if (!network)
        {
        	printf("Connecting to the network failed... See serial output.\r\n");
            return 1;
        }
        else
        {
        	printf("Wifi connected successfully\n");
        	break;
        }
    }

    if(setupRealTime() != 0)
    {
        result = __LINE__;
    } 
    else
    {
        result = 0;
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

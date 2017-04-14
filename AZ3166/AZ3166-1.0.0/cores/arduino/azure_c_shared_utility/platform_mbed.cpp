// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/platform.h"
#include "EMW10xxInterface.h"
#include "EEPROMInterface.h"
#include "NTPClient.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio_mbedtls.h"

extern NetworkInterface *network;
EMW10xxInterface wifi;

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
    int ret;
    EEPROMInterface eeprom;
    
    uint8_t *pSSID = (uint8_t*)malloc(32);
    int responseCode = eeprom.read(pSSID, 32, 0x03);

    if(responseCode)
    {   
        uint8_t *pPassword = (uint8_t*)malloc(64);
        responseCode = eeprom.read(pPassword, 64, 0x0A);

        if(responseCode)
        {
            ret = wifi.connect( (char*)pSSID, (char*)pPassword, NSAPI_SECURITY_WPA_WPA2, 0 );
        }
        else
        {
            //empty password
            ret = wifi.connect( (char*)pSSID, "" , NSAPI_SECURITY_WPA_WPA2, 0 );          
        }
        network = &wifi;
    }
    else
    {
        printf("unable to get the SSID from EEPROM... Please set the value in configuration mode.\r\n");
        return 1;
    }
    
    if(ret != 0)
    {
      	printf("Connecting to the network failed... See serial output.\r\n");
        return 1;
    }
    else
    {
     	printf("Wifi connected successfully\n");
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

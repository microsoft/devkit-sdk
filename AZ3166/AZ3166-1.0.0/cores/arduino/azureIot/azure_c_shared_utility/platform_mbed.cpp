// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/platform.h"
#include "EMW10xxInterface.h"
#include "EEPROMInterface.h"
#include "SystemWiFi.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio_mbedtls.h"

int platform_init(void)
{
    SyncTime();
    
    // turn on Azure led 
    DigitalOut LedAzure(LED_AZURE);
    LedAzure = 0;
    
    return 0;
}

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
    return tlsio_mbedtls_get_interface_description();
}

void platform_deinit(void)
{
    // turn off Azure led 
    DigitalOut LedAzure(LED_AZURE);
    LedAzure = 1;
}

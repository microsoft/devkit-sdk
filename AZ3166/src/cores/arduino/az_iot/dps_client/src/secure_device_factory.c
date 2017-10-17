// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "azure_hub_modules/secure_device_factory.h"
#include "secure_device_riot.h"

const void* dps_secure_device_interface(void)
{
	return secure_device_riot_interface();
}

SECURE_DEVICE_TYPE dps_secure_device_type(void)
{
    return SECURE_DEVICE_TYPE_X509;
}
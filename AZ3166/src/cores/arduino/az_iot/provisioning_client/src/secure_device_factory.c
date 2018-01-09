// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "azure_prov_client/secure_device_factory.h"
#include "secure_device_riot.h"

const void* hsm_client_x509_interface(void)
{
	return secure_device_riot_interface();
}

SECURE_DEVICE_TYPE prov_dev_security_get_type(void)
{
    return SECURE_DEVICE_TYPE_X509;
}
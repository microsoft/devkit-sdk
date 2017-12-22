// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "azure_prov_client/iothub_security_factory.h"

#define DPS_X509_TYPE 1

IOTHUB_SECURITY_TYPE iothub_security_type()
{
#ifdef DPS_X509_TYPE
    return IOTHUB_SECURITY_TYPE_X509;
#else
    return IOTHUB_SECURITY_TYPE_SAS;
#endif
}
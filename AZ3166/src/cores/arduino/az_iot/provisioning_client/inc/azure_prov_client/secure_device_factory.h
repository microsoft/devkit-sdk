// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef SECURE_DEVICE_FACTORY_H
#define SECURE_DEVICE_FACTORY_H

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif /* __cplusplus */

#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/macro_utils.h"
#include "azure_c_shared_utility/buffer_.h"

#define SECURE_DEVICE_TYPE_VALUES \
    SECURE_DEVICE_TYPE_UNKNOWN,   \
    SECURE_DEVICE_TYPE_TPM,       \
    SECURE_DEVICE_TYPE_X509

DEFINE_ENUM(SECURE_DEVICE_TYPE, SECURE_DEVICE_TYPE_VALUES);

MOCKABLE_FUNCTION(, const void*, hsm_client_x509_interface);
MOCKABLE_FUNCTION(, SECURE_DEVICE_TYPE, prov_dev_security_get_type);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // SECURE_DEVICE_FACTORY_H
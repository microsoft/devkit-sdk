// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef DEVICE_AUTH_H
#define DEVICE_AUTH_H

#ifdef __cplusplus
extern "C" {
#include <cstdint>
#include <cstddef>
#else
#include <stdint.h>
#include <stddef.h>
#endif /* __cplusplus */

#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/macro_utils.h"
#include "hsm_client_data.h"
#include "azure_prov_client/secure_device_factory.h"

MOCKABLE_FUNCTION(, HSM_CLIENT_HANDLE, secure_device_riot_create);
MOCKABLE_FUNCTION(, void, secure_device_riot_destroy, HSM_CLIENT_HANDLE, handle);
MOCKABLE_FUNCTION(, char*, secure_device_riot_get_certificate, HSM_CLIENT_HANDLE, handle);
MOCKABLE_FUNCTION(, char*, secure_device_riot_get_alias_key, HSM_CLIENT_HANDLE, handle);
MOCKABLE_FUNCTION(, char*, secure_device_riot_get_device_cert, HSM_CLIENT_HANDLE, handle);
MOCKABLE_FUNCTION(, char*, secure_device_riot_get_signer_cert, HSM_CLIENT_HANDLE, handle);
MOCKABLE_FUNCTION(, char*, secure_device_riot_get_root_cert, HSM_CLIENT_HANDLE, handle);
MOCKABLE_FUNCTION(, char*, secure_device_riot_get_root_key, HSM_CLIENT_HANDLE, handle);
MOCKABLE_FUNCTION(, char*, secure_device_riot_get_common_name, HSM_CLIENT_HANDLE, handle);

MOCKABLE_FUNCTION(, const HSM_CLIENT_X509_INTERFACE*, secure_device_riot_interface);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // DEVICE_AUTH

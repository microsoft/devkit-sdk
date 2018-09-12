// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdbool.h>
#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/sastoken.h"
#include "azure_c_shared_utility/base64.h"
#include "azure_c_shared_utility/sha.h"
#include "azure_c_shared_utility/urlencode.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/crt_abstractions.h"

#include "hsm_client_tpm.h"
#include "hsm_client_data.h"

#define EPOCH_TIME_T_VALUE          0
#define HMAC_LENGTH                 32
#define TPM_DATA_LENGTH             1024

static const HSM_CLIENT_TPM_INTERFACE tpm_interface = 
{
    hsm_client_tpm_create,
    hsm_client_tpm_destroy,
    hsm_client_tpm_import_key,
    hsm_client_tpm_get_endorsement_key,
    hsm_client_tpm_get_storage_key,
    hsm_client_tpm_sign_data
};

HSM_CLIENT_HANDLE hsm_client_tpm_create()
{
    HSM_CLIENT_HANDLE result;
    /* Codes_SRS_HSM_CLIENT_TPM_07_002: [ On success hsm_client_tpm_create shall allocate a new instance of the secure device tpm interface. ] */
    result = NULL;
    return result;
}

void hsm_client_tpm_destroy(HSM_CLIENT_HANDLE handle)
{
    /* Codes_SRS_HSM_CLIENT_TPM_07_005: [ if handle is NULL, hsm_client_tpm_destroy shall do nothing. ] */
}

int hsm_client_tpm_init(void)
{
    return 0;
}

void hsm_client_tpm_deinit(void)
{
}

const HSM_CLIENT_TPM_INTERFACE* hsm_client_tpm_interface(void)
{
    return &tpm_interface;
}

int hsm_client_tpm_import_key(HSM_CLIENT_HANDLE handle, const unsigned char* key, size_t key_len)
{
    return __FAILURE__;
}

int hsm_client_tpm_get_endorsement_key(HSM_CLIENT_HANDLE handle, unsigned char** key, size_t* key_len)
{
    return __FAILURE__;
}

int hsm_client_tpm_get_storage_key(HSM_CLIENT_HANDLE handle, unsigned char** key, size_t* key_len)
{
    return __FAILURE__;
}

int hsm_client_tpm_sign_data(HSM_CLIENT_HANDLE handle, const unsigned char* data, size_t data_len, unsigned char** signed_value, size_t* signed_len)
{
    return __FAILURE__;
}

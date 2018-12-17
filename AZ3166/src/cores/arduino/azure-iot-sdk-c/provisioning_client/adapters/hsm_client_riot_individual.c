// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/urlencode.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/crt_abstractions.h"

#include "hsm_client_riot_individual.h"
#include "hsm_client_data.h"

#include "RiotCore.h"
#include "RIoT.h"
#include "RiotCrypt.h"
#include "RiotDerEnc.h"
#include "RiotX509Bldr.h"

extern char *RIoTGetDeviceID(unsigned int *);
extern char *RIoTGetAliasKey(unsigned int *);
extern char *RIoTGetAliasCert(unsigned int *);
extern char *RIoTGetDeviceCert(unsigned int *);
extern RIOT_X509_TBS_DATA x509AliasTBSData;
extern RIOT_X509_TBS_DATA x509DeviceTBSData;

typedef struct HSM_CLIENT_X509_INDIVIDUAL_INFO_TAG
{
    // In Riot these are call the device Id pub and pri
    RIOT_ECC_PUBLIC     device_id_pub; 
    RIOT_ECC_PRIVATE    device_id_priv;

    RIOT_ECC_PUBLIC     alias_key_pub;
    RIOT_ECC_PRIVATE    alias_key_priv;

    char* certificate_common_name;

    uint32_t device_id_length;
    char device_id_public_pem[DER_MAX_PEM];

    uint32_t device_signed_length;
    char device_signed_pem[DER_MAX_PEM];

    uint32_t alias_key_length;
    char alias_priv_key_pem[DER_MAX_PEM];

    uint32_t alias_cert_length;
    char alias_cert_pem[DER_MAX_PEM];

    uint32_t root_ca_length;
    char root_ca_pem[DER_MAX_PEM];

    uint32_t root_ca_priv_length;
    char root_ca_priv_pem[DER_MAX_PEM];
    
} HSM_CLIENT_X509_INDIVIDUAL_INFO;

static const HSM_CLIENT_X509_INTERFACE x509_individual_interface =
{
    hsm_client_riot_individual_create,
    hsm_client_riot_individual_destroy,
    hsm_client_riot_individual_get_certificate,
    hsm_client_riot_individual_get_alias_key,
    hsm_client_riot_individual_get_common_name
};


HSM_CLIENT_HANDLE hsm_client_riot_individual_create()
{
    HSM_CLIENT_X509_INDIVIDUAL_INFO* result;
    
    result = malloc(sizeof(HSM_CLIENT_X509_INDIVIDUAL_INFO) );
    if (result == NULL)
    {
        LogError("Failure: malloc HSM_CLIENT_X509_INDIVIDUAL_INFO.");
    }
    else
    {
        memset(result, 0, sizeof(HSM_CLIENT_X509_INDIVIDUAL_INFO));

        // Fill the handle by getting data from RIoTCore
        memcpy(result->device_id_public_pem, RIoTGetDeviceID((unsigned int *)&result->device_id_length), result->device_id_length);
        memcpy(result->alias_priv_key_pem, RIoTGetAliasKey((unsigned int *)&result->alias_key_length), result->alias_key_length);
        memcpy(result->alias_cert_pem, RIoTGetAliasCert((unsigned int *)&result->alias_cert_length), result->alias_cert_length);
        mallocAndStrcpy_s(&result->certificate_common_name, x509AliasTBSData.SubjectCommon);
    }
    return result;
}

void hsm_client_riot_individual_destroy(HSM_CLIENT_HANDLE handle)
{
    if (handle != NULL)
    {
        HSM_CLIENT_X509_INDIVIDUAL_INFO* riot_handle = (HSM_CLIENT_X509_INDIVIDUAL_INFO*)handle;
        free(riot_handle->certificate_common_name);
        free(riot_handle);
    }
}

char* hsm_client_riot_individual_get_certificate(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        LogError("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        HSM_CLIENT_X509_INDIVIDUAL_INFO* riot_handle = (HSM_CLIENT_X509_INDIVIDUAL_INFO*)handle;

        result = malloc(riot_handle->alias_cert_length+1);
        if (result == NULL)
        {
            LogError("Failed to allocate cert buffer.");
        }
        else
        {
            memset(result, 0, riot_handle->alias_cert_length+1);
            memcpy(result, riot_handle->alias_cert_pem, riot_handle->alias_cert_length);
        }
    }
    
    /*
    (void)printf("The riot certificate: \r\n");

    for(int i = 735; i < 810; i++){
        (void)printf("%x\r\n", result[i]);
    }
    */
    return result;
}

char* hsm_client_riot_individual_get_alias_key(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        LogError("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        HSM_CLIENT_X509_INDIVIDUAL_INFO* riot_handle = (HSM_CLIENT_X509_INDIVIDUAL_INFO*)handle;

        if ((result = malloc(riot_handle->alias_key_length+1)) == NULL)
        {
            LogError("Failure allocating registration id.");
        }
        else
        {
            memset(result, 0, riot_handle->alias_key_length+1);
            memcpy(result, riot_handle->alias_priv_key_pem, riot_handle->alias_key_length);
        }
    }
    return result;
}

char* hsm_client_riot_individual_get_common_name(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        LogError("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        HSM_CLIENT_X509_INDIVIDUAL_INFO* riot_handle = (HSM_CLIENT_X509_INDIVIDUAL_INFO*)handle;
        if (mallocAndStrcpy_s(&result, riot_handle->certificate_common_name) != 0)
        {
            LogError("Failure allocating common name.");
            result = NULL;
        }
    }
    return result;
}

const HSM_CLIENT_X509_INTERFACE* hsm_client_x509_individual_interface()
{
    /* Codes_SRS_SECURE_DEVICE_RIOT_07_029: [ hsm_client_x509_individual_interface shall return the SEC_X509_INTERFACE structure. ]*/
    return &x509_individual_interface;
}


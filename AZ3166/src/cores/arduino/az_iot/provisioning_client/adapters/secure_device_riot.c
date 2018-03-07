// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/urlencode.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/crt_abstractions.h"

#include "secure_device_riot.h"

#include "RiotCore.h"
#include "RIoT.h"
#include "RiotCrypt.h"
#include "RiotDerEnc.h"
#include "RiotX509Bldr.h"

typedef struct HSM_CLIENT_X509_INFO_TAG
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
    
} HSM_CLIENT_X509_INFO;

static const HSM_CLIENT_X509_INTERFACE sec_riot_interface =
{
    secure_device_riot_create,
    secure_device_riot_destroy,
    secure_device_riot_get_certificate,
    secure_device_riot_get_alias_key,
    secure_device_riot_get_common_name
};

static char* convert_key_to_string(const unsigned char* key_value, size_t key_length)
{
    char* result;

    result = malloc((key_length*2)+1);
    memset(result, 0, (key_length*2)+1);

    char hex_val[3];
    for (size_t index = 0; index < key_length; index++)
    {
        sprintf(hex_val, "%02x", key_value[index]);
        strcat(result, hex_val);
    }
    return result;
}

extern char *RIoTGetDeviceID(unsigned int *);
extern char *RIoTGetAliasKey(unsigned int *);
extern char *RIoTGetAliasCert(unsigned int *);
extern char *RIoTGetDeviceCert(unsigned int *);
extern RIOT_X509_TBS_DATA x509AliasTBSData;
extern RIOT_X509_TBS_DATA x509DeviceTBSData;

static int produce_device_cert(HSM_CLIENT_X509_INFO* riot_info)
{
	memcpy(riot_info->device_signed_pem, RIoTGetDeviceCert(&riot_info->device_signed_length), riot_info->device_signed_length);
	return 0;
}

static int produce_alias_key_cert(HSM_CLIENT_X509_INFO* riot_info)
{
    memcpy(riot_info->alias_cert_pem, RIoTGetAliasCert(&riot_info->alias_cert_length), riot_info->alias_cert_length);
    return 0;
}

static int produce_alias_key_pair(HSM_CLIENT_X509_INFO* riot_info)
{
	memcpy(riot_info->alias_priv_key_pem, RIoTGetAliasKey(&riot_info->alias_key_length), riot_info->alias_key_length);
	return 0;
}

static int produce_device_id_public(HSM_CLIENT_X509_INFO* riot_info)
{
	memcpy(riot_info->device_id_public_pem, RIoTGetDeviceID(&riot_info->device_id_length), riot_info->device_id_length);
	return 0;
}

static int process_riot_key_info(HSM_CLIENT_X509_INFO* riot_info)
{
	int result;
	
	if (produce_device_id_public(riot_info) != 0)
  {
      LogError("Failure: produce_device_id_public returned invalid result.");
      result = __FAILURE__;
  }
  else if (produce_alias_key_pair(riot_info) != 0)
  {
      LogError("Failure: produce_alias_key_pair returned invalid result.");
      result = __FAILURE__;
  }
  else if (produce_alias_key_cert(riot_info) != 0)
  {
      LogError("Failure: X509MakeAliasCert returned invalid status.");
      result = __FAILURE__;
  }
  else if (produce_device_cert(riot_info) != 0)
  {
      LogError("Failure: X509MakeAliasCert returned invalid status.");
      result = __FAILURE__;
  }
  else if (mallocAndStrcpy_s(&riot_info->certificate_common_name, x509AliasTBSData.SubjectCommon) != 0)
  {
      LogError("Failure: attempting to get common name");
      result = __FAILURE__;
  }
  else
  {
      result = 0;
  }
  return result;
}

HSM_CLIENT_HANDLE secure_device_riot_create()
{
    HSM_CLIENT_X509_INFO* result;
    /* Codes_SRS_SECURE_DEVICE_RIOT_07_001: [ On success secure_device_riot_create shall allocate a new instance of the device auth interface. ] */
    result = malloc(sizeof(HSM_CLIENT_X509_INFO) );
    if (result == NULL)
    {
        /* Codes_SRS_SECURE_DEVICE_RIOT_07_006: [ If any failure is encountered secure_device_riot_create shall return NULL ] */
        LogError("Failure: malloc HSM_CLIENT_X509_INFO.");
    }
    else
    {
        memset(result, 0, sizeof(HSM_CLIENT_X509_INFO));
        if (process_riot_key_info(result) != 0)
        {
            /* Codes_SRS_SECURE_DEVICE_RIOT_07_006: [ If any failure is encountered secure_device_riot_create shall return NULL ] */
            free(result);
            result = NULL;
        }
    }
    return result;
}

void secure_device_riot_destroy(HSM_CLIENT_HANDLE handle)
{
    /* Codes_SRS_SECURE_DEVICE_RIOT_07_007: [ if handle is NULL, secure_device_riot_destroy shall do nothing. ] */
    if (handle != NULL)
    {
        /* Codes_SRS_SECURE_DEVICE_RIOT_07_008: [ secure_device_riot_destroy shall free the HSM_CLIENT_HANDLE instance. ] */
        free(handle->certificate_common_name);
        /* Codes_SRS_SECURE_DEVICE_RIOT_07_009: [ secure_device_riot_destroy shall free all resources allocated in this module. ] */
        free(handle);
    }
}

char* secure_device_riot_get_certificate(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        /* Codes_SRS_SECURE_DEVICE_RIOT_07_010: [ if handle is NULL, secure_device_riot_get_certificate shall return NULL. ] */
        LogError("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        /* Codes_SRS_SECURE_DEVICE_RIOT_07_011: [ secure_device_riot_get_certificate shall allocate a char* to return the riot certificate. ] */
        result = malloc(handle->alias_cert_length+1);
        if (result == NULL)
        {
            /* Codes_SRS_SECURE_DEVICE_RIOT_07_013: [ If any failure is encountered secure_device_riot_get_certificate shall return NULL ] */
            LogError("Failed to allocate cert buffer.");
        }
        else
        {
            /* Codes_SRS_SECURE_DEVICE_RIOT_07_012: [ On success secure_device_riot_get_certificate shall return the riot certificate. ] */
            memset(result, 0, handle->alias_cert_length+1);
            memcpy(result, handle->alias_cert_pem, handle->alias_cert_length);
        }
    }
    
    /*(void)printf("The riot certificate: \r\n");

    for(int i = 735; i < 810; i++){
        (void)printf("%x\r\n", result[i]);
    }*/
    return result;
}

char* secure_device_riot_get_alias_key(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        /* Codes_SRS_SECURE_DEVICE_RIOT_07_014: [ if handle is NULL, secure_device_riot_get_alias_key shall return NULL. ] */
        LogError("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        /* Codes_SRS_SECURE_DEVICE_RIOT_07_015: [ secure_device_riot_get_alias_key shall allocate a char* to return the alias certificate. ] */
        if ((result = malloc(handle->alias_key_length+1)) == NULL)
        {
            /* Codes_SRS_SECURE_DEVICE_RIOT_07_017: [ If any failure is encountered secure_device_riot_get_alias_key shall return NULL ] */
            LogError("Failure allocating registration id.");
        }
        else
        {
            /* Codes_SRS_SECURE_DEVICE_RIOT_07_016: [ On success secure_device_riot_get_alias_key shall return the alias certificate. ] */
            memset(result, 0, handle->alias_key_length+1);
            memcpy(result, handle->alias_priv_key_pem, handle->alias_key_length);
        }
    }
    return result;
}

char* secure_device_riot_get_device_cert(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        /* Codes_SRS_SECURE_DEVICE_RIOT_07_018: [ if handle is NULL, secure_device_riot_get_device_cert shall return NULL. ]*/
        LogError("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        /* Codes_SRS_SECURE_DEVICE_RIOT_07_019: [ secure_device_riot_get_device_cert shall allocate a char* to return the device certificate. ] */
        if ((result = malloc(handle->device_id_length+1)) == NULL)
        {
            /* Codes_SRS_SECURE_DEVICE_RIOT_07_021: [ If any failure is encountered secure_device_riot_get_device_cert shall return NULL ] */
            LogError("Failure allocating registration id.");
        }
        else
        {
            /* Codes_SRS_SECURE_DEVICE_RIOT_07_020: [ On success secure_device_riot_get_device_cert shall return the device certificate. ] */
            memset(result, 0, handle->device_id_length+1);
            memcpy(result, handle->device_id_public_pem, handle->device_id_length);
        }
    }
    return result;
}

char* secure_device_riot_get_signer_cert(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        /* Codes_SRS_SECURE_DEVICE_RIOT_07_022: [ if handle is NULL, secure_device_riot_get_signer_cert shall return NULL. ] */
        LogError("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        /* Codes_SRS_SECURE_DEVICE_RIOT_07_023: [ secure_device_riot_get_signer_cert shall allocate a char* to return the signer certificate. ] */
        if ((result = malloc(handle->device_signed_length + 1)) == NULL)
        {
            /* Codes_SRS_SECURE_DEVICE_RIOT_07_025: [ If any failure is encountered secure_device_riot_get_signer_cert shall return NULL ] */
            LogError("Failure allocating registration id.");
        }
        else
        {
            /* Codes_SRS_SECURE_DEVICE_RIOT_07_024: [ On success secure_device_riot_get_signer_cert shall return the signer certificate. ] */
            memset(result, 0, handle->device_signed_length + 1);
            memcpy(result, handle->device_signed_pem, handle->device_signed_length);
        }
    }
    return result;
}

char* secure_device_riot_get_root_cert(HSM_CLIENT_HANDLE handle)
{
    char* result = "\0";
    return result;
}

char* secure_device_riot_get_root_key(HSM_CLIENT_HANDLE handle)
{
    char* result = "\0";
    return result;
}

char* secure_device_riot_get_common_name(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        /* Codes_SRS_SECURE_DEVICE_RIOT_07_026: [ if handle is NULL, secure_device_riot_get_common_name shall return NULL. ] */
        LogError("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        /* Codes_SRS_SECURE_DEVICE_RIOT_07_027: [ secure_device_riot_get_common_name shall allocate a char* to return the certificate common name. ] */
        if (mallocAndStrcpy_s(&result, handle->certificate_common_name) != 0)
        {
            /* Codes_SRS_SECURE_DEVICE_RIOT_07_028: [ If any failure is encountered secure_device_riot_get_signer_cert shall return NULL ] */
            LogError("Failure allocating common name.");
            result = NULL;
        }
    }
    return result;
}

const HSM_CLIENT_X509_INTERFACE* secure_device_riot_interface()
{
    /* Codes_SRS_SECURE_DEVICE_RIOT_07_029: [ secure_device_riot_interface shall return the SEC_X509_INTERFACE structure. ]*/
    return &sec_riot_interface;
}

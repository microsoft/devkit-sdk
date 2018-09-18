// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <assert.h>
#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/urlencode.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/crt_abstractions.h"

#include "hsm_client_data.h"


char *SYMMETRIC_KEY_VALUE = NULL;
char *REGISTRATION_NAME = NULL;

void free_keys()
{
    if (SYMMETRIC_KEY_VALUE != NULL)
    {
        free(SYMMETRIC_KEY_VALUE);
        SYMMETRIC_KEY_VALUE = NULL;
    }

    if (REGISTRATION_NAME != NULL)
    {
        free(REGISTRATION_NAME);
        REGISTRATION_NAME = NULL;
    }
}

extern void hsm_client_set_registration_name_and_key(const char* name, const char* key) {
    size_t key_length  = strlen(key),
           name_length = strlen(name);

    free_keys();

    SYMMETRIC_KEY_VALUE = (char*) malloc(key_length + 1);
    if (!SYMMETRIC_KEY_VALUE)
    {
        LogError("OOM: hsm_client_set_registration_name_and_key SYMMETRIC_KEY_VALUE(%d bytes)", strlen(key));
        return;
    }
    REGISTRATION_NAME = (char*) malloc(name_length + 1);
    if (!REGISTRATION_NAME)
    {
        free(SYMMETRIC_KEY_VALUE);
        LogError("OOM: hsm_client_set_registration_name_and_key REGISTRATION_NAME(%d bytes)", strlen(name));
        return;
    }
    strcpy(SYMMETRIC_KEY_VALUE, key);
    strcpy(REGISTRATION_NAME, name);

    SYMMETRIC_KEY_VALUE[key_length] = 0;
    REGISTRATION_NAME[name_length] = 0;
}

typedef struct HSM_CLIENT_KEY_INFO_TAG
{
    const char* symmetrical_key;
    const char* registration_name;
} HSM_CLIENT_KEY_INFO;

/*int hsm_client_key_init(void)
{
    return 0;
}

void hsm_client_key_deinit(void)
{
}*/

HSM_CLIENT_HANDLE hsm_client_key_create(void)
{
    HSM_CLIENT_KEY_INFO* result;
    result = malloc(sizeof(HSM_CLIENT_KEY_INFO) );
    if (result == NULL)
    {
        LogError("Failure: malloc HSM_CLIENT_KEY_INFO.");
    }
    else
    {
        if (SYMMETRIC_KEY_VALUE == NULL)
        {
            LogError("Failure: SYMMETRIC_KEY_VALUE is not set.");
        }
        else
        {
            memset(result, 0, sizeof(HSM_CLIENT_KEY_INFO));
            result->symmetrical_key = SYMMETRIC_KEY_VALUE;
            result->registration_name = REGISTRATION_NAME;
        }
    }
    return result;
}

void hsm_client_key_destroy(HSM_CLIENT_HANDLE handle)
{
    if (handle != NULL)
    {
        HSM_CLIENT_KEY_INFO* key_client = (HSM_CLIENT_KEY_INFO*)handle;
        free(key_client);
    }
}

char* hsm_client_get_symmetric_key(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        /* Codes_SRS_HSM_CLIENT_RIOT_07_010: [ if handle is NULL, hsm_client_riot_get_certificate shall return NULL. ] */
        LogError("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        HSM_CLIENT_KEY_INFO* key_client = (HSM_CLIENT_KEY_INFO*)handle;

        if (mallocAndStrcpy_s(&result, key_client->symmetrical_key) != 0)
        {
            LogError("Failed to allocate symmetrical_key.");
            result = NULL;
        }
    }
    return result;
}

char* hsm_client_get_registration_name(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        /* Codes_SRS_HSM_CLIENT_RIOT_07_026: [ if handle is NULL, hsm_client_riot_get_common_name shall return NULL. ] */
        LogError("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        HSM_CLIENT_KEY_INFO* key_client = (HSM_CLIENT_KEY_INFO*)handle;
        if (mallocAndStrcpy_s(&result, key_client->registration_name) != 0)
        {
            LogError("Failure allocating registration_name");
            result = NULL;
        }
    }
    return result;
}

static const HSM_CLIENT_KEY_INTERFACE key_interface =
{
    hsm_client_key_create,
    hsm_client_key_destroy,
    hsm_client_get_symmetric_key,
    hsm_client_get_registration_name
};

const HSM_CLIENT_KEY_INTERFACE* hsm_client_key_interface(void)
{
    return &key_interface;
}

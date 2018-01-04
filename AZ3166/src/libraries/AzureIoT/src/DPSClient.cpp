// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include "mbed.h"
#include "DPSClient.h"
#include "DiceRIoT.h"

#include "iothub_client_version.h"

#include "azure_prov_client/prov_device_client.h"
#include "azure_prov_client/prov_device_ll_client.h"
#include "azure_prov_client/prov_transport_http_client.h"

DEFINE_ENUM_STRINGS(PROV_DEVICE_RESULT, PROV_DEVICE_RESULT_VALUE);
DEFINE_ENUM_STRINGS(PROV_DEVICE_REG_STATUS, PROV_DEVICE_REG_STATUS_VALUES);

typedef struct CLIENT_SAMPLE_INFO_TAG
{
    unsigned int sleep_time;
    char* iothub_uri;
    char* access_key_name;
    char* device_key;
    char* device_id;
    int registration_complete;
} CLIENT_SAMPLE_INFO;

typedef struct IOTHUB_CLIENT_SAMPLE_INFO_TAG
{
    int connected;
    int stop_running;
} IOTHUB_CLIENT_SAMPLE_INFO;

static CLIENT_SAMPLE_INFO user_ctx = { 0 };
bool is_iothub_from_dps = false;
static bool g_trace_on = true;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers
static void registation_status_callback(PROV_DEVICE_REG_STATUS reg_status, void* user_context)
{
    if (user_context == NULL)
    {
        LogError("user_context is NULL");
    }
    else
    {
        LogInfo(">>>Provisioning Status: %s", ENUM_TO_STRING(PROV_DEVICE_REG_STATUS, reg_status));
        if (reg_status == PROV_DEVICE_REG_STATUS_CONNECTED)
        {
            (void)printf("\r\nRegistration status: CONNECTED\r\n");
        }
        else if (reg_status == PROV_DEVICE_REG_STATUS_REGISTERING)
        {
            (void)printf("\r\nRegistration status: REGISTERING\r\n");
        }
        else if (reg_status == PROV_DEVICE_REG_STATUS_ASSIGNING)
        {
            (void)printf("\r\nRegistration status: ASSIGNING\r\n");
        }
    }
}

static void register_device_callback(PROV_DEVICE_RESULT register_result, const char* iothub_uri, const char* device_id, void* user_context)
{
    if (user_context == NULL)
    {
        LogError("user_context is NULL");
    }
    else
    {
        CLIENT_SAMPLE_INFO* user_ctx = (CLIENT_SAMPLE_INFO*)user_context;
        if (register_result == PROV_DEVICE_RESULT_OK)
        {
            (void)printf("Registration Information received from service: %s!\r\n", iothub_uri);
            user_ctx->iothub_uri = strdup(iothub_uri);
            user_ctx->device_id = strdup(device_id);
            user_ctx->registration_complete = 1;
        }
        else
        {
            LogError("Failure encountered on registration!");
            user_ctx->registration_complete = 2;
        }
    }
}

bool __attribute__((section(".riot_fw"))) DPSClientStart(const char* global_prov_uri, const char* id_scope, const char* registration_id, const char* proxy_address, int proxy_port)
{
    if (global_prov_uri == NULL)
    {
        LogError("global_prov_uri is NULL");
        return false;
    }
    if (id_scope == NULL)
    {
        LogError("id_scope is NULL");
        return false;
    }
    
    if (is_iothub_from_dps)
    {
        return true;
    }
    
    if (platform_init() != 0)
    {
        LogError("Failed to initialize the platform.");
        return false;
    }

    memset(&user_ctx, 0, sizeof(CLIENT_SAMPLE_INFO));

    // Set ini
    user_ctx.registration_complete = 0;
    user_ctx.sleep_time = 10;

    LogInfo("   DPS Version: %s\r\n", Prov_Device_GetVersionString());
    LogInfo("Iothub Version: %s\r\n", IoTHubClient_GetVersionString());

    PROV_DEVICE_LL_HANDLE handle = NULL;

    if ((handle = Prov_Device_LL_Create(global_prov_uri, id_scope, Prov_Device_HTTP_Protocol)) == NULL)
    {
        LogError("failed calling Prov_Device_LL_Create");
        return false;
    }

    Prov_Device_LL_SetOption(handle, "logtrace", &g_trace_on);
    if (Prov_Device_LL_SetOption(handle, "TrustedCerts", certificates) != PROV_DEVICE_RESULT_OK)
    {
        LogError("Failed to set option \"TrustedCerts\"");
        Prov_Device_LL_Destroy(handle);
        return false;
    }
    if (proxy_address != NULL)
    {
        HTTP_PROXY_OPTIONS http_proxy;
        http_proxy.host_address = proxy_address;
        http_proxy.port = proxy_port;
        if (Prov_Device_LL_SetOption(handle, OPTION_HTTP_PROXY, &http_proxy) != PROV_DEVICE_RESULT_OK)
        {
            LogError("Failed to set option \"TrustedCerts\"");
            Prov_Device_LL_Destroy(handle);
            return false;
        }
    }

    if (Prov_Device_LL_Register_Device(handle, register_device_callback, &user_ctx, registation_status_callback, &user_ctx) != PROV_DEVICE_RESULT_OK)
    {
        LogError("failed calling Prov_Device_LL_Register_Device");
        Prov_Device_LL_Destroy(handle);
        return false;
    }
    // Waiting the register to be completed
    do
    {
        Prov_Device_LL_DoWork(handle);
        ThreadAPI_Sleep(user_ctx.sleep_time);
    } while (user_ctx.registration_complete == 0);
    // Free DPS client
    Prov_Device_LL_Destroy(handle);
    
    if (user_ctx.registration_complete != 1)
    {
        LogError("registration failed!\r\n");
        return false;
    }
    
    is_iothub_from_dps = true;
    
    return true;
}

char* DPSGetIoTHubURI(void)
{
    return (is_iothub_from_dps ? user_ctx.iothub_uri : NULL);
}

char* DPSGetDeviceID(void)
{
    return (is_iothub_from_dps ? user_ctx.device_id : NULL);
}

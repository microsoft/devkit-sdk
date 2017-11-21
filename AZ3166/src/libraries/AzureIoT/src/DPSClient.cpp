// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include "mbed.h"
#include "DPSClient.h"
#include "DiceRIoT.h"

#include "iothub_client_version.h"

#include "azure_hub_modules/dps_client.h"
#include "azure_hub_modules/dps_transport_http_client.h"

DEFINE_ENUM_STRINGS(DPS_ERROR, DPS_ERROR_VALUES);
DEFINE_ENUM_STRINGS(DPS_REGISTRATION_STATUS, DPS_REGISTRATION_STATUS_VALUES);

typedef struct DPS_CLIENT_SAMPLE_INFO_TAG
{
    char* iothub_uri;
    char* access_key_name;
    char* device_key;
    char* device_id;
    int registration_complete;
} DPS_CLIENT_SAMPLE_INFO;

static DPS_CLIENT_SAMPLE_INFO _dps_client_info = { 0 };

bool is_iothub_from_dps = false;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers
static void dps_registation_status(DPS_REGISTRATION_STATUS reg_status, void* user_context)
{
    if (user_context == NULL)
    {
        LogError("user_context is NULL");
    }
    else
    {
        DPS_CLIENT_SAMPLE_INFO* dps_user_ctx = (DPS_CLIENT_SAMPLE_INFO*)user_context;
        LogInfo(">>>DPS Status: %s", ENUM_TO_STRING(DPS_REGISTRATION_STATUS, reg_status));
        if (reg_status == DPS_REGISTRATION_STATUS_CONNECTED)
        {
        }
        else if (reg_status == DPS_REGISTRATION_STATUS_REGISTERING)
        {
        }
        else if (reg_status == DPS_REGISTRATION_STATUS_ASSIGNING)
        {
        }
    }
}

static void iothub_dps_register_device(DPS_RESULT register_result, const char* iothub_uri, const char* device_id, void* user_context)
{
    if (user_context == NULL)
    {
        LogError("user_context is NULL");
    }
    else
    {
        DPS_CLIENT_SAMPLE_INFO* dps_user_ctx = (DPS_CLIENT_SAMPLE_INFO*)user_context;
        if (register_result == DPS_CLIENT_OK)
        {
            LogInfo(">>>Registration Information received from DPS: %s!", iothub_uri);
            dps_user_ctx->iothub_uri = strdup(iothub_uri);
            dps_user_ctx->device_id = strdup(device_id);
            dps_user_ctx->registration_complete = 1;
        }
        else
        {
            dps_user_ctx->registration_complete = 2;
        }
    }
}

static void on_dps_error_callback(DPS_ERROR error_type, void* user_context)
{
    if (user_context == NULL)
    {
        LogError("user_context is NULL");
    }
    else
    {
        DPS_CLIENT_SAMPLE_INFO* dps_user_ctx = (DPS_CLIENT_SAMPLE_INFO*)user_context;
        LogError("Failure encountered in DPS info %s", ENUM_TO_STRING(DPS_ERROR, error_type));
        dps_user_ctx->registration_complete = 2;
    }
}

bool __attribute__((section(".riot_fw"))) DPSClientStart(const char* dps_uri, const char* dps_scope_id, const char* registration_id, const char* proxy_address, int proxy_port)
{
    if (dps_uri == NULL)
    {
        LogError("dps_uri is NULL");
        return false;
    }
    if (dps_scope_id == NULL)
    {
        LogError("dps_scope_id is NULL");
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

    LogInfo("   DPS Version: %s\r\n", DPS_LL_Get_Version_String());
    LogInfo("Iothub Version: %s\r\n", IoTHubClient_GetVersionString());
    
    // Transfer control to DICE|RIoT
    if(DiceRIoTStart(registration_id) != 0)
    {
        LogError("Untrusted device.");
        return false;
    }
    
    DPS_LL_HANDLE handle = NULL;
    
    memset(&_dps_client_info, 0, sizeof(DPS_CLIENT_SAMPLE_INFO));
    if ((handle = DPS_LL_Create(dps_uri, dps_scope_id, DPS_HTTP_Protocol, on_dps_error_callback, &_dps_client_info)) == NULL)
    {
        LogError("Failed to create DPS DPS_LL_Create");
        return false;
    }

    bool trace_on = true;
    DPS_LL_SetOption(handle, "logtrace", &trace_on);
    if (DPS_LL_SetOption(handle, "TrustedCerts", certificates) != DPS_CLIENT_OK)
    {
        LogError("Failed to set option \"TrustedCerts\"");
        DPS_LL_Destroy(handle);
        return false;
    }
    if (proxy_address != NULL)
    {
        HTTP_PROXY_OPTIONS http_proxy;
        http_proxy.host_address = proxy_address;
        http_proxy.port = proxy_port;
        if (DPS_LL_SetOption(handle, OPTION_HTTP_PROXY, &http_proxy) != DPS_CLIENT_OK)
        {
            LogError("Failed to set option \"TrustedCerts\"");
            DPS_LL_Destroy(handle);
            return false;
        }
    }

    if (DPS_LL_Register_Device(handle, iothub_dps_register_device, &_dps_client_info, dps_registation_status, &_dps_client_info) != DPS_CLIENT_OK)
    {
        LogError("Failed to register device");
        DPS_LL_Destroy(handle);
        return false;
    }
    // Waiting the register to be completed
    do
    {
        DPS_LL_DoWork(handle);
        ThreadAPI_Sleep(10);
    } while (_dps_client_info.registration_complete == 0);
    // Free DPS client
    DPS_LL_Destroy(handle);
    
    if (_dps_client_info.registration_complete != 1)
    {
        LogError("DPS registration failed!\r\n");
        return false;
    }
    
    is_iothub_from_dps = true;
    
    return true;
}

char* DPSGetIoTHubURI(void)
{
    return (is_iothub_from_dps ? _dps_client_info.iothub_uri : NULL);
}

char* DPSGetDeviceID(void)
{
    return (is_iothub_from_dps ? _dps_client_info.device_id : NULL);
}

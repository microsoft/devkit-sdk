// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include "mbed.h"
#include "DevkitDPSClient.h"
#include "DiceCore.h"
#include "EEPROMInterface.h"
#include "RiotCore.h"

#include "iothub_client.h"
#include "hsm_client_key.h"
#include "azure_prov_client/prov_security_factory.h"
#include "azure_prov_client/prov_device_client.h"
#include "azure_prov_client/prov_device_ll_client.h"
#include "azure_prov_client/prov_transport_http_client.h"
#include "azure_prov_client/prov_transport_mqtt_client.h"
#include "azure_prov_client/iothub_security_factory.h"

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
static DPS_AUTH_TYPE g_auth_type = DPS_AUTH_X509_INDIVIDUAL;
bool is_iothub_from_dps = false;
static bool g_trace_on = true;
extern void* __start_riot_core;
extern void* __stop_riot_core;

DICE_DATA DiceData = { 0 };
DICE_CMPND_ID DiceCDI = { DICE_CMPND_TAG , { 0x00 } };
DICE_UDS DiceUDS = { DICE_UDS_TAG, 0 };

void DevkitDPSSetLogTrace(bool ison)
{
    g_trace_on = ison;
}

uint8_t* getUDSBytesFromString(char* udsString)
{
    if (strlen(udsString) != DPS_UDS_MAX_LEN)
    {
        return NULL;
    }
    uint8_t* udsBytes = (uint8_t*)malloc(DPS_UDS_MAX_LEN / 2 + 1);
    char element[2];
    unsigned long int resLeft;
    unsigned long int resRight;

    memset(element, 0, 2);
    for (int i = 0; i < (DPS_UDS_MAX_LEN/2); i++) {
        element[0] = udsString[i * 2];
        resLeft = strtoul(element, NULL, 16);
        element[0] = udsString[i * 2 + 1];
        resRight = strtoul(element, NULL, 16);
        udsBytes[i] = (resLeft << 4) + resRight;
    }

    return udsBytes;
}

char* readUDSString()
{
    uint8_t* udsString = (uint8_t*)malloc(DPS_UDS_MAX_LEN + 1);
    EEPROMInterface eeprom;
    int ret = eeprom.read(udsString, DPS_UDS_MAX_LEN, 0x00, DPS_UDS_ZONE_IDX);

    if (ret < 0)
    {
        LogError("Unable to get DPS UDS string from EEPROM. Please set the value in configuration mode.");
        free(udsString);
        return NULL;
    }
    else if (ret == 0)
    {
        LogError("The DPS UDS string is empty.\r\nPlease set the value in configuration mode.");
        free(udsString);
        return NULL;
    }
    else if (ret < DPS_UDS_MAX_LEN)
    {
        LogError("The length of DPS UDS string must be 64.\r\nPlease set the value with correct length in configuration mode.");
        free(udsString);
        return NULL;
    }
    udsString[DPS_UDS_MAX_LEN] = 0;
    return (char *)udsString;
}

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

char* DiceInit(char* udsString)
{
    if (udsString == NULL)
    {
        udsString = readUDSString();
    }

    uint8_t* udsBytes = getUDSBytesFromString(udsString);
    if (udsBytes == NULL)
    {
        return NULL;
    }
    for (int i = 0; i < DICE_UDS_LENGTH; i++) {
        DiceUDS.bytes[i] = udsBytes[i];
    }
    delete udsBytes;

    // Up-front sanity check
    if (DiceUDS.tag != DICE_UDS_TAG) {
        return NULL;
    }

    // Initialize CDI structure
    memset(&DiceCDI, 0x00, sizeof(DICE_CMPND_ID));
    DiceCDI.tag = DICE_CMPND_TAG;

    // Pointers to protected DICE Data
    DiceData.UDS = &DiceUDS;
    DiceData.CDI = &DiceCDI;

    // Start of RIoT Invariant Code
    DiceData.riotCore = (uint8_t*)&__start_riot_core;

    // Calculate size of RIoT Core
    if((DiceData.riotSize = (uint8_t*)&__stop_riot_core - DiceData.riotCore) == 0){
        return NULL;
    }

    #if logging
        LogInfo("The riot_core start address: %p", &__start_riot_core);
        LogInfo("The riot_core end address: %p", &__stop_riot_core);
    #endif

    return udsString;
}

bool __attribute__((section(".riot_fw"))) DevkitDPSClientStart(const char* global_prov_uri,
    const char* id_scope, const char* registration_id, char* udsString,
    const char* proxy_address, int proxy_port)
{
    bool result = true;
    
    if (global_prov_uri == NULL || id_scope == NULL)
    {
        LogError("invalid parameter global_prov_uri: %p id_scope: %p", global_prov_uri, id_scope);
        result = false;
    }
    else if (is_iothub_from_dps)
    {
        return result; // already've been here
    }
    
    LogInfo("   DPS Version: %s\r\n", Prov_Device_GetVersionString());

    if (g_auth_type == DPS_AUTH_X509_INDIVIDUAL)
    {
        // Initialize DICE
        udsString = DiceInit(udsString);
        if (udsString == NULL)
        {
            LogError("DiceInit failed! Check UDS string provided or set on configuration mode");
            return false;
        }
        
        // Launch protected DICE code. This will measure RIoT Core, derive the
        // CDI value. It must execute with interrupts disabled. Therefore, it
        // must return so we can restore interrupt state.
        if (DiceCore() != 0)
        {
            return false;
        }

        // If DiceCore detects an error condition, it will not enable access to
        // the volatile storage segment. This attempt to transfer control to RIoT
        // will trigger a system reset. We will not be able to proceed.
        // TODO: DETECT WHEN A RESET HAS OCCURRED AND TAKE SOME ACTION.
        if (RiotStart(DiceCDI.bytes, (uint16_t)DICE_DIGEST_LENGTH, registration_id) != 0)
        {
            return false;
        }
    }
    
    if (platform_init() != 0)
    {
        LogError("Failed to initialize the platform.");
        result = false;
    }
    else if ((g_auth_type == DPS_AUTH_X509_INDIVIDUAL || g_auth_type == DPS_AUTH_X509_GROUP) && prov_dev_security_init(SECURE_DEVICE_TYPE_X509) != 0)
    {
        LogError("Failed to initialize the platform.");
        result = false;
    }
    else if (g_auth_type == DPS_AUTH_SYMMETRIC_KEY && prov_dev_security_init(SECURE_DEVICE_TYPE_SYMMETRIC_KEY) != 0)
    {
        LogError("Failed to initialize the platform.");
        result = false;
    }
    else
    {
        memset(&user_ctx, 0, sizeof(CLIENT_SAMPLE_INFO));

        // Set ini
        user_ctx.registration_complete = 0;
        user_ctx.sleep_time = 10;
        
        PROV_DEVICE_LL_HANDLE handle = NULL;

        if ((handle = Prov_Device_LL_Create(global_prov_uri, id_scope, Prov_Device_HTTP_Protocol)) == NULL)
        {
            LogError("failed calling Prov_Device_LL_Create");
            result = false;
        }
        else
        {
            if (proxy_address != NULL)
            {
                HTTP_PROXY_OPTIONS http_proxy;
                http_proxy.host_address = proxy_address;
                http_proxy.port = proxy_port;
                if (Prov_Device_LL_SetOption(handle, OPTION_HTTP_PROXY, &http_proxy) != PROV_DEVICE_RESULT_OK)
                {
                    LogError("Failed to set option \"HTTP Proxy\"");
                    result = false;
                }
            }

            (void)Prov_Device_LL_SetOption(handle, "logtrace", &g_trace_on);
            if (Prov_Device_LL_SetOption(handle, "TrustedCerts", certificates) != PROV_DEVICE_RESULT_OK)
            {
                LogError("Failed to set option \"TrustedCerts\"");
                result = false;
            }
            else if (Prov_Device_LL_Register_Device(handle, register_device_callback, &user_ctx, registation_status_callback, &user_ctx) != PROV_DEVICE_RESULT_OK)
            {
                LogError("failed calling Prov_Device_LL_Register_Device");
                result = false;
            }
            else
            {
                // Waiting the register to be completed
                do
                {
                    Prov_Device_LL_DoWork(handle);
                    ThreadAPI_Sleep(user_ctx.sleep_time);
                } while (user_ctx.registration_complete == 0);
            }
            // Free DPS client
            Prov_Device_LL_Destroy(handle);
        }

        if (user_ctx.registration_complete != 1)
        {
            LogError("registration failed!\r\n");
            result = false;
        }
        else
        {
            is_iothub_from_dps = true;
        }
    }
    return result;
}

void DevkitDPSSetAuthType(DPS_AUTH_TYPE auth_type)
{
    g_auth_type = auth_type;
}

char* DevkitDPSGetIoTHubURI(void)
{
    return (is_iothub_from_dps ? user_ctx.iothub_uri : NULL);
}

char* DevkitDPSGetDeviceID(void)
{
    return (is_iothub_from_dps ? user_ctx.device_id : NULL);
}


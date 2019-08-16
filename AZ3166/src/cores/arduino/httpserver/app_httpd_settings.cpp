#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mbed.h"
#include "SystemWeb.h"
#include "EEPROMInterface.h"
#include "EMW10xxInterface.h"

static bool set_az_iothub(char *value_device_connection_string)
{
    EEPROMInterface eeprom;
    if (eeprom.saveDeviceConnectionString(value_device_connection_string) == 0)
    {
      return true;
    }
    else
    {
      return false;
    }
}

static bool set_az_x509(char *value_x509)
{
    EEPROMInterface eeprom;
    if (eeprom.saveX509Cert(value_x509) == 0)
    {
      return true;
    }
    else
    {
      return false;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Azure IoT hub device settings - connection string and x.509
static int process_az_connection_string(WEB_PAGE_SETTINGS *settings)
{
    if (settings == &az_iot_default_settings || settings == &az_iot_x509_settings)
    {
        // IoT Device Connection string
        if (settings->items[0].value_text == NULL || settings->items[0].value_text[0] == 0)
        {
            settings->items[0].result = SETTING_RESULT_NO_CHANGE;
        }
        else
        {
            if (set_az_iothub(settings->items[0].value_text))
            {
                settings->items[0].result = SETTING_RESULT_SAVED;
            }
            else
            {
                settings->items[0].result = SETTING_RESULT_SAVE_FAILED;
            }
        }
    }
    if (settings == &az_iot_x509_settings)
    {
        // X.509 cert
        if (settings->items[1].value_text == NULL || settings->items[1].value_text[0] == 0)
        {
            settings->items[1].result = SETTING_RESULT_NO_CHANGE;
        }
        else
        {
            printf(settings->items[1].value_text);
            printf("\r\n");
            if (set_az_x509(settings->items[1].value_text))
            {
                settings->items[1].result = SETTING_RESULT_SAVED;
            }
            else
            {
                settings->items[1].result = SETTING_RESULT_SAVE_FAILED;
            }
        }
    }
    return 0;
}

static WEB_PAGE_SETTING_ITEM az_iot_default_items[] = 
{
    {
        "DeviceConnectionString",
        "IoT Device Connection String",
        ITEM_INPUT_TEXT,
        "IoT Device Connection String",
        NULL,
        NULL,
        AZ_IOT_HUB_MAX_LEN,
        0
    }
};

WEB_PAGE_SETTINGS az_iot_default_settings = 
{
    "Azure IoT Settings",
    sizeof(az_iot_default_items) / sizeof(WEB_PAGE_SETTING_ITEM),
    az_iot_default_items,
    process_az_connection_string
};

static WEB_PAGE_SETTING_ITEM az_iot_x509_items[] = 
{
    {
        "DeviceConnectionString",
        "IoT Device Connection String",
        ITEM_INPUT_TEXT,
        "IoT Device Connection String",
        NULL,
        NULL,
        AZ_IOT_HUB_MAX_LEN,
        0
    },
    {
        "Certificate",
        "X.509 Certificate",
        ITEM_INPUT_RICH_TEXT,
        "X.509 Certificate",
        NULL,
        NULL,
        AZ_IOT_X509_MAX_LEN,
        0
    }
};

WEB_PAGE_SETTINGS az_iot_x509_settings = 
{
    "Azure IoT Settings",
    sizeof(az_iot_x509_items) / sizeof(WEB_PAGE_SETTING_ITEM),
    az_iot_x509_items,
    process_az_connection_string
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Azure IoT hub device settings - symmetric key
static int process_az_symmetric_key_string(WEB_PAGE_SETTINGS *settings)
{
    if (settings != &az_iot_symmetric_key_settings)
    {
        return -1;
    }
    
    if (settings->items[0].value_text == NULL || settings->items[0].value_text[0] == 0
        || settings->items[1].value_text == NULL || settings->items[1].value_text[1] == 0
        || settings->items[2].value_text == NULL || settings->items[2].value_text[2] == 0
        || settings->items[3].value_text == NULL || settings->items[3].value_text[3] == 0)
    {
        // One value is empty, the whole symmetric key setting is emtpy
        return 0;
    }

    char *device_connection_string = (char*)calloc(AZ_IOT_HUB_MAX_LEN, 1);
    if (device_connection_string == NULL)
    {
        return -1;
    }
    snprintf(device_connection_string, AZ_IOT_HUB_MAX_LEN, 
             "DPSEndpoint=%s;ScopeId=%s;RegistrationId=%s;SymmetricKey=%s",
             settings->items[0].value_text,
             settings->items[1].value_text,
             settings->items[2].value_text,
             settings->items[3].value_text);
    if (set_az_iothub(device_connection_string))
    {
        settings->items[0].result = SETTING_RESULT_SAVED;
        settings->items[1].result = SETTING_RESULT_SAVED;
        settings->items[2].result = SETTING_RESULT_SAVED;
        settings->items[3].result = SETTING_RESULT_SAVED;
    }
    else
    {
        settings->items[0].result = SETTING_RESULT_SAVE_FAILED;
        settings->items[1].result = SETTING_RESULT_SAVE_FAILED;
        settings->items[2].result = SETTING_RESULT_SAVE_FAILED;
        settings->items[3].result = SETTING_RESULT_SAVE_FAILED;
    }
    free(device_connection_string);
    return 0;
}

static WEB_PAGE_SETTING_ITEM az_iot_symmetric_key_items[] = 
{
    {
        "DPSEndpoint",
        "The DPS endpoint",
        ITEM_INPUT_TEXT,
        "DPS endpoint",
        "global.azure-devices-provisioning.net",
        NULL,
        128,
        0
    },
    {
        "ScopeId",
        "The DPS ID Scope",
        ITEM_INPUT_TEXT,
        "DPS ID Scope",
        NULL,
        NULL,
        32,
        0
    },
    {
        "RegistrationId",
        "The Registration ID",
        ITEM_INPUT_TEXT,
        "Registration ID",
        NULL,
        NULL,
        128,
        0
    },
    {
        "SymmetricKey",
        "The symmetric key",
        ITEM_INPUT_PASSWORD,
        "Symmetric key",
        NULL,
        NULL,
        64,
        0
    }
};

WEB_PAGE_SETTINGS az_iot_symmetric_key_settings = 
{
    "Azure IoT Settings",
    sizeof(az_iot_symmetric_key_items) / sizeof(WEB_PAGE_SETTING_ITEM),
    az_iot_symmetric_key_items,
    process_az_symmetric_key_string
};
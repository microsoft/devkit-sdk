// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "AzureIotHub.h"
#include "Arduino.h"
#include "EEPROMInterface.h"
#include "iothub_client_sample_mqtt.h"

static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

void iothubInit()
{
    srand((unsigned int)time(NULL));

    // Load connection from EEPROM
    EEPROMInterface eeprom;
    uint8_t connString[AZ_IOT_HUB_MAX_LEN + 1] = { '\0' };
    int ret = eeprom.read(connString, AZ_IOT_HUB_MAX_LEN, 0x00, AZ_IOT_HUB_ZONE_IDX);
    if (ret < 0)
    { 
        LogInfo("ERROR: Unable to get the azure iot connection string from EEPROM. Please set the value in configuration mode.");
        return;
    }
    else if (ret == 0)
    {
        LogInfo("INFO: The connection string is empty. Please set the value in configuration mode.");
        return;
    }

    if (platform_init() != 0)
    {
        LogInfo("Failed to initialize the platform.");
        return;
    }

    if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString((char*)connString, MQTT_Protocol)) == NULL)
    {
        LogInfo("iotHubClientHandle is NULL!");
        return;
    }

    if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
    {
        LogInfo("failure to set option \"TrustedCerts\"");
        return;
    }
}

static void sendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
    if (IOTHUB_CLIENT_CONFIRMATION_OK == result)
    {
        LogInfo("Message sent to Azure IoT Hub");
    }
    else
    {
        LogInfo("Failed to send message to Azure IoT Hub");
    }
}

void iothubSendMessage(const unsigned char *text)
{
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray(text, strlen((const char *)text));
    if (messageHandle == NULL)
    {
        LogInfo("unable to create a new IoTHubMessage");
        return;
    }
    LogInfo("Sending message: %s", text);
    if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, sendConfirmationCallback, NULL) != IOTHUB_CLIENT_OK)
    {
        LogInfo("Failed to hand over the message to IoTHubClient");
        return;
    }
    LogInfo("IoTHubClient accepted the message for delivery");
    IoTHubMessage_Destroy(messageHandle);
}

void iothubLoop(void)
{
    IOTHUB_CLIENT_STATUS sendStatusContext;
    unsigned char doWorkLoopCounter = 0;
    do
    {
        IoTHubClient_LL_DoWork(iotHubClientHandle);
        ThreadAPI_Sleep(10);
    } while (++doWorkLoopCounter < 10 && (IoTHubClient_LL_GetSendStatus(iotHubClientHandle, &sendStatusContext) == IOTHUB_CLIENT_OK) && (sendStatusContext == IOTHUB_CLIENT_SEND_STATUS_BUSY));
}

void iothubClose(void)
{
    IoTHubClient_LL_Destroy(iotHubClientHandle);
}
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "Arduino.h"
#include "AzureIotHub.h"
#include "EEPROMInterface.h"
#include "_iothub_client_sample_mqtt.h"

static int callbackCounter;
IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
static int receiveContext = 0;
static int trackingId = 0;

typedef struct EVENT_INSTANCE_TAG
{
    IOTHUB_MESSAGE_HANDLE messageHandle;
    int messageTrackingId; // For tracking the messages within the user callback.
} EVENT_INSTANCE;

static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void *userContextCallback)
{
    int *counter = (int *)userContextCallback;
    const char *buffer;
    size_t size;
    
    // Message content
    if (IoTHubMessage_GetByteArray(message, (const unsigned char **)&buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        (void)Serial.printf("unable to retrieve the message data\r\n");
    }
    else
    {
        (void)Serial.printf("Received Message [%d], Size=%d\r\n", *counter, (int)size);
        _showMessage(buffer, size);
    }

    /* Some device specific action code goes here... */
    (*counter)++;
    return IOTHUBMESSAGE_ACCEPTED;
}

static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
    EVENT_INSTANCE *eventInstance = (EVENT_INSTANCE *)userContextCallback;
    (void)Serial.printf("Confirmation[%d] received for message tracking id = %d with result = %s\r\n", callbackCounter, eventInstance->messageTrackingId, ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
    /* Some device specific action code goes here... */
    callbackCounter++;
    IoTHubMessage_Destroy(eventInstance->messageHandle);
    
    _SendConfirmationCallback();
}

void iothub_client_sample_mqtt_init()
{
    callbackCounter = 0;
    
    srand((unsigned int)time(NULL));
    trackingId = 0;

    // Load connection from EEPROM
    EEPROMInterface eeprom;
    uint8_t connString[AZ_IOT_HUB_MAX_LEN + 1] = { '\0' };
    int ret = eeprom.read(connString, AZ_IOT_HUB_MAX_LEN, 0x00, AZ_IOT_HUB_ZONE_IDX);
    if (ret < 0)
    { 
        (void)Serial.printf("ERROR: Unable to get the azure iot connection string from EEPROM. Please set the value in configuration mode.\r\n");
        return;
    }
    else if (ret == 0)
    {
        (void)Serial.printf("INFO: The connection string is empty.\r\nPlease set the value in configuration mode.\r\n");
    }
    
    if (platform_init() != 0)
    {
        (void)Serial.printf("Failed to initialize the platform.\r\n");
        return;
    }

    if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString((char*)connString, MQTT_Protocol)) == NULL)
    {
        (void)Serial.printf("ERROR: iotHubClientHandle is NULL!\r\n");
        return;
    }
    bool traceOn = false;
    IoTHubClient_LL_SetOption(iotHubClientHandle, "logtrace", &traceOn);
    if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
    {
        (void)Serial.printf("failure to set option \"TrustedCerts\"\r\n");
        return;
    }

    /* Setting Message call back, so we can receive Commands. */
    if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK)
    {
        (void)Serial.printf("ERROR: IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
        return;
    }
}

void iothub_client_sample_send_event(const unsigned char *text)
{
    EVENT_INSTANCE currentMessage;
    currentMessage.messageHandle = IoTHubMessage_CreateFromByteArray(text, strlen((const char*)text));
    currentMessage.messageTrackingId = trackingId++;
    if (currentMessage.messageHandle == NULL) {
        (void)Serial.printf("ERROR: iotHubMessageHandle is NULL!\r\n");
        return;
    }

    MAP_HANDLE propMap = IoTHubMessage_Properties(currentMessage.messageHandle);
    
    char propText[32];
    sprintf_s(propText, sizeof(propText), "PropMsg_%d", currentMessage.messageTrackingId);
    if (Map_AddOrUpdate(propMap, "PropName", propText) != MAP_OK)
    {
         (void)Serial.printf("ERROR: Map_AddOrUpdate Failed!\r\n");
         return;
    }

    if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, currentMessage.messageHandle, SendConfirmationCallback, &currentMessage) != IOTHUB_CLIENT_OK)
    {
        (void)Serial.printf("ERROR: IoTHubClient_LL_SendEventAsync..........FAILED!\r\n");
        return;
    }
    (void)Serial.printf("IoTHubClient_LL_SendEventAsync accepted messagefor transmission to IoT Hub.\r\n");
}

void iothub_client_sample_mqtt_loop(void)
{
    IoTHubClient_LL_DoWork(iotHubClientHandle);
    ThreadAPI_Sleep(1);
}

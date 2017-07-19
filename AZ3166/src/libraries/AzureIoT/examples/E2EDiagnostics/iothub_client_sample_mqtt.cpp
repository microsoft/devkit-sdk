// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include <time.h>

#include "AzureIotHub.h"
#include "Arduino.h"
#include "config.h"
#include "iothub_client_sample_mqtt.h"
#include "json.h"
#include "Sensor.h"

static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
static bool messagePending = false;
static bool messageSending = true;
static DevI2C *ext_i2c;
static HTS221Sensor *ht_sensor;
static int interval = INTERVAL;

int getInterval()
{
    return interval;
}

static IOTHUBMESSAGE_DISPOSITION_RESULT c2dMessageCallback(IOTHUB_MESSAGE_HANDLE message, void *userContextCallback)
{
    const char *buffer;
    size_t size;

    if (IoTHubMessage_GetByteArray(message, (const unsigned char **)&buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        LogError("IoTHubMessage_GetByteArray Failed");
        return IOTHUBMESSAGE_REJECTED;
    }
    else
    {
        char *temp = (char *)malloc(size + 1);
        if (temp == NULL)
        {
            LogError("Failed to malloc for command");
            return IOTHUBMESSAGE_REJECTED;
        }
        memcpy(temp, buffer, size);
        temp[size] = '\0';
        LogInfo("Receive C2D message: %s", temp);
        showC2DMessageReceived();
        free(temp);
        return IOTHUBMESSAGE_ACCEPTED;
    }
}

void sensorInit()
{
    ext_i2c = new DevI2C(D14, D15);
    ht_sensor = new HTS221Sensor(*ext_i2c);
    ht_sensor->init(NULL);
}

void readMessage(int messageId, char *payload)
{
    ht_sensor->reset();
    float temperature = 0;
    ht_sensor->getTemperature(&temperature);
    temperature = temperature * 1.8 + 32;
    float humidity = 0;
    ht_sensor->getHumidity(&humidity);
    if (temperature <= TEMPERATURE_F_MAX)
    {
        char *root = "{"
                     "\"deviceId\": \"%s\","
                     "\"messageId\": \"%d\","
                     "\"humidity\": \"%d\","
                     "\"temperature\": \"%d\""
                     "}";
        sprintf(payload, root, DEVICE_ID, messageId, (int)humidity, (int)temperature);
    }
    else
    {
        char *root = "{"
                     "\"deviceId\": \"%s\","
                     "\"messageId\": \"%d\","
                     "\"humidity\": \"%d\""
                     "}";
        sprintf(payload, root, DEVICE_ID, messageId, (int)humidity);
    }
}

void showC2DMessageReceived()
{
    Screen.print(2, "C2D msg received");
}

void showSendConfirmation()
{
    Screen.print(2, "Msg sent ok");
}

void parseTwinMessage(const char *message)
{
    json_object *rootObject;
    json_object *desiredObject;
    json_object *intervalObject;
    if (message == NULL || (rootObject = json_tokener_parse(message)) == NULL)
    {
        LogError("Parse %s failed", message);
        return;
    }
    if ((desiredObject = json_object_object_get(rootObject, "desired")) != NULL)
    {
        if ((intervalObject = json_object_object_get(desiredObject, "interval")) != NULL || (intervalObject = json_object_object_get(rootObject, "interval")) != NULL)
        {
            const char *intervalString;
            if ((intervalString = json_object_get_string(intervalObject)) != NULL)
            {
                interval = atoi(intervalString);
            }
        }
    }
}
static void twinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payLoad, size_t size, void *userContextCallback)
{
    char *temp = (char *)malloc(size + 1);
    for (int i = 0; i < size; i++)
    {
        temp[i] = (char)(payLoad[i]);
    }
    temp[size] = '\0';
    parseTwinMessage(temp);
    free(temp);
}

void iothubInit()
{
    srand((unsigned int)time(NULL));

    if (platform_init() != 0)
    {
        LogError("Failed to initialize the platform.");
        return;
    }

    if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString_WithDiagnostics(SAMPLING_SERVER, 0, CONNECTION_STRING, MQTT_Protocol)) == NULL)
    {
        LogError("Error: iotHubClientHandle is NULL!");
        return;
    }

    if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
    {
        LogError("Failed to set option \"TrustedCerts\"");
        return;
    }

    if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, c2dMessageCallback, NULL) != IOTHUB_CLIENT_OK)
    {
        LogError("IoTHubClient_LL_SetMessageCallback FAILED!");
        return;
    }

    if (IoTHubClient_LL_SetDeviceTwinCallback_WithDiagnostics(iotHubClientHandle, twinCallback, NULL) != IOTHUB_CLIENT_OK)
    {
        LogError("Failed on IoTHubClient_LL_SetDeviceTwinCallback");
        return;
    }
}

static void sendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
    if (IOTHUB_CLIENT_CONFIRMATION_OK == result)
    {
        LogInfo("Message sent to Azure IoT Hub");
        showSendConfirmation();
    }
    else
    {
        LogInfo("Failed to send message to Azure IoT Hub");
    }
    messagePending = false;
}

void iothubSendMessage(const unsigned char *text)
{
    if (messageSending && !messagePending)
    {
        IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray(text, strlen((const char *)text));
        if (messageHandle == NULL)
        {
            LogError("Failed to create a new IoTHubMessage");
            return;
        }

        LogInfo("Sending message: %s", text);
        if (IoTHubClient_LL_SendEventAsync_WithDiagnostics(iotHubClientHandle, messageHandle, sendConfirmationCallback, NULL) != IOTHUB_CLIENT_OK)
        {
            LogError("Failed to hand over the message to IoTHubClient");
            return;
        }
        LogInfo("IoTHubClient accepted the message for delivery");
        messagePending = true;
        IoTHubMessage_Destroy(messageHandle);
        delay(getInterval());
    }
}

void iothubLoop(void)
{
    IoTHubClient_LL_DoWork(iotHubClientHandle);
}

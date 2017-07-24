// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include <time.h>

#include "AzureIotHub.h"
#include "Arduino.h"
#include "config.h"
#include "EEPROMInterface.h"
#include "iothub_client_sample_mqtt.h"
#include "json.h"
#include "Sensor.h"

const int INTERVAL = 3000;
const int MESSAGE_MAX_LEN = 256;
const int TEMPERATURE_F_MAX = 86;

static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
static bool messagePending = false;
static bool messageSending = true;
static DevI2C *ext_i2c;
static HTS221Sensor *ht_sensor;
static int interval = INTERVAL;
static int messageCount = 0;

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
                     "\"messageId\": \"%d\","
                     "\"humidity\": \"%d\","
                     "\"temperature\": \"%d\""
                     "}";
        sprintf(payload, root, messageId, (int)humidity, (int)temperature);
    }
    else
    {
        char *root = "{"
                     "\"messageId\": \"%d\","
                     "\"humidity\": \"%d\""
                     "}";
        sprintf(payload, root, messageId, (int)humidity);
    }
}

void showC2DMessageReceived()
{
    Screen.print(2, "C2D msg received");
}

void showSendConfirmation()
{
    char message[25];
    sprintf(message, "Msg %d sent", messageCount-1);
    Screen.print(2, message);
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
    if(temp == NULL)
    {
        LogError("Failed to malloc for twinCallback.");
        return;
    }
    memcpy(temp, payLoad, size);
    temp[size] = '\0';
    parseTwinMessage(temp);
    free(temp);
}

bool iothubInit()
{
    srand((unsigned int)time(NULL));

    EEPROMInterface eeprom;
    uint8_t connString[AZ_IOT_HUB_MAX_LEN + 1] = { '\0' };
    int ret = eeprom.read(connString, AZ_IOT_HUB_MAX_LEN, 0x00, AZ_IOT_HUB_ZONE_IDX);
    if (ret < 0)
    {
        LogError("Unable to get the azure iot connection string from EEPROM. Please set the value in configuration mode.");
        return false;
    }
    else if (ret == 0)
    {
        LogError("The connection string is empty. Please set the value in configuration mode.");
        return false;
    }

    if (platform_init() != 0)
    {
        LogError("Failed to initialize the platform.");
        return false;
    }

    if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString_WithDiagnostics(SAMPLING_SERVER, 0, (const char*)connString, MQTT_Protocol)) == NULL)
    {
        LogError("Error: iotHubClientHandle is NULL!");
        return false;
    }

    if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
    {
        LogError("Failed to set option \"TrustedCerts\"");
        return false;
    }

    if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, c2dMessageCallback, NULL) != IOTHUB_CLIENT_OK)
    {
        LogError("IoTHubClient_LL_SetMessageCallback FAILED!");
        return false;
    }

    if (IoTHubClient_LL_SetDeviceTwinCallback_WithDiagnostics(iotHubClientHandle, twinCallback, NULL) != IOTHUB_CLIENT_OK)
    {
        LogError("Failed on IoTHubClient_LL_SetDeviceTwinCallback");
        return false;
    }
    return true;
}

static void sendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
    if (result == IOTHUB_CLIENT_CONFIRMATION_OK)
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

void iothubSendMessage(void)
{
    if (messageSending && !messagePending)
    {
        char text[MESSAGE_MAX_LEN];
        readMessage(messageCount++, text);
       
        IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char *)text, strlen((const char *)text));
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
        delay(interval);
    }
}

void iothubLoop(void)
{
    IoTHubClient_LL_DoWork(iotHubClientHandle);
}

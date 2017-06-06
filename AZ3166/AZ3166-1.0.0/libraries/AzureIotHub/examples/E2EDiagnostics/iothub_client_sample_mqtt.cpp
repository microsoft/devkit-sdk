// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <time.h>

#include "AzureIotHub.h"
#include "Arduino.h"
#include "config.h"
#include "iothub_client_sample_mqtt.h"
#include "json.h"

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

/*
 * As there is a problem of sprintf %f in Arduino,
   follow https://github.com/blynkkk/blynk-library/issues/14 to implement dtostrf
 */
char *dtostrf(double number, signed char width, unsigned char prec, char *s)
{
    if (isnan(number))
    {
        strcpy(s, "nan");
        return s;
    }
    if (isinf(number))
    {
        strcpy(s, "inf");
        return s;
    }

    if (number > 4294967040.0 || number < -4294967040.0)
    {
        strcpy(s, "ovf");
        return s;
    }
    char *out = s;
    // Handle negative numbers
    if (number < 0.0)
    {
        *out = '-';
        ++out;
        number = -number;
    }
    // Round correctly so that print(1.999, 2) prints as "2.00"
    double rounding = 0.5;
    for (uint8_t i = 0; i < prec; ++i)
        rounding /= 10.0;
    number += rounding;

    // Extract the integer part of the number and print it
    unsigned long int_part = (unsigned long)number;
    double remainder = number - (double)int_part;
    out += sprintf(out, "%d", int_part);

    // Print the decimal point, but only if there are digits beyond
    if (prec > 0)
    {
        *out = '.';
        ++out;
    }

    while (prec-- > 0)
    {
        remainder *= 10.0;
        if ((int)remainder == 0)
        {
            *out = '0';
            ++out;
        }
    }
    sprintf(out, "%d", (int)remainder);
    return s;
}

char *f2s(float f, int p)
{
    char *pBuff;
    const int iSize = 10;
    static char sBuff[iSize][20];
    static int iCount = 0;
    pBuff = sBuff[iCount];
    if (iCount >= iSize - 1)
    {
        iCount = 0;
    }
    else
    {
        iCount++;
    }
    return dtostrf(f, 0, p, pBuff);
}

static IOTHUBMESSAGE_DISPOSITION_RESULT c2dMessageCallback(IOTHUB_MESSAGE_HANDLE message, void *userContextCallback)
{
    const char *buffer;
    size_t size;

    if (IoTHubMessage_GetByteArray(message, (const unsigned char **)&buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        LogInfo("unable to IoTHubMessage_GetByteArray");
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
                     "\"humidity\": \"%s\","
                     "\"temperature\": \"%s\""
                     "}";
        snprintf(payload, MESSAGE_MAX_LEN, root, DEVICE_ID, messageId, f2s(humidity, 1), f2s(temperature, 1));
    }
    else
    {
        char *root = "{"
                     "\"deviceId\": \"%s\","
                     "\"messageId\": \"%d\","
                     "\"humidity\": \"%s\""
                     "}";
        sprintf(payload, root, DEVICE_ID, messageId, f2s(humidity, 1));
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
        LogError("parse %s failed", message);
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
        LogInfo("Failed to initialize the platform.");
        return;
    }

    if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString_WithDiagnostics(SAMPLING_SERVER, 0, CONNECTION_STRING, MQTT_Protocol)) == NULL)
    {
        LogInfo("iotHubClientHandle is NULL!");
        return;
    }

    if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
    {
        LogInfo("failure to set option \"TrustedCerts\"");
        return;
    }

    if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, c2dMessageCallback, NULL) != IOTHUB_CLIENT_OK)
    {
        LogInfo("IoTHubClient_LL_SetMessageCallback FAILED!");
        return;
    }

    if (IoTHubClient_LL_SetDeviceTwinCallback_WithDiagnostics(iotHubClientHandle, twinCallback, NULL) != IOTHUB_CLIENT_OK)
    {
        LogInfo("Failed on IoTHubClient_LL_SetDeviceTwinCallback");
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
            LogInfo("unable to create a new IoTHubMessage");
            return;
        }

        LogInfo("Sending message: %s", text);
        if (IoTHubClient_LL_SendEventAsync_WithDiagnostics(iotHubClientHandle, messageHandle, sendConfirmationCallback, NULL) != IOTHUB_CLIENT_OK)
        {
            LogInfo("Failed to hand over the message to IoTHubClient");
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

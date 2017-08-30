// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "HTS221Sensor.h"
#include "AzureIotHub.h"
#include "Arduino.h"
#include "json_object.h"
#include "json_tokener.h"
#include <assert.h>
#include "config.h"
#include "RGB_LED.h"

#define RGB_LED_BRIGHTNESS 32

DevI2C *i2c;
HTS221Sensor *sensor;
static RGB_LED rgbLed;
static int interval = INTERVAL;

int getInterval()
{
    return interval;
}

void blinkLED()
{
    rgbLed.turnOff();
    rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
    delay(500);
    rgbLed.turnOff();
}

void blinkSendConfirmation()
{
    rgbLed.turnOff();
    rgbLed.setColor(0, 0, RGB_LED_BRIGHTNESS);
    delay(500);
    rgbLed.turnOff();
}

void parseTwinMessage(const char *message)
{
    json_object *jsonObject, *desiredObject, *intervalObject;
	if ((jsonObject = json_tokener_parse(message)) != NULL) {
        bool success = false;
        json_object_object_get_ex(jsonObject, "desired", &desiredObject);
        if (desiredObject != NULL && json_object_get_type(desiredObject) == json_type_object)
        {
            json_object_object_get_ex(desiredObject, "interval", &intervalObject);
            if (intervalObject != NULL)
            {
                interval = json_object_get_int(intervalObject);
                success = true;
            }
        }
        if (success == false) {
            json_object_object_get_ex(jsonObject, "interval", &intervalObject);
            if (intervalObject != NULL)
            {
                interval = json_object_get_int(intervalObject);
            }
        }
    }
    else
    {
        LogError("parse %s failed", message);
        return;
    }
    json_object_put(jsonObject);
}

void SensorInit()
{
    i2c = new DevI2C(D14, D15);
    sensor = new HTS221Sensor(*i2c);
    sensor->init(NULL);
}

float readTemperature()
{
    sensor->reset();

    float temperature = 0;
    sensor->getTemperature(&temperature);

    return temperature;
}

float readHumidity()
{
    sensor->reset();

    float humidity = 0;
    sensor->getHumidity(&humidity);

    return humidity;
}

bool readMessage(int messageId, char *payload)
{
    json_object *jsonObject, *tmpObject;
    jsonObject = json_object_new_object();
    json_object_object_add(jsonObject, "deviceId", json_object_new_string(DEVICE_ID));
    json_object_object_add(jsonObject, "messageId", json_object_new_int(messageId));

    float temperature = readTemperature();
    float humidity = readHumidity();
    bool temperatureAlert = false;

    if(temperature != temperature)
    {
        json_object_object_add(jsonObject, "temperature", NULL);
    }
    else
    {
        json_object_object_add(jsonObject, "temperature", json_object_new_string(f2s(temperature, 2)));
        if(temperature > TEMPERATURE_ALERT)
        {
            temperatureAlert = true;
        }
    }

    if(humidity != humidity)
    {
        json_object_object_add(jsonObject, "humidity", NULL);
    }
    else
    {
        json_object_object_add(jsonObject, "humidity", json_object_new_string(f2s(humidity, 2)));
    }
    snprintf(payload, MESSAGE_MAX_LEN, "%s", json_object_to_json_string(jsonObject));
    json_object_put(jsonObject);
    return temperatureAlert;
}
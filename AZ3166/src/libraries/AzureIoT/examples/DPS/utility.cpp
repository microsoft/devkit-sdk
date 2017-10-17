// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "AzureIotHub.h"
#include "Arduino.h"
#include "parson.h"
#include <assert.h>
#include "config.h"

void parseTwinMessage(DEVICE_TWIN_UPDATE_STATE updateState, const char *message)
{
    Serial.println(message);

    JSON_Value *root_value;
    root_value = json_parse_string(message);
    if (json_value_get_type(root_value) != JSONObject)
    {
        if (root_value != NULL)
        {
            json_value_free(root_value);
        }
        LogError("parse %s failed", message);
        return;
    }
    JSON_Object *root_object = json_value_get_object(root_value);

    const char * luisAppUrl = NULL;
    if (updateState == DEVICE_TWIN_UPDATE_COMPLETE)
    {
        JSON_Object *desired_object = json_object_get_object(root_object, "desired");
        if (desired_object != NULL)
        {
            luisAppUrl = json_object_get_string(desired_object, "luisappurl");
        }
    }
    else
    {
        luisAppUrl = json_object_get_string(root_object, "luisappurl");
    }

    if (luisAppUrl != NULL)
    {
        LogInfo(">>>Get LUIS request URL: %s", luisAppUrl);
    }

    json_value_free(root_value);
}

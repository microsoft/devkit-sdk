// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "AzureIotHub.h"
#include "Arduino.h"
#include "parson.h"
#include <assert.h>
#include "config.h"
#include "utility.h"

const char * parseTwinMessage(DEVICE_TWIN_UPDATE_STATE updateState, const char *message)
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
        return NULL;
    }
    JSON_Object *root_object = json_value_get_object(root_value);

    const char * LUIS_App_Url;
    if (updateState == DEVICE_TWIN_UPDATE_COMPLETE)
    {
        JSON_Object *desired_object = json_object_get_object(root_object, "desired");
        if (desired_object != NULL)
        {
            LUIS_App_Url = json_object_get_string(desired_object, "luisappurl");
        }
    }
    else
    {
        LUIS_App_Url = json_object_get_string(root_object, "luisappurl");
    }

    if (LUIS_App_Url != NULL)
    {
        LogInfo(">>>Get LUIS request URL: %s", LUIS_App_Url);
    }

    json_value_free(root_value);
    return LUIS_App_Url;
}

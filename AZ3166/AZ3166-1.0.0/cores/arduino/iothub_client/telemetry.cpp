// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <json.h>
#include "http_client.h"

#include "Queue.h"
#include "Thread.h"

#ifdef _cplusplus
extern "C" {
#endif

#include "azure_c_shared_utility/base64.h"
#include "azure_c_shared_utility/hmacsha256.h"
#include "azure_c_shared_utility/xlogging.h"
#include "Arduino.h"
#include "telemetry.h"

#define AI_ENDPOINT "https://dc.services.visualstudio.com/v2/track"
#define IKEY "f0d75de5-4331-4033-8bb2-bf96f435d184"
#define AI_EVENT "AIEVENT"
#define KEYWORD "DevKit"
#define EVENT_NAME "setup"
#define HARDWARE_VERSION "1.0.0"
#define MCU_TYPE "STM32F412"
#define STACK_SIZE 0x4000
#define MAX_MESSAGE_SIZE 128

NetworkInterface *network;
static const unsigned char DEVKITMACHASH[] = "dev_init_hash";
static Thread ai_thread(osPriorityNormal, STACK_SIZE, NULL);

struct Telemetry
{
    const char *iothub;
    const char *event;
    const char *message;
};

static Queue<Telemetry, 16> queue;

void telemetry_enqueue(const char *iothub, const char *event, const char *message)
{
    struct Telemetry *telemetry = (struct Telemetry *)malloc(sizeof(struct Telemetry));
    telemetry->iothub = (const char *)malloc(sizeof(iothub));
    telemetry->iothub = iothub;
    telemetry->event = (const char *)malloc(sizeof(event));
    telemetry->event = event;
    telemetry->message = (const char *)malloc(sizeof(message));
    telemetry->message = message;

    if (queue.put(telemetry) != 0)
    {
        LogError("error to enqueue telemetry with event: %s, iothub : %s, message: %s", event, iothub, message);
    }
}

static void trace_telemetry()
{
    HTTPClient *request;
    struct Telemetry *telemetry = (struct Telemetry *)malloc(sizeof(struct Telemetry));
    telemetry->iothub = (const char *)malloc(sizeof(MAX_MESSAGE_SIZE));
    telemetry->event = (const char *)malloc(sizeof(MAX_MESSAGE_SIZE));
    telemetry->message = (const char *)malloc(sizeof(MAX_MESSAGE_SIZE));
    struct json_object *body, *data, *base, *prop;
    BUFFER_HANDLE hash;

    while (true)
    {
        osEvent evt = queue.get();
        if (evt.status != osEventMessage)
        {
            continue;
        }

        telemetry = (Telemetry *)evt.value.p;
        body = json_tokener_parse("{\"data\":{\"baseType\":\"EventData\",\"baseData\":{\"properties\":{}}}}");
        json_object_object_get_ex(body, "data", &data);
        json_object_object_get_ex(data, "baseData", &base);
        json_object_object_get_ex(base, "properties", &prop);
        json_object_object_add_ex(base, "name", json_object_new_string(telemetry->event), 0);
        json_object_object_add_ex(prop, "keyword", json_object_new_string(KEYWORD), 0);
        json_object_object_add_ex(prop, "hardware_version", json_object_new_string(HARDWARE_VERSION), 0);
        json_object_object_add_ex(prop, "mcu", json_object_new_string(MCU_TYPE), 0);
        json_object_object_add_ex(prop, "message", json_object_new_string(telemetry->message), 0);

        hash = BUFFER_new();
        const unsigned char *key = reinterpret_cast<const unsigned char *>(network->get_mac_address());
        HMACSHA256_RESULT result = HMACSHA256_ComputeHash(key, sizeof(key) - 1, DEVKITMACHASH, sizeof(DEVKITMACHASH) - 1, hash);
        const char *s1 = STRING_c_str(Base64_Encode(hash));
        json_object_object_add_ex(prop, "mac_address", json_object_new_string(s1), 0);

        key = reinterpret_cast<const unsigned char *>(telemetry->iothub);
        hash = BUFFER_new();
        result = HMACSHA256_ComputeHash(key, sizeof(key) - 1, DEVKITMACHASH, sizeof(DEVKITMACHASH) - 1, hash);
        s1 = STRING_c_str(Base64_Encode(hash));
        json_object_object_add_ex(prop, "iothub_name", json_object_new_string(s1), 0);

        time_t t;
        time(&t);
        json_object_object_add_ex(body, "time", json_object_new_string(ctime(&t)), 0);
        json_object_object_add_ex(body, "name", json_object_new_string(AI_EVENT), 0);
        json_object_object_add_ex(body, "iKey", json_object_new_string(IKEY), 0);

        const char *serialized_body = json_object_to_json_string_ext(body, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);

        request = new HTTPClient(HTTP_POST, AI_ENDPOINT);
        request->set_header("Content-Type", "application/json");
        request->set_header("Connection", "keep-alive");
        Http_Response *_response = request->send(serialized_body, strlen(serialized_body));
        LogInfo("request body :%s", serialized_body);

        LogInfo("Got response message: <%s>.", _response->status_message);
        delete request;
    }
}

void telemetry_init()
{
    int ret = 0;
    int retry = 0;
    do
    {
        ret = ai_thread.start(trace_telemetry);
    } while (ret != 0 && retry < 3);

    if (ret != 0)
    {
        LogError("telemetry start failed after retry 3 times");
    }
}

#ifdef _cplusplus
}
#endif
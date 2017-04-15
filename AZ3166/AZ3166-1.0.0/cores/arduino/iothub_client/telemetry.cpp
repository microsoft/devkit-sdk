// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "http_client.h"
#include "Queue.h"
#include "Thread.h"
#include "md5.h"
#include "base64.h"

#ifdef _cplusplus
extern "C" {
#endif

#include "Arduino.h"
#include "telemetry.h"
#include "azure_c_shared_utility/xlogging.h"

#define AI_ENDPOINT "https://dc.services.visualstudio.com/v2/track"
#define IKEY "f0d75de5-4331-4033-8bb2-bf96f435d184"
#define AI_EVENT "AIEVENT"
#define KEYWORD "AZ3166"
#define HARDWARE_VERSION "1.0.0"
#define MCU_TYPE "STM32F412"
#define STACK_SIZE 0x2000
#define IOTHUB_NAME_MAX_LEN 50
#define MAX_MESSAGE_SIZE 256
#define TEMPLATE "{\"data\": {\"baseType\": \"EventData\",\"baseData\": {\"properties\": "        \
                 "{\"keyword\": \"%s\",\"hardware_version\": \"%s\",\"mcu\": \"%s\",\"message\":" \
                 "\"%s\",\"mac_address\": \"%s\",\"iothub_name\":\"%s\"},"                        \
                 "\"name\": \"%s\"}},\"time\": \"%s\",\"name\": \"%s\",\"iKey\": \"%s\"}"
NetworkInterface *network;
static Thread ai_thread(osPriorityNormal, STACK_SIZE, NULL);
static HTTPClient *client;
static Http_Response *_response;

struct Telemetry
{
    char iothub[IOTHUB_NAME_MAX_LEN + 1];
    char event[MAX_MESSAGE_SIZE];
    char message[MAX_MESSAGE_SIZE];
};

static Queue<Telemetry, 16> queue;

void telemetry_enqueue(const char *iothub, const char *event, const char *message)
{
    if(strlen(iothub) > IOTHUB_NAME_MAX_LEN || strlen(event) > MAX_MESSAGE_SIZE || strlen(message) > MAX_MESSAGE_SIZE)
    {
        LogError("telemetry message exceeds the max length permitted. IotHub %s, event %s, message %s", iothub, event, message);
        return;
    }
    struct Telemetry *telemetry = (Telemetry *)malloc(sizeof(struct Telemetry));
    strcpy(telemetry->iothub, iothub);
    strcpy(telemetry->event, event);
    strcpy(telemetry->message, message);

    if (queue.put(telemetry) != 0)
    {
        LogError("error to enqueue telemetry with event: %s, iothub : %s, message: %s", event, iothub, message);
    }
    LogInfo("enqueue OK");
}

static void trace_telemetry()
{
    osEvent evt;
    char mac_addr[18];
    unsigned char output[16];
    char serialized_body[1024];
    unsigned char mac_buffer[128];
    unsigned char iot_buffer[128];
    unsigned char temp_buffer[128];
    size_t len;
    char *_ctime;
    time_t t;

    while (true)
    {
        evt = queue.get();
        if (evt.status != osEventMessage)
        {
            continue;
        }

        time(&t);
        _ctime = (char *)ctime(&t);
        _ctime[strlen(_ctime) - 1] = 0;

        struct Telemetry *telemetry = (Telemetry *)evt.value.p;
        strcpy(mac_addr, network->get_mac_address());
        memcpy(temp_buffer, mac_addr, sizeof(mac_addr));
        mbedtls_md5(temp_buffer, strlen(mac_addr), output);
        mbedtls_base64_encode(mac_buffer, sizeof(mac_buffer), &len, output, 64);
        memcpy(temp_buffer, telemetry->iothub, sizeof(telemetry->iothub));
        mbedtls_md5(temp_buffer, strlen(telemetry->iothub), output);
        mbedtls_base64_encode(iot_buffer, sizeof(iot_buffer), &len, output, 64);

        sprintf(serialized_body, TEMPLATE, KEYWORD, HARDWARE_VERSION, MCU_TYPE, telemetry->message,
                mac_buffer, iot_buffer, telemetry->event, _ctime, AI_EVENT, IKEY);

        _response = client->send(serialized_body, strlen(serialized_body));
        if (_response->status_code != 200)
        {
            LogError("Response error: <%d>, <%s>.", _response->status_code, _response->status_message);
        }

        free(telemetry);
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
        return;
    }

    client = new HTTPClient(HTTP_POST, AI_ENDPOINT);
    client->set_header("Content-Type", "application/json");
    client->set_header("Connection", "keep-alive");
}

#ifdef _cplusplus
}
#endif
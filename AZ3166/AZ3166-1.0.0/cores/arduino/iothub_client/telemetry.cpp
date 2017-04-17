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

#define STACK_SIZE 0x2000
#define IOTHUB_NAME_MAX_LEN 50
#define MAX_MESSAGE_SIZE 128

extern NetworkInterface *network;

const char *body_template = "{\"data\": {\"baseType\": \"EventData\",\"baseData\": {\"properties\": "
                            "{\"keyword\": \"%s\",\"hardware_version\": \"%s\",\"mcu\": \"%s\",\"message\":"
                            "\"%s\",\"mac_address\": \"%s\",\"iothub_name\":\"%s\"},"
                            "\"name\": \"%s\"}},\"time\": \"%s\",\"name\": \"%s\",\"iKey\": \"%s\"}";
static Thread ai_thread(osPriorityNormal, STACK_SIZE, NULL);
static char mac_addr[33];
const char *ai_endpoint = "https://dc.services.visualstudio.com/v2/track";
const char *ikey = "63d78aab-86a7-49b9-855f-3bdcff5d39d7";
const char *ai_event = "AIEVENT";
const char *keyword = "AZ3166";
const char *hardware_v = "1.0.0";
const char *mcu_type = "STM32F412";

struct Telemetry
{
    char iothub[IOTHUB_NAME_MAX_LEN + 2];
    char event[MAX_MESSAGE_SIZE];
    char message[MAX_MESSAGE_SIZE];
};

static Queue<Telemetry, 16> queue;

void telemetry_enqueue(const char *iothub, const char *event, const char *message)
{
    if (!iothub || !event || !message || strlen(iothub) > IOTHUB_NAME_MAX_LEN || strlen(event) > MAX_MESSAGE_SIZE || strlen(message) > MAX_MESSAGE_SIZE)
    {
        LogError("telemetry message exceeds the max length permitted. IotHub %s, event %s, message %s", iothub, event, message);
        return;
    }
    struct Telemetry *telemetry = (Telemetry *)malloc(sizeof(struct Telemetry));
    strncpy(telemetry->iothub, iothub, strlen(iothub));
    strncpy(telemetry->event, event, strlen(event));
    strncpy(telemetry->message, message, strlen(message));

    telemetry->iothub[strlen(iothub)] = 0;
    telemetry->event[strlen(event)] = 0;
    telemetry->message[strlen(message)] = 0;
    int ret = queue.put(telemetry);
    if (ret != 0)
    {
        LogError("error to enqueue telemetry event: %s, iothub : %s, message: %s, ret %d.", event, iothub, message, ret);
    }
}

void hash(char *result, const char *input)
{
    unsigned char output[16];
    unsigned char temp_buffer[20];

    memcpy(temp_buffer, input, strlen(input));
    mbedtls_md5(temp_buffer, sizeof(temp_buffer), output);

    int i = 0;
    for (i = 0; i < 16; i++)
    {
        sprintf(result + i * 2, "%02x", output[i]);
    }
    result[i * 2] = 0;
}

void do_trace_telemetry()
{
    osEvent evt = queue.get();
    if (evt.status != osEventMessage)
    {
        return;
    }

    char serialized_body[512];
    char *_ctime;
    time_t t;
    time(&t);
    struct Telemetry *telemetry = (Telemetry *)evt.value.p;
    unsigned char output[16];
    unsigned char temp_buffer[20];
    char iothub_name[33];
    hash(iothub_name, telemetry->iothub);
    _ctime = (char *)ctime(&t);
    _ctime[strlen(_ctime) - 1] = 0;
    sprintf(serialized_body, body_template, keyword, hardware_v, mcu_type, telemetry->message,
            mac_addr, iothub_name, telemetry->event, _ctime, ai_event, ikey);

    HTTPClient *client = new HTTPClient(HTTP_POST, ai_endpoint);
    client->set_header("Content-Type", "application/json");
    client->set_header("Connection", "keep-alive");

    Http_Response *response = client->send(serialized_body, strlen(serialized_body));
    if (response->status_code != 200)
    {
        LogError("Response error: <%d>, <%s>.", response->status_code, response->status_message);
    }

    free(telemetry);
    delete client;
    delete response;
}

static void trace_telemetry()
{
    hash(mac_addr, network->get_mac_address());
    while (true)
    {
        do_trace_telemetry();
    }
}

void telemetry_init()
{
    if (!network || ai_thread.start(trace_telemetry) != 0)
    {
        LogError("telemetry init failed.");
    }
}

#ifdef _cplusplus
}
#endif
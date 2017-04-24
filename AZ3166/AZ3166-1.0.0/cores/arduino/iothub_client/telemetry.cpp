// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "http_client.h"
#include "Queue.h"
#include "Thread.h"
#include "md5.h"

#ifdef _cplusplus
extern "C" {
#endif

#include "Arduino.h"
#include "telemetry.h"
#include "SystemWiFi.h"

#define STACK_SIZE 0x2000
#define IOTHUB_NAME_MAX_LEN 52
#define MAX_MESSAGE_SIZE 128

extern NetworkInterface *network;
static Thread ai_thread(osPriorityNormal, STACK_SIZE, NULL);
static char hash_mac_addr[33] = {NULL};
static char hash_iothub_name[33] = {NULL};

static const char *ai_endpoint = "https://dc.services.visualstudio.com/v2/track";
static const char *ikey = "63d78aab-86a7-49b9-855f-3bdcff5d39d7";
static const char *ai_event = "AIEVENT";
static const char *keyword = "AZ3166";
static const char *hardware_v = "0.8.0";
static const char *mcu_type = "STM32F412";

static const char *body_template = "{\"data\": {\"baseType\": \"EventData\",\"baseData\": {\"properties\": "
                                   "{\"keyword\": \"%s\",\"hardware_version\": \"%s\",\"mcu\": \"%s\",\"message\":"
                                   "\"%s\",\"mac_address\": \"%s\",\"iothub_name\":\"%s\"},"
                                   "\"name\": \"%s\"}},\"time\": \"%s\",\"name\": \"%s\",\"iKey\": \"%s\"}";
static const char hex_str[] = "0123456789abcdef";

struct Telemetry
{
    char iothub[IOTHUB_NAME_MAX_LEN];
    char event[MAX_MESSAGE_SIZE];
    char message[MAX_MESSAGE_SIZE];
};

static Queue<Telemetry, 16> queue;

void telemetry_enqueue(const char *iothub, const char *event, const char *message)
{
    struct Telemetry *telemetry = (Telemetry *)malloc(sizeof(struct Telemetry));
    strncpy(telemetry->iothub, iothub, IOTHUB_NAME_MAX_LEN);
    strncpy(telemetry->event, event, MAX_MESSAGE_SIZE);
    strncpy(telemetry->message, message, MAX_MESSAGE_SIZE);

    telemetry->iothub[IOTHUB_NAME_MAX_LEN - 1] = 0;
    telemetry->event[MAX_MESSAGE_SIZE - 1] = 0;
    telemetry->message[MAX_MESSAGE_SIZE - 1] = 0;
    queue.put(telemetry);
}

void hash(char *result, const char *input)
{
    unsigned char output[16];
    mbedtls_md5(reinterpret_cast<const unsigned char *>(input), strlen(input), output);

    int i = 0;
    for (i = 0; i < 16; i++)
    {
        result[i * 2] = hex_str[(output[i] >> 4) & 0x0F];
        result[i * 2 + 1] = hex_str[(output[i]) & 0x0F];
    }
    result[i * 2] = 0;
}

void do_trace_telemetry()
{
    osEvent evt = queue.get();
    if (evt.status != osEventMessage)
    {
        wait_ms(500);
        return;
    }
    SyncTime();

    char serialized_body[512];
    char *_ctime;
    time_t t;
    time(&t);
    struct Telemetry *telemetry = (Telemetry *)evt.value.p;

    if (!hash_iothub_name[0])
    {
        hash(hash_iothub_name, telemetry->iothub);
    }

    _ctime = ctime(&t);
    _ctime[strlen(_ctime) - 1] = 0;
    sprintf(serialized_body, body_template, keyword, hardware_v, mcu_type, telemetry->message,
            hash_mac_addr, hash_iothub_name, telemetry->event, _ctime, ai_event, ikey);

    HTTPClient *client = new HTTPClient(HTTP_POST, ai_endpoint);
    client->set_header("Content-Type", "application/json");
    client->set_header("Connection", "keep-alive");
    Http_Response *response = client->send(serialized_body, strlen(serialized_body));
    
    free(telemetry);
    free(response->status_message);
    free(response->body);
    delete client;
    delete response;
}

static void trace_telemetry()
{
    hash(hash_mac_addr, network->get_mac_address());
    while (true)
    {
        do_trace_telemetry();
    }
}

void telemetry_init()
{
    // Sync up the date
    SyncTime();
    // Start the telemetry thread
    ai_thread.start(trace_telemetry);
}

#ifdef _cplusplus
}
#endif
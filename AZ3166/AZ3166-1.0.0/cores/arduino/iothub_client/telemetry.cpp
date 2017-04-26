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
static Thread TELEMETRY_THREAD(osPriorityNormal, STACK_SIZE, NULL);
static char HASH_MAC[33] = {NULL};
static char HASH_IOTHUB[33] = {NULL};

static const char *PATH = "https://dc.services.visualstudio.com/v2/track";
static const char *IKEY = "63d78aab-86a7-49b9-855f-3bdcff5d39d7";
static const char *EVENT = "AIEVENT";
static const char *KEYWORD = "AZ3166";
static const char *VERSION = "0.8.0";
static const char *MCU = "STM32F412";
static const char *BODY_TEMPLATE = "{\"data\": {\"baseType\": \"EventData\",\"baseData\": {\"properties\": "
                                   "{\"keyword\": \"%s\",\"hardware_version\": \"%s\",\"mcu\": \"%s\",\"message\":"
                                   "\"%s\",\"hash_mac_address\": \"%s\",\"hash_iothub_name\":\"%s\"},"
                                   "\"name\": \"%s\"}},\"time\": \"%s\",\"name\": \"%s\",\"iKey\": \"%s\"}";
static const char HEX_STR[] = "0123456789abcdef";

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
        result[i * 2] = HEX_STR[(output[i] >> 4) & 0x0F];
        result[i * 2 + 1] = HEX_STR[(output[i]) & 0x0F];
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

    char body[512];
    char *_ctime;
    time_t t;
    time(&t);
    struct Telemetry *telemetry = (Telemetry *)evt.value.p;

    if (!HASH_IOTHUB[0] && telemetry->iothub[0] != '\0')
    {
        hash(HASH_IOTHUB, telemetry->iothub);
    }

    _ctime = ctime(&t);
    _ctime[strlen(_ctime) - 1] = 0;
    
    sprintf(body, BODY_TEMPLATE, KEYWORD, VERSION, MCU, telemetry->message, HASH_MAC, HASH_IOTHUB, telemetry->event, _ctime, EVENT, IKEY);
    HTTPClient *request = new HTTPClient(HTTP_POST, PATH);
    Http_Response *response = request->send(body, strlen(body));
    
    free(telemetry);
    free(response->status_message);
    free(response->body);
    delete request;
    delete response;
}

static void trace_telemetry()
{
    hash(HASH_MAC, network->get_mac_address());
    while (true)
    {
        do_trace_telemetry();
    }
}

void telemetry_init()
{
    // Sync up the date
    SyncTime();
    TELEMETRY_THREAD.start(trace_telemetry);
}

#ifdef _cplusplus
}
#endif
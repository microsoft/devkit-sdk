// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "Arduino.h"
#include "telemetry.h"
#include "SystemWiFi.h"
#include "SystemTime.h"
#include "SystemLock.h"
#include "Queue.h"
#include "Thread.h"
#include "md5.h"
#include "http_client.h"

#define STACK_SIZE 0x2000
#define IOTHUB_NAME_MAX_LEN 52
#define MAX_MESSAGE_SIZE 128

// Todo, the url and key of AI shall be get from REST service / web page, instead of hardcode here 
static const char *PATH = "https://dc.services.visualstudio.com/v2/track";
static const char *IKEY = "63d78aab-86a7-49b9-855f-3bdcff5d39d7";

static const char *EVENT = "AIEVENT";
static const char *KEYWORD = "AZ3166";
static const char *VERSION = "0.8.0";
static const char *MCU = "STM32F412";
static const char *BODY_TEMPLATE = 
"{"
    "\"data\": {"
        "\"baseType\": \"EventData\","
        "\"baseData\": {"
            "\"properties\": {"
                "\"keyword\": \"%s\","
                "\"hardware_version\": \"%s\","
                "\"mcu\": \"%s\","
                "\"message\":\"%s\","
                "\"hash_mac_address\": \"%s\","
                "\"hash_iothub_name\":\"%s\""
            "},"
            "\"name\": \"%s\""
        "}"
    "},"
    "\"time\": \"%s\","
    "\"name\": \"%s\","
    "\"iKey\": \"%s\""
"}";
static const char HEX_STR[] = "0123456789abcdef";

static Thread TELEMETRY_THREAD(osPriorityNormal, STACK_SIZE, NULL);
static char hash_mac[33] = { '\0' };
static char hash_iothub_name[33] = { '\0' };

typedef struct _tagTelemetry
{
    char iothub[IOTHUB_NAME_MAX_LEN];
    char event[MAX_MESSAGE_SIZE];
    char message[MAX_MESSAGE_SIZE];
}Telemetry;

static Queue<Telemetry, 16> queue;

static void hash(char *result, const char *input)
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

static void send_data_to_ai(const char* data, int size)
{
    SystemLock lock;
    HTTPClient *client = new HTTPClient(HTTP_POST, PATH);
    client->set_header("mem","good");
    const Http_Response *response = client->send(data, size);
    delete client;
}

static void do_trace_telemetry(const char *iothub, const char *event, const char *message)
{
    // Prepare the hash data
    if (hash_mac[0] == 0)
    {
        hash(hash_mac, WiFiInterface()->get_mac_address());
    }
    if (hash_iothub_name[0] == 0 && iothub[0] != '\0')
    {
        hash(hash_iothub_name, iothub);
    }

    // Time
    time_t t = time(NULL);
    char *_ctime = ctime(&t);
    // There is a new line character ('\n') at the end of the string which will disturb the json string, so remove it
    int tlen = strlen(_ctime) - 1;
    _ctime[tlen] = 0;

    // Calculate the size of the event (json) string
    int size = strlen(BODY_TEMPLATE) + strlen(KEYWORD) + strlen(VERSION) + strlen(MCU) + strlen(EVENT) + strlen(IKEY) - 20;
    size += strlen(message) + strlen(hash_mac) + strlen(hash_iothub_name) + strlen(event) + tlen + 1;
    
    // Send
    char* data = new char[size];
    sprintf(data, BODY_TEMPLATE, KEYWORD, VERSION, MCU, message, hash_mac, hash_iothub_name, event, _ctime, EVENT, IKEY);
    send_data_to_ai(data, size);
    delete [] data;
}

static void trace_telemetry()
{
    while (true)
    {
        osEvent evt = queue.get();
        if (evt.status != osEventMessage)
        {
            wait_ms(500);
            return;
        }
        SyncTime();

        Telemetry *telemetry = (Telemetry *)evt.value.p;

        do_trace_telemetry(telemetry->iothub, telemetry->event, telemetry->message);
        
        delete telemetry;
    }
}

void telemetry_init()
{
    // Sync up the date
    SyncTime();
    TELEMETRY_THREAD.start(trace_telemetry);
}

void send_telemetry_data_async(const char *iothub, const char *event, const char *message)
{
    Telemetry *telemetry = new Telemetry;
    memset(telemetry, 0, sizeof(Telemetry));
    strncpy(telemetry->iothub, iothub, IOTHUB_NAME_MAX_LEN - 1);
    strncpy(telemetry->event, event, MAX_MESSAGE_SIZE - 1);
    strncpy(telemetry->message, message, MAX_MESSAGE_SIZE - 1);

    queue.put(telemetry);
}

void send_telemetry_data(const char *iothub, const char *event, const char *message)
{
    do_trace_telemetry(iothub, event, message);
}
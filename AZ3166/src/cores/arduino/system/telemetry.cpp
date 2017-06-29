// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "Arduino.h"
#include "telemetry.h"
#include "SystemWiFi.h"
#include "SystemTime.h"
#include "Queue.h"
#include "Thread.h"
#include "md5.h"
#include "http_client.h"
#include "SystemVersion.h"

#define STACK_SIZE 0x1000
#define IOTHUB_NAME_MAX_LEN 52
#define MAX_EVENT_SIZE 64
#define MAX_MESSAGE_SIZE 128

// Todo, the url and key of AI shall be get from REST service / web page, instead of hardcode here 
static const char *PATH = "https://dc.services.visualstudio.com/v2/track";
static const char *IKEY = "63d78aab-86a7-49b9-855f-3bdcff5d39d7";

static const char *EVENT = "AIEVENT";
static const char *KEYWORD = "AZ3166";
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

static char hash_mac[33] = { '\0' };
static char hash_iothub_name[33] = { '\0' };
static int base_size;

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
    int size = base_size + strlen(message) + strlen(event) + tlen + 1;
    
    // Send
    char* data = new char[size];
    sprintf(data, BODY_TEMPLATE, KEYWORD, getDevkitVersion(), MCU, message, hash_mac, hash_iothub_name, event, _ctime, EVENT, IKEY);
    send_data_to_ai(data, size);
    delete [] data;
}

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

void telemetry_init()
{
    base_size = strlen(BODY_TEMPLATE) + strlen(KEYWORD) + strlen(getDevkitVersion()) + strlen(MCU) + strlen(EVENT) + strlen(IKEY) - 20 + sizeof(hash_mac) + sizeof(hash_iothub_name);
    // Sync up the date
    SyncTime();
}

void send_telemetry_data(const char *iothub, const char *event, const char *message)
{
    do_trace_telemetry(iothub ? iothub : "", event ? event : "", message ? message : "");
}

#ifdef __cplusplus
}
#endif  // __cplusplus
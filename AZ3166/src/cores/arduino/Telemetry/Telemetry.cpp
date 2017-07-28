// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "Arduino.h"
#include "http_client.h"
#include "md5.h"
#include "SystemTime.h"
#include "SystemVariables.h"
#include "SystemVersion.h"
#include "SystemWiFi.h"
#include "Telemetry.h"

#define STACK_SIZE 0x1000

static const char *EVENT = "AIEVENT";
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

Telemetry::Telemetry(const char *ai_endoint, const char *ai_ikey)
    : m_telemetry_thread(osPriorityIdle, STACK_SIZE, NULL)
{
    m_ai_endoint = ai_endoint;
    m_ai_ikey = ai_ikey;

    memset(m_hash_mac, 0, sizeof(m_hash_mac));
    memset(m_hash_iothub_name, 0, sizeof(m_hash_iothub_name));
    m_base_size = strlen(BODY_TEMPLATE) + sizeof(BOARD_NAME) + strlen(getDevkitVersion()) + sizeof(BOARD_MCU) + strlen(EVENT) + strlen(m_ai_ikey) - 20 + sizeof(m_hash_mac) + sizeof(m_hash_iothub_name);

    m_telemetry_thread.start(callback(this, &Telemetry::telemetry_worker));
}

void Telemetry::Send(const char *event, const char *message, const char *iothub, bool async)
{
    do_trace_telemetry(iothub ? iothub : "", event ? event : "", message ? message : "", async);
}

void Telemetry::hash(char *result, const char *input)
{
    static const char HEX_STR[] = "0123456789abcdef";

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

void Telemetry::send_data_to_ai(const char* data, int size)
{
    HTTPClient client(HTTP_POST, m_ai_endoint);
    client.set_header("mem","good");
    const Http_Response *response = client.send(data, size);
}

void Telemetry::do_trace_telemetry(const char *iothub, const char *event, const char *message, bool async)
{
    // Prepare the hash data
    if (m_hash_mac[0] == 0)
    {
        hash(m_hash_mac, WiFiInterface()->get_mac_address());
    }
    if (m_hash_iothub_name[0] == 0 && iothub[0] != '\0')
    {
        hash(m_hash_iothub_name, iothub);
    }

    // Time
    time_t t = time(NULL);
    char *_ctime = ctime(&t);
    // There is a new line character ('\n') at the end of the string which will disturb the json string, so remove it
    int tlen = strlen(_ctime) - 1;
    _ctime[tlen] = 0;

    // Calculate the size of the event (json) string
    int size = m_base_size + strlen(message) + strlen(event) + tlen + 1;
    
    // Send
    char* data = new char[size];
    sprintf(data, BODY_TEMPLATE, BOARD_NAME, getDevkitVersion(), BOARD_MCU, message, m_hash_mac, m_hash_iothub_name, event, _ctime, EVENT, m_ai_ikey);

    if (async)
    {
       if (!push_msg(data))
       {
            // Buffer is full, throw away this message
            delete [] data;
       }
    }
    else
    {
        send_data_to_ai(data, size);
        delete [] data;
    }
}

bool Telemetry::push_msg(char* message)
{
    if (!m_pool.full()) // Not perfect but the easiest way
    {
        m_pool.push(message);
        return true;
    }
    return false;
}

char* Telemetry::pop_msg(void)
{
    char* result;
    if (m_pool.pop(result))
    {
        return result;
    }
    return NULL;
}

void Telemetry::telemetry_worker(void)
{
    while (true)
    {
        char* msg = pop_msg();
        if (msg != NULL)
        {
            send_data_to_ai(msg, strlen(msg) + 1);
            delete [] msg;
        }
        else
        {
            wait_ms(5000);
        }
    }
}

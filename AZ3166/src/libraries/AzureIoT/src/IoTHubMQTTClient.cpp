// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
#include "EEPROMInterface.h"
#include "IoTHubMQTTClient.h"
#include "SerialLog.h"
#include "SystemTickCounter.h"
#include "SystemWiFi.h"
#include "Telemetry.h"

#define CHECK_INTERVAL_MS           5000
#define MQTT_KEEPALIVE_INTERVAL_S   120
#define MESSAGE_SEND_RETRY_COUNT    2
#define MESSAGE_SEND_TIMEOUT_MS     3000
#define MESSAGE_CONFIRMED           -2

static int callbackCounter;
static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = NULL;
static int receiveContext = 0;
static int statusContext = 0;
static int trackingId = 0;
static int currentTrackingId = -1;
static bool resetClient = false;
static CONNECTION_STATUS_CALLBACK _connection_status_callback = NULL;
static SEND_CONFIRMATION_CALLBACK _send_confirmation_callback = NULL;
static MESSAGE_CALLBACK _message_callback = NULL;

static uint64_t iothub_check_ms;

static char* iothub_hostname = NULL;

typedef struct EVENT_INSTANCE_TAG
{
    IOTHUB_MESSAGE_HANDLE messageHandle;
    int messageTrackingId; // For tracking the messages within the user callback.
} EVENT_INSTANCE;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities
static void CheckConnection()
{
    if (resetClient)
    {
        if (SystemWiFiRSSI() == 0)
        {
            LogInfo(">>>No Wi-Fi.");
        }
        else
        {
            LogInfo(">>>Re-connect.");
            // Re-connect the IoT Hub
            IoTHubMQTT_Close();
            IoTHubMQTT_Init();
            resetClient = false;
        }
    }
}

static void AZIoTLog(LOG_CATEGORY log_category, const char* file, const char* func, const int line, unsigned int options, const char* format, ...)
{
    va_list arg;
    char temp[64];
    char* buffer = temp;
    
    va_start(arg, format);
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    
    if (len > sizeof(temp) - 1)
    {
        buffer = (char*)malloc(len + 1);
        if (!buffer)
        {
            return;
        }

        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    
    time_t t = time(NULL);
    struct tm* tm_info = gmtime(&t);
    char ct[26];
    strftime(ct, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    switch (log_category)
    {
    case AZ_LOG_INFO:
        serial_xlog("%s INFO:  ", ct);
        break;
    case AZ_LOG_ERROR:
        serial_xlog("%s ERROR: File:%s Func:%s Line:%d, ", ct, file, func, line);
        break;
    default:
        break;
    }

    serial_log(buffer);
    
    if (options & LOG_LINE)
	{
		serial_log("\r\n");
	}

    if (buffer != temp)
    {
        free(buffer);
    }	
}

static char* GetHostNameFromConnectionString(char* connectionString)
{
    if (connectionString == NULL)
    {
        return NULL;
    }
    int start = 0;
    int cur = 0;
    bool find = false;
    while(connectionString[cur] > 0)
    {
        if (connectionString[cur] == '=')
        {
            // Check the key
            if (memcmp(&connectionString[start], "HostName", 8) == 0)
            {
                // This is the host name
                find = true;
            }
            start = ++cur;
            // Value
            while(connectionString[cur] > 0)
            {
                if (connectionString[cur] == ';')
                {
                    break;
                }
                cur++;
            }
            if (find && cur - start > 0)
            {
                char* hostname = (char*)malloc(cur - start + 1);
                memcpy(hostname, &connectionString[start], cur - start);
                hostname[cur - start] = 0;
                return hostname;
            }
            start = cur + 1;
        }
        cur++;
    }
    return NULL;
}

static EVENT_INSTANCE* GenerateMessage(const char *text)
{
    EVENT_INSTANCE *currentMessage = (EVENT_INSTANCE*)malloc(sizeof(EVENT_INSTANCE));
    currentMessage->messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)text, strlen(text));
    if (currentMessage->messageHandle == NULL)
    {
        LogError("iotHubMessageHandle is NULL!");
        free(currentMessage);
        return NULL;
    }
    currentMessage->messageTrackingId = trackingId++;
    currentTrackingId = currentMessage->messageTrackingId;

    MAP_HANDLE propMap = IoTHubMessage_Properties(currentMessage->messageHandle);
    
    char propText[32];
    sprintf_s(propText, sizeof(propText), "PropMsg_%d", currentMessage->messageTrackingId);
    if (Map_AddOrUpdate(propMap, "PropName", propText) != MAP_OK)
    {
         LogError("Map_AddOrUpdate Failed!");
         free(currentMessage);
         return NULL;
    }

    return currentMessage;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers
static void ConnectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* userContextCallback)
{
    LogInfo(">>>ConnectionStatusCallback %d, %d", result, reason);
    switch(reason)
    {
    case IOTHUB_CLIENT_CONNECTION_EXPIRED_SAS_TOKEN:
        if (result == IOTHUB_CLIENT_CONNECTION_UNAUTHENTICATED)
        {
            // turn off Azure led 
            DigitalOut LedAzure(LED_AZURE);
            LedAzure = 0;
            resetClient = true;
            LogInfo(">>>Connection status: timeout");
        }
        break;
    case IOTHUB_CLIENT_CONNECTION_DEVICE_DISABLED:
        break;
    case IOTHUB_CLIENT_CONNECTION_BAD_CREDENTIAL:
        break;
    case IOTHUB_CLIENT_CONNECTION_RETRY_EXPIRED:
        break;
    case IOTHUB_CLIENT_CONNECTION_NO_NETWORK:
        if (result == IOTHUB_CLIENT_CONNECTION_UNAUTHENTICATED)
        {
            // Turn off Azure led 
            DigitalOut LedAzure(LED_AZURE);
            LedAzure = 0;
            resetClient = true;
            LogInfo(">>>Connection status: disconnected");
        }
        break;
    case IOTHUB_CLIENT_CONNECTION_COMMUNICATION_ERROR:
        break;
    case IOTHUB_CLIENT_CONNECTION_OK:
        if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
        {
            // Turn on Azure led 
            DigitalOut LedAzure(LED_AZURE);
            LedAzure = 1;
            // Microsoft collects data to operate effectively and provide you the best experiences with our products. 
            // We collect data about the features you use, how often you use them, and how you use them.
            send_telemetry_data(iothub_hostname, "Create", "IoT hub established");
        }
        break;
    }
    
    if (_connection_status_callback)
    {
        _connection_status_callback(result, reason);
    }
}

static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
    EVENT_INSTANCE *eventInstance = (EVENT_INSTANCE *)userContextCallback;
    LogInfo("Confirmation[%d] received for message tracking id = %d with result = %s", callbackCounter++, eventInstance->messageTrackingId, ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
    
    if (result == IOTHUB_CLIENT_CONFIRMATION_OK)
    {
        if (currentTrackingId == eventInstance->messageTrackingId)
        {
            currentTrackingId = MESSAGE_CONFIRMED;
        }
    }
    
    // Free the message
    IoTHubMessage_Destroy(eventInstance->messageHandle);
    free(eventInstance);
    
    if (_send_confirmation_callback)
    {
        _send_confirmation_callback(result);
    }
}

static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void *userContextCallback)
{
    int *counter = (int *)userContextCallback;
    const char *buffer;
    size_t size;
    
    // Message content
    if (IoTHubMessage_GetByteArray(message, (const unsigned char **)&buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        LogError("unable to retrieve the message data");
    }
    else
    {
        LogInfo("Received Message [%d], Size=%d", *counter, (int)size);
        if (_message_callback)
        {
            _message_callback(buffer, size);
        }
    }

    /* Some device specific action code goes here... */
    (*counter)++;
    return IOTHUBMESSAGE_ACCEPTED;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MQTT APIs
void IoTHubMQTT_Init(void)
{
    if (iotHubClientHandle != NULL)
    {
        return;
    }

    callbackCounter = 0;
    
    xlogging_set_log_function(AZIoTLog);
    
    srand((unsigned int)time(NULL));
    trackingId = 0;

    // Load connection from EEPROM
    EEPROMInterface eeprom;
    uint8_t connString[AZ_IOT_HUB_MAX_LEN + 1] = { '\0' };
    int ret = eeprom.read(connString, AZ_IOT_HUB_MAX_LEN, 0x00, AZ_IOT_HUB_ZONE_IDX);
    if (ret < 0)
    { 
        LogError("Unable to get the azure iot connection string from EEPROM. Please set the value in configuration mode.");
        return;
    }
    else if (ret == 0)
    {
        LogInfo("The connection string is empty.\r\nPlease set the value in configuration mode.");
    }
    iothub_hostname = GetHostNameFromConnectionString((char*)connString);
    
    if (platform_init() != 0)
    {
        LogError("Failed to initialize the platform.");
        return;
    }
    
    // Create the IoTHub client
    if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString((char*)connString, MQTT_Protocol)) == NULL)
    {
        // Microsoft collects data to operate effectively and provide you the best experiences with our products. 
        // We collect data about the features you use, how often you use them, and how you use them.
        send_telemetry_data(iothub_hostname, "Create", "IoT hub establish failed");
        return;
    }
	
    int keepalive = MQTT_KEEPALIVE_INTERVAL_S;
    IoTHubClient_LL_SetOption(iotHubClientHandle, "keepalive", &keepalive);
    bool traceOn = false;
    IoTHubClient_LL_SetOption(iotHubClientHandle, "logtrace", &traceOn);
    if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
    {
        LogError("Failed to set option \"TrustedCerts\"");
        return;
    }

    /* Setting Message call back, so we can receive Commands. */
    if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK)
    {
        LogError("IoTHubClient_LL_SetMessageCallback..........FAILED!");
        return;
    }

    if (IoTHubClient_LL_SetConnectionStatusCallback(iotHubClientHandle, ConnectionStatusCallback, &statusContext) != IOTHUB_CLIENT_OK)
    {
        LogError("IoTHubClient_LL_SetConnectionStatusCallback..........FAILED!");
        return;
    }
    iothub_check_ms = SystemTickCounterRead();
}

bool IoTHubMQTT_SendEvent(const char *text)
{
    if (iotHubClientHandle == NULL || text == NULL)
    {
        return false;
    }

    for (int i = 0; i < MESSAGE_SEND_RETRY_COUNT; i++)
    {
        if (SystemWiFiRSSI() == 0)
        {
            return false;
        }

        uint64_t start_ms = SystemTickCounterRead();

        CheckConnection();

        EVENT_INSTANCE *currentMessage = GenerateMessage(text);
        if (currentMessage == NULL)
        {
            return false;
        }

        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, currentMessage->messageHandle, SendConfirmationCallback, currentMessage) != IOTHUB_CLIENT_OK)
        {
            LogError("IoTHubClient_LL_SendEventAsync..........FAILED!");
            IoTHubMessage_Destroy(currentMessage->messageHandle);
            free(currentMessage);
            return false;
        }
        LogInfo("IoTHubClient_LL_SendEventAsync accepted message for transmission to IoT Hub.");

        while(true)
        {
            IoTHubClient_LL_DoWork(iotHubClientHandle);
            
            if (currentTrackingId == MESSAGE_CONFIRMED)
            {
                // IoT Hub got this message
                return true;
            }
            
            // Check timeout
            int diff = (int)(SystemTickCounterRead() - start_ms);
            if (diff >= MESSAGE_SEND_TIMEOUT_MS)
            {
                // Time out, reset the client
                resetClient = true;
            }

            if (resetClient)
            {
                // Disconnected, re-send the message
                break;
            }
            else
            {
                // Sleep a while
                ThreadAPI_Sleep(100);
            }
        }
    }
    return false;
}

void IoTHubMQTT_Check(void)
{
    if (iotHubClientHandle == NULL || SystemWiFiRSSI() == 0)
    {
        return;
    }

    int diff = (int)(SystemTickCounterRead() - iothub_check_ms);
    if (diff >= CHECK_INTERVAL_MS)
    {
        CheckConnection();
        IoTHubClient_LL_DoWork(iotHubClientHandle);
        iothub_check_ms = SystemTickCounterRead();
    }
}

void IoTHubMQTT_Close(void)
{
    IoTHubClient_LL_Destroy(iotHubClientHandle);
    iotHubClientHandle = NULL;
}

void IoTHubMQTT_SetConnectionStatusCallback(CONNECTION_STATUS_CALLBACK connection_status_callback)
{
    _connection_status_callback = connection_status_callback;
}

void IoTHubMQTT_SetSendConfirmationCallback(SEND_CONFIRMATION_CALLBACK send_confirmation_callback)
{
    _send_confirmation_callback = send_confirmation_callback;
}

void IoTHubMQTT_SetMessageCallback(MESSAGE_CALLBACK message_callback)
{
    _message_callback = message_callback;
}

void LogTrace(const char *event, const char *message)
{
    // Microsoft collects data to operate effectively and provide you the best experiences with our products. 
    // We collect data about the features you use, how often you use them, and how you use them.
    send_telemetry_data_async(iothub_hostname, event, message);
}
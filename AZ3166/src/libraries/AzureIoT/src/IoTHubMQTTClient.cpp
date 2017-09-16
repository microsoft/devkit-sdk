// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
#include "EEPROMInterface.h"
#include "IoTHubMQTTClient.h"
#include "SerialLog.h"
#include "SystemTickCounter.h"
#include "SystemWiFi.h"
#include "Telemetry.h"

#define CONNECT_TIMEOUT_MS          30000
#define CHECK_INTERVAL_MS           5000
#define MQTT_KEEPALIVE_INTERVAL_S   120
#define SEND_EVENT_RETRY_COUNT      2
#define EVENT_TIMEOUT_MS            10000
#define EVENT_CONFIRMED             -2
#define EVENT_FAILED                -3

static int callbackCounter;
static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = NULL;
static int receiveContext = 0;
static int statusContext = 0;
static int trackingId = 0;
static int currentTrackingId = -1;
static bool clientConnected = false;
static bool resetClient = false;
static CONNECTION_STATUS_CALLBACK _connection_status_callback = NULL;
static SEND_CONFIRMATION_CALLBACK _send_confirmation_callback = NULL;
static MESSAGE_CALLBACK _message_callback = NULL;
static DEVICE_TWIN_CALLBACK _device_twin_callback = NULL;
static DEVICE_METHOD_CALLBACK _device_method_callback = NULL;
static REPORT_CONFIRMATION_CALLBACK _report_confirmation_callback = NULL;
static bool enableDeviceTwin = false;

static uint64_t iothub_check_ms;

static char* iothub_hostname = NULL;

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
            IoTHubMQTT_Init(enableDeviceTwin);
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers
static void ConnectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* userContextCallback)
{
    clientConnected = false;
    
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
            clientConnected = true;
            LogInfo(">>>Connection status: connected");
            
            LogTrace("Create", "IoT hub established");
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
    EVENT_INSTANCE *event = (EVENT_INSTANCE *)userContextCallback;
    LogInfo(">>>Confirmation[%d] received for message tracking id = %d with result = %s", callbackCounter++, event->trackingId, ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
    
    if (currentTrackingId == event->trackingId)
    {
        if (result == IOTHUB_CLIENT_CONFIRMATION_OK)
        {
            currentTrackingId = EVENT_CONFIRMED;
        }
        else
        {
            currentTrackingId = EVENT_FAILED;
        }
    }
    
    // Free the message
    IoTHubMessage_Destroy(event->messageHandle);
    free(event);
    
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
        return IOTHUBMESSAGE_REJECTED;
    }
    else
    {
        char *temp = (char *)malloc(size + 1);
        if (temp == NULL)
        {
            LogError("Failed to malloc for command");
            return IOTHUBMESSAGE_REJECTED;
        }
        memcpy(temp, buffer, size);
        temp[size] = '\0';
        LogInfo(">>>Received Message [%d], Size=%d Message %s", *counter, (int)size, temp);
        if (_message_callback)
        {
            _message_callback(temp, size);
        }
        free(temp);
    }

    /* Some device specific action code goes here... */
    (*counter)++;
    return IOTHUBMESSAGE_ACCEPTED;
}

static void DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payLoad, size_t size, void *userContextCallback)
{
    if (_device_twin_callback)
    {
        _device_twin_callback(updateState, payLoad, size);
    }
}

static int DeviceMethodCallback(const char *methodName, const unsigned char *payload, size_t size, unsigned char **response, size_t *response_size, void *userContextCallback)
{
    if (_device_method_callback)
    {
        return _device_method_callback(methodName, payload, size, response, (int *)response_size);
    }
    
    const char *responseMessage = "\"No method found\"";
    *response_size = strlen(responseMessage);
    *response = (unsigned char *)strdup("\"No method found\"");
    
    return 404;
}

static void ReportConfirmationCallback(int statusCode, void *userContextCallback)
{
    EVENT_INSTANCE *event = (EVENT_INSTANCE *)userContextCallback;
    LogInfo(">>>Confirmation[%d] received for state tracking id = %d with state code = %d", callbackCounter++, event->trackingId, statusCode);
    
    if (statusCode == 204)
    {
        if (currentTrackingId == event->trackingId)
        {
            currentTrackingId = EVENT_CONFIRMED;
        }
    }
    else
    {
        LogError("Report confirmation failed with state code %d", statusCode);
    }
    
    // Free the state
    free(event);
    
    if (_report_confirmation_callback)
    {
        _report_confirmation_callback(statusCode);
    }
}

static bool SendEventOnce(EVENT_INSTANCE *event)
{
    if (iotHubClientHandle == NULL || event == NULL)
    {
        return false;
    }

    if (SystemWiFiRSSI() == 0)
    {
        return false;
    }

    event->trackingId = trackingId++;
    currentTrackingId = event->trackingId;

    uint64_t start_ms = SystemTickCounterRead();

    CheckConnection();
    wait_ms(1000);

    if (event->type == MESSAGE)
    {
        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, event->messageHandle, SendConfirmationCallback, event) != IOTHUB_CLIENT_OK)
        {
            LogError("IoTHubClient_LL_SendEventAsync..........FAILED!");
            IoTHubMessage_Destroy(event->messageHandle);
            free(event);
            wait_ms(1000);
            return false;
        }
        LogInfo(">>>IoTHubClient_LL_SendEventAsync accepted message for transmission to IoT Hub.");
        wait_ms(1000);
    }
    else if (event->type == STATE)
    {
        if (IoTHubClient_LL_SendReportedState(iotHubClientHandle, (const unsigned char*)event->stateString, strlen(event->stateString), ReportConfirmationCallback, event) != IOTHUB_CLIENT_OK)
        {
            LogError("IoTHubClient_LL_SendReportedState..........FAILED!");
            IoTHubMessage_Destroy(event->messageHandle);
            free(event);
            return false;
        }
        LogInfo(">>>IoTHubClient_LL_SendReportedState accepted state for transmission to IoT Hub.");
    }

    while(true)
    {
        IoTHubClient_LL_DoWork(iotHubClientHandle);
        
        if (currentTrackingId == EVENT_CONFIRMED)
        {
            // IoT Hub got this event
            return true;
        }
        
        // Check timeout
        int diff = (int)(SystemTickCounterRead() - start_ms);
        if (diff >= EVENT_TIMEOUT_MS)
        {
            // Time out, reset the client
            LogError("Waiting for send confirmation, time is up %d", diff);
            resetClient = true;
        }

        if (resetClient)
        {
            // resetClient also can be set as true in the IoTHubClient_LL_DoWork
            // Disconnected, re-send the message
            break;
        }
        else
        {
            // Sleep a while
            ThreadAPI_Sleep(100);
        }
    }
    
    return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MQTT APIs
EVENT_INSTANCE* GenerateEvent(const char *eventString, EVENT_TYPE type)
{
    if (eventString == NULL)
    {
        return NULL;
    }

    EVENT_INSTANCE *event = (EVENT_INSTANCE*)malloc(sizeof(EVENT_INSTANCE));
    event->type = type;

    if (type == MESSAGE)
    {
        event->messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)eventString, strlen(eventString));
        if (event->messageHandle == NULL)
        {
            LogError("iotHubMessageHandle is NULL!");
            free(event);
            return NULL;
        }
    
        MAP_HANDLE propMap = IoTHubMessage_Properties(event->messageHandle);
        
        char propText[32];
        sprintf_s(propText, sizeof(propText), "PropMsg_%d", event->trackingId);
        if (Map_AddOrUpdate(propMap, "PropName", propText) != MAP_OK)
        {
             LogError("Map_AddOrUpdate Failed!");
             free(event);
             return NULL;
        }
    }

    if (type == STATE)
    {
        event->stateString = eventString;
    }

    return event;
}

void AddProp(EVENT_INSTANCE *message, const char *key, const char *value)
{
    if (message == NULL || key == NULL) return;
    MAP_HANDLE propMap = IoTHubMessage_Properties(message->messageHandle);
    Map_AddOrUpdate(propMap, key, value);
}

bool IoTHubMQTT_Init(bool hasDeviceTwin)
{
    if (iotHubClientHandle != NULL)
    {
        return true;
    }
    enableDeviceTwin = hasDeviceTwin;
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
        return false;
    }
    else if (ret == 0)
    {
        LogError("The connection string is empty.\r\nPlease set the value in configuration mode.");
        return false;
    }
    iothub_hostname = GetHostNameFromConnectionString((char*)connString);
    
    if (platform_init() != 0)
    {
        LogError("Failed to initialize the platform.");
        return false;
    }
    
    // Create the IoTHub client
    if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString((char*)connString, MQTT_Protocol)) == NULL)
    {
        LogTrace("Create", "IoT hub establish failed");
        return false;
    }
    
    int keepalive = MQTT_KEEPALIVE_INTERVAL_S;
    IoTHubClient_LL_SetOption(iotHubClientHandle, "keepalive", &keepalive);
    bool traceOn = false;
    IoTHubClient_LL_SetOption(iotHubClientHandle, "logtrace", &traceOn);
    if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
    {
        LogError("Failed to set option \"TrustedCerts\"");
        return false;
    }

    /* Setting Message call back, so we can receive Commands. */
    if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK)
    {
        LogError("IoTHubClient_LL_SetMessageCallback..........FAILED!");
        return false;
    }

    if (IoTHubClient_LL_SetConnectionStatusCallback(iotHubClientHandle, ConnectionStatusCallback, &statusContext) != IOTHUB_CLIENT_OK)
    {
        LogError("IoTHubClient_LL_SetConnectionStatusCallback..........FAILED!");
        return false;
    }

    if (enableDeviceTwin)
    {
        if (IoTHubClient_LL_SetDeviceTwinCallback(iotHubClientHandle, DeviceTwinCallback, NULL) != IOTHUB_CLIENT_OK)
        {
            LogError("Failed on IoTHubClient_LL_SetDeviceTwinCallback");
            return false;
        }

        if(IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, DeviceMethodCallback, NULL) != IOTHUB_CLIENT_OK)
        {
            LogError("Failed on IoTHubClient_LL_SetDeviceMethodCallback");
            return false;
        }
    }

    iothub_check_ms = SystemTickCounterRead();

    // Waiting for the confirmation 
    uint64_t start_ms = SystemTickCounterRead();
    while (true)
    {
        IoTHubClient_LL_DoWork(iotHubClientHandle);
        if (clientConnected)
        {
            break;
        }
        int diff = (int)(SystemTickCounterRead() - start_ms);
        if (diff >= CONNECT_TIMEOUT_MS)
        {
            // Time out, reset the client
            resetClient = true;
            return false;
        }
        ThreadAPI_Sleep(500);
    }
    
    return true;
}

bool IoTHubMQTT_SendEvent(const char *text)
{
    if (text == NULL)
    {
        return false;
    }
    for (int i = 0; i < SEND_EVENT_RETRY_COUNT; i++)
    {
        if (SendEventOnce(GenerateEvent(text, MESSAGE)))
        {
            return true;
        }
    }
    return false;
}

bool IoTHubMQTT_ReportState(const char *stateString)
{
    if (stateString == NULL)
    {
        return false;
    }
    for (int i = 0; i < SEND_EVENT_RETRY_COUNT; i++)
    {
        if (SendEventOnce(GenerateEvent(stateString, STATE)))
        {
            return true;
        }
    }
    return false;
}

bool IoTHubMQTT_SendEventInstance(EVENT_INSTANCE *event)
{
    if (event == NULL)
    {
        return false;
    }

    return SendEventOnce(event);
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
        for (int i = 0; i < 5; i++)
        {
            IoTHubClient_LL_DoWork(iotHubClientHandle);
            if (resetClient || SystemWiFiRSSI() == 0)
            {
                // Disconnected
                break;
            }
        }
        iothub_check_ms = SystemTickCounterRead();
    }
}

void IoTHubMQTT_Close(void)
{
    if (iotHubClientHandle != NULL)
    {
        IoTHubClient_LL_Destroy(iotHubClientHandle);
        iotHubClientHandle = NULL;
    }
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

void IoTHubMQTT_SetDeviceTwinCallback(DEVICE_TWIN_CALLBACK device_twin_callback)
{
    _device_twin_callback = device_twin_callback;
}

void IoTHubMQTT_SetDeviceMethodCallback(DEVICE_METHOD_CALLBACK device_method_callback)
{
    _device_method_callback = device_method_callback;
}

void IoTHubMQTT_SetReportConfirmationCallback(REPORT_CONFIRMATION_CALLBACK report_confirmation_callback)
{
    _report_confirmation_callback = report_confirmation_callback;
}

void LogTrace(const char *event, const char *message)
{
    // Microsoft collects data to operate effectively and provide you the best experiences with our products. 
    // We collect data about the features you use, how often you use them, and how you use them.
    send_telemetry_data_async(iothub_hostname, event, message);
}
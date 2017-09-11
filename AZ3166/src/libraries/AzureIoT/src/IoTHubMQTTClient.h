// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __IOTHUB_MQTT_CLIENT_H__
#define __IOTHUB_MQTT_CLIENT_H__

#include "AzureIotHub.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum EVENT_TYPE {
    MESSAGE, STATE
};

typedef struct EVENT_INSTANCE_TAG
{
    EVENT_TYPE type;
    IOTHUB_MESSAGE_HANDLE messageHandle;
    const char* stateString;
    int trackingId; // For tracking the events within the user callback.
} EVENT_INSTANCE;

/**
* @brief	Generate an event with the event string specified by @p eventString.
*
* @param	eventString		   	The string of event.
*/
EVENT_INSTANCE* GenerateEvent(const char *eventString, EVENT_TYPE type);

/**
* @brief	Add new property value for message.
*
* @param	message		   	The message need to be modified.
* @param	key		   	    The property name.
* @param	value		   	The property value.
*/
void AddProp(EVENT_INSTANCE *message, const char * key, const char * value);

/**
* @brief	Initialize a IoT Hub MQTT client for communication with an existing IoT hub.
*           The connection string is load from the EEPROM.
*/
bool IoTHubMQTT_Init(void);

/**
* @brief	Asynchronous call to send the message specified by @p text.
*
* @param	text		   	The text message.
*/
bool IoTHubMQTT_SendEvent(const char *text);

/**
* @brief	Synchronous call to report the state specified by @p stateString.
*
* @param	stateString		The JSON string of reported state.
*/
bool IoTHubMQTT_ReportState(const char *stateString);

/**
* @brief	Synchronous call to report the event specified by @p event.
*
* @param	event           The event instance.
*/
bool IoTHubMQTT_SendEventInstance(EVENT_INSTANCE *event);

/**
* @brief	The function is called to try receiving message from IoT hub.
*/
void IoTHubMQTT_Check(void);

/**
* @brief	Disposes of resources allocated by the IoT Hub client.
*/
void IoTHubMQTT_Close(void);

/**
* @brief	Sets up connection status callback to be invoked representing the status of the connection to IOT Hub.
*/
void IoTHubMQTT_SetConnectionStatusCallback(CONNECTION_STATUS_CALLBACK connection_status_callback);

/**
* @brief	Sets up send confirmation status callback to be invoked representing the status of sending message to IOT Hub.
*/
void IoTHubMQTT_SetSendConfirmationCallback(SEND_CONFIRMATION_CALLBACK send_confirmation_callback);

/**
* @brief	Sets up the message callback to be invoked when IoT Hub issues a message to the device.
*/
void IoTHubMQTT_SetMessageCallback(MESSAGE_CALLBACK message_callback);

/**
* @brief	Sets up the device twin callback to be invoked when IoT Hub update device twin of the device.
*/
void IoTHubMQTT_SetDeviceTwinCallback(DEVICE_TWIN_CALLBACK device_twin_callback);

/**
* @brief	Sets up the device method callback to be invoked when IoT Hub call method on the device.
*/
void IoTHubMQTT_SetDeviceMethodCallback(DEVICE_METHOD_CALLBACK device_method_callback);

/**
* @brief	Log the trace to Microsoft Application Insights
*/
void LogTrace(const char *event, const char *message);

#ifdef __cplusplus
}
#endif

#endif /* __IOTHUB_MQTT_CLIENT_H__ */

// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __IOTHUB_MQTT_CLIENT_H__
#define __IOTHUB_MQTT_CLIENT_H__

#include "AzureIotHub.h"

#ifdef __cplusplus
extern "C"
{
#endif

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
* @brief	Log the trace to Microsoft Application Insights
*/
void LogTrace(const char *event, const char *message);

#ifdef __cplusplus
}
#endif

#endif /* __IOTHUB_MQTT_CLIENT_H__ */

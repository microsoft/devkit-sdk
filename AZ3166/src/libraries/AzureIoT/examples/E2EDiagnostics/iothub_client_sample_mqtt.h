// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef IOTHUB_CLIENT_SAMPLE_MQTT_H
#define IOTHUB_CLIENT_SAMPLE_MQTT_H

#include "iothub_client_ll.h"

void iothubInit(void);
void sensorInit(void);
void showC2DMessageReceived(void);
void parseTwinMessage(const char *);
void showSendConfirmation(void);
void readMessage(int, char *);
void iothubSendMessage(const unsigned char *text);
void iothubLoop(void);
void iothubClose(void);

#endif /* IOTHUB_CLIENT_SAMPLE_MQTT_H */
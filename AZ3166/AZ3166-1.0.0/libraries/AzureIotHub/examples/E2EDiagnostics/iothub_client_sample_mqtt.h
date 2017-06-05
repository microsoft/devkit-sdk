// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef IOTHUB_CLIENT_SAMPLE_MQTT_H
#define IOTHUB_CLIENT_SAMPLE_MQTT_H

#include "iothub_client_ll.h"

void iothubInit(void);
void sensorInit(void);
void blinkLED(void);
void parseTwinMessage(const char *);
void blinkSendConfirmation(void);
void readMessage(int, char *);
void iothubSendMessage(const unsigned char *text);
void iothubLoop(void);
void iothubClose(void);

#endif /* IOTHUB_CLIENT_SAMPLE_MQTT_H */
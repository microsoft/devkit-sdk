// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef IOTHUB_CLIENT_SAMPLE_MQTT_H
#define IOTHUB_CLIENT_SAMPLE_MQTT_H

void iothubInit(void);
void iothubSendMessage(const unsigned char *);
void iothubLoop(void);
void iothubClose(void);

#endif /* IOTHUB_CLIENT_SAMPLE_MQTT_H */
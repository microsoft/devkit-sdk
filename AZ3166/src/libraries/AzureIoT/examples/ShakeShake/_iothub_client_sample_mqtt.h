// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef IOTHUB_CLIENT_SAMPLE_MQTT_H
#define IOTHUB_CLIENT_SAMPLE_MQTT_H

void iothub_client_sample_mqtt_init(void);
void iothub_client_sample_send_event(const unsigned char *text);
void iothub_client_sample_mqtt_loop(void);
void iothub_client_sample_mqtt_close(void);

void TwitterMessageCallback(const char* tweet, int lenTweet);
void MessageSendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result);
#endif /* IOTHUB_CLIENT_SAMPLE_MQTT_H */
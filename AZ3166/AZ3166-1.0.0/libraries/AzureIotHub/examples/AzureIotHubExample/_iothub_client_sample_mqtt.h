// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef IOTHUB_CLIENT_SAMPLE_MQTT_H
#define IOTHUB_CLIENT_SAMPLE_MQTT_H

void iothub_client_sample_mqtt_init(void);
void iothub_client_sample_send_event(const unsigned char *text);
void iothub_client_sample_mqtt_loop(void);
void showMessage(const char* tweet);
#endif /* IOTHUB_CLIENT_SAMPLE_MQTT_H */
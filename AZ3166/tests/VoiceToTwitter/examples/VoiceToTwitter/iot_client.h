// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _IOT_HUB_CLIENT_H
#define _IOT_HUB_CLIENT_H

#define MAX_UPLOAD_SIZE (64 * 1024)
int iot_client_set_connection_string(const char*conn_str);
int iot_client_blob_upload_step1(const char *blobName);
int iot_client_blob_upload_step2(const char *content, int length);
int iot_client_blob_upload_step3(bool isSuccess);
const char* iot_client_get_c2d_message(char ** etag);
int complete_c2d_message(char *etag);
int iot_client_send_event(const char*text, int length);


#define D2C_ENDPOINT "https://%s/devices/%s/messages/events?api-version=2016-11-14"
#define C2D_ENDPOINT "https://%s/devices/%s/messages/deviceBound?api-version=2016-11-14"
#define C2D_CB_ENDPOINT "https://%s/devices/%s/messages/deviceBound/%s?api-version=2016-11-14"
#define BLOB_REQUEST_ENDPOINT "https://%s/devices/%s/files/%s?api-version=2016-11-14"
#define BLOB_NOTIFICATION_ENDPOINT "https://%s/devices/%s/files/notifications/%s?api-version=2016-11-14"

#define SEMICOLON ";"
#define EQUAL_CHARACTOR '='
static const char HOSTNAME_TOKEN[] = "HostName";
static const char DEVICEID_TOKEN[] = "DeviceId";
static const char DEVICEKEY_TOKEN[] = "SharedAccessKey";
#endif
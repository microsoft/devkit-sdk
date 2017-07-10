// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "iot_client.h"
#include "Arduino.h"
#include <json.h>
#include <stdlib.h>
#include "azure_c_shared_utility/sastoken.h"
#include "http_client.h"

static char *hostNameString = NULL;
static char *deviceIdString = NULL;
static char *deviceKeyString = NULL;
static size_t current_expiry = 0;
static char *current_token = NULL;
static char *sasUri = NULL;
static char *correlationId = NULL;
static int step2_status_code = 0;
static char temp[1024];
static char temp2[1024];

/// Utility methods begin
const char *_json_object_get_string(json_object *obj, const char *name);
void _setString(char **p, const char *value, int length);
int _check_iot_ready_for_request();
/// Utility methods end

// Public methods
int iot_client_set_connection_string(const char *conn_str)
{
    int len = strlen(conn_str);
    strcpy(temp, conn_str);

    char *pch;
    pch = strtok(temp, SEMICOLON);

    while (pch != NULL)
    {

        String keyValuePair(pch);
        int equalPos = keyValuePair.indexOf(EQUAL_CHARACTOR);
        if (equalPos > 0 && equalPos < keyValuePair.length() - 1)
        {
            String key = keyValuePair.substring(0, equalPos);
            String value = keyValuePair.substring(equalPos + 1);
            key.trim();
            value.trim();
            if (strcmp(key.c_str(), HOSTNAME_TOKEN) == 0)
            {
                _setString(&hostNameString, value.c_str(), value.length());
            }
            else if (strcmp(key.c_str(), DEVICEID_TOKEN) == 0)
            {
                _setString(&deviceIdString, value.c_str(), value.length());
            }
            else if (strcmp(key.c_str(), DEVICEKEY_TOKEN) == 0)
            {
                _setString(&deviceKeyString, value.c_str(), value.length());
            }
            else
            {
                Serial.print("Invalid connection string property :");
                Serial.println(key);
                return -1;
            }
        }
        else
        {
            Serial.print("Invalid connection string");
            Serial.println(conn_str);
            return -1;
        }

        pch = strtok(NULL, SEMICOLON);
    }
    return 0;
}

int iot_client_blob_upload_step1(const char *blobName)
{
    if (blobName == NULL)
    {
        Serial.println("Invalid blob name");
        return -1;
    }

    if (_check_iot_ready_for_request() != 0)
    {
        return -1;
    }
    sprintf(temp, BLOB_REQUEST_ENDPOINT, hostNameString, deviceIdString, blobName);
    HTTPClient blobRequest = HTTPClient(HTTP_GET, temp);
    blobRequest.set_header("Authorization", current_token);
    blobRequest.set_header("Accept", "application/json");
    const Http_Response *response = blobRequest.send();
    bool error = false;
    if (response == NULL)
    {
        Serial.println("iot_client_blob_upload_step1 failed!");
        return -1;
    }
    if (response->status_code < 300)
    {
        json_object *jsonObject = json_tokener_parse(response->body);
        if (jsonObject != NULL)
        {

            const char *json_correlationId = _json_object_get_string(jsonObject, "correlationId");
            if (json_correlationId == NULL)
            {
                Serial.println("Unable to get string from json object for \"correlationId\"");
                error = true;
            }
            const char *json_hostName = _json_object_get_string(jsonObject, "hostName");
            if (json_hostName == NULL)
            {
                Serial.println("Unable to get string from json object for \"hostName\"");
                error = true;
            }
            const char *json_containerName = _json_object_get_string(jsonObject, "containerName");
            if (json_containerName == NULL)
            {
                Serial.println("Unable to get string from json object for \"containerName\"");
                error = true;
            }

            const char *json_blobName = _json_object_get_string(jsonObject, "blobName");
            if (json_blobName == NULL)
            {
                Serial.println("Unable to get string from json object for \"blobName\"");
                error = true;
            }
            const char *json_sasToken = _json_object_get_string(jsonObject, "sasToken");
            if (json_sasToken == NULL)
            {
                Serial.println("Unable to get string from json object for \"sasToken\"");
                error = true;
            }
            if (!error)
            {
                sprintf(temp2, "https://%s/%s/%s%s", json_hostName, json_containerName, json_blobName, json_sasToken);
                _setString(&sasUri, temp2, strlen(temp2));
                _setString(&correlationId, json_correlationId, strlen(json_correlationId));
            }
        }

        if (jsonObject != NULL)
            json_object_put(jsonObject);
    }
    
    return error ? -1 : 0;
}

int iot_client_blob_upload_step2(const char *content, int length)
{
    if (content == NULL)
    {
        Serial.println("No content to upload");
        return -1;
    }
    if (length <= 0 || length > MAX_UPLOAD_SIZE)
    {
        Serial.println(length <= 0 ? "Too few bytes to upload" : "Too many bytes to upload");
        return -1;
    }
    if (hostNameString == NULL || deviceIdString == NULL || deviceKeyString == NULL)
    {
        Serial.println("Iot hub connection string is not initialized");
        return -1;
    }
    if (sasUri == NULL)
    {
        Serial.println("Please run iot_client_blob_upload_step2 after iot_client_blob_upload_step1.");
        return -1;
    }
    HTTPClient uploadRequest = HTTPClient(HTTP_PUT, sasUri);
    uploadRequest.set_header("x-ms-blob-type", "BlockBlob");
    const Http_Response *response = uploadRequest.send(content, length);
    if (response == NULL)
    {
        Serial.println("iot_client_blob_upload_step2 failed!");
        return -1;
    }
    printf("Upload blob result: <%d> message <%s>\r\n", response->status_code, response->status_message);
    step2_status_code = response->status_code;
    return !(step2_status_code >= 200 && step2_status_code < 300);
}

int iot_client_blob_upload_step3(bool isSuccess)
{
    if (_check_iot_ready_for_request() != 0)
    {
        return -1;
    }
    sprintf(temp, BLOB_NOTIFICATION_ENDPOINT, hostNameString, deviceIdString, correlationId);
    HTTPClient notificationRequest = HTTPClient(HTTP_POST, temp);
    notificationRequest.set_header("Content-Type", "application/json");
    notificationRequest.set_header("Accept", "application/json");
    notificationRequest.set_header("User-Agent", "iothubclient 1.1.4");
    notificationRequest.set_header("Authorization", current_token);

    int postBodyLength = sprintf(temp2, "{\"isSuccess\":%s, \"statusCode\":%d, \"statusDescription\":\"%s\"}",
                                 (isSuccess ? "true" : "false"), step2_status_code, (isSuccess ? "ok" : "failure"));
    const Http_Response *response = notificationRequest.send(temp2, postBodyLength);
    if (response == NULL)
    {
        Serial.println("iot_client_blob_upload_step3 (send notification) faield!");
        return -1;
    }
    printf("Send notification result: <%d> message <%s>\r\n", response->status_code, response->status_message);
    return 0;
}

int iot_client_send_event(const char *event, int length)
{
    if (_check_iot_ready_for_request() != 0)
    {
        return -1;
    }
    sprintf(temp, D2C_ENDPOINT, hostNameString, deviceIdString);
    HTTPClient request = HTTPClient(HTTP_POST, temp);
    request.set_header("Authorization", current_token);
    request.set_header("Accept", "application/json");
    const Http_Response *response = request.send(event, length);
    if (response == NULL)
    {
        Serial.println("iot_client_send_event faield.");
        return -1;
    }
    printf("Send event result: <%d> message <%s>\r\n", response->status_code, response->status_message);
    return 0;
}

int complete_c2d_message(char *etag)
{
    if (etag == NULL || strlen(etag) < 2)
    {
        Serial.println("Invalid etag.");
        return -1;
    }
    if (_check_iot_ready_for_request() != 0)
    {
        return -1;
    }
    etag[strlen(etag) - 1] = '\0';
    sprintf(temp, C2D_CB_ENDPOINT, hostNameString, deviceIdString, etag + 1);
    HTTPClient request = HTTPClient(HTTP_DELETE, temp);
    request.set_header("Authorization", current_token);
    request.set_header("Accept", "application/json");

    const Http_Response *response = request.send();
    if (response == NULL)
    {
        Serial.println("Cannot delete message(Null Response).");
        return -1;
    }

    int status = response->status_code;
    return !(status >= 200 && status < 300);
}

const char *iot_client_get_c2d_message(char ** etag)
{
    if (_check_iot_ready_for_request() != 0)
    {
        return NULL;
    }
    sprintf(temp, C2D_ENDPOINT, hostNameString, deviceIdString);
    HTTPClient request = HTTPClient(HTTP_GET, temp);
    request.set_header("Authorization", current_token);
    request.set_header("Accept", "application/json");
    const Http_Response *response = request.send();
    const char *res = NULL;
    if (response == NULL)
    {
        Serial.println("Cannot get message(Null Response).");
        return NULL;
    }

    KEYVALUE *header = (KEYVALUE *)response -> headers;
    while (header -> prev != NULL) {
        if (strcmp("ETag", header -> prev -> key) == 0) {
            _setString(etag, header -> value, strlen(header -> value));
            Serial.print("ETag: ");
            Serial.println(*etag);
        }

        header = header -> prev;
    }

    if (response->body != NULL)
    {
        res = response->body;
    }
    return res != NULL ? strdup(res) : NULL;
}

const char *_json_object_get_string(json_object *obj, const char *name)
{
    return json_object_get_string(json_object_object_get(obj, name));
}
void _setString(char **p, const char *value, int length)
{
    if (*p != NULL)
    {
        free(*p);
    }
    *p = (char *)malloc(length + 1);
    strcpy(*p, value);
}

int _check_and_refresh_sastoken()
{
    time_t currentTime = time(NULL);
    if (currentTime == (time_t)-1 || currentTime < 1492333149)
    {
        Serial.println("Time does not appear to be working.");
        return -1;
    }
    size_t expiry = (size_t)(difftime(currentTime, 0) + 3600);
    if (current_expiry > (size_t)(difftime(currentTime, 0)))
    {
        return 0;
    }
    current_expiry = expiry;
    // Serial.println("generate new sas token.");
    STRING_HANDLE keyString = STRING_construct(deviceKeyString);
    sprintf(temp2, "%s/devices/%s", hostNameString, deviceIdString);
    STRING_HANDLE uriResource = STRING_construct(temp2);
    STRING_HANDLE empty = STRING_new();
    STRING_HANDLE newSASToken = SASToken_Create(
        keyString, uriResource, empty, expiry);
    _setString(&current_token, STRING_c_str(newSASToken), STRING_length(newSASToken));
    STRING_delete(newSASToken);
    STRING_delete(keyString);
    STRING_delete(uriResource);
    STRING_delete(empty);
    return 0;
}

int _check_iot_ready_for_request()
{
    if (hostNameString == NULL || deviceIdString == NULL || deviceKeyString == NULL)
    {
        Serial.println("Iot hub connection string is not initialized");
        return -1;
    }

    if (_check_and_refresh_sastoken() != 0)
    {
        Serial.println("Cannot generate sas token.");
        return -1;
    }
    return 0;
}

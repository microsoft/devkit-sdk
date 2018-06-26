// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#include "OTAClass.h"
static int status;
static CRC16_Context contex;
static volatile uint32_t flashIdx = 0;
static int body_length = 0;

static void getFwCalback(const char *at, size_t length){
    int err = MicoFlashWrite((mico_partition_t)MICO_PARTITION_OTA_TEMP, &flashIdx, (uint8_t *)at, length);
    if (err) status = -1;
    CRC16_Update(&contex, at, length);
    body_length += length;
}

OTAClass::OTAClass(){
}

int OTAClass::OTAFromUrl(char *url, const char* ssl_ca_pem){
    uint16_t crc = 0;
    status = 0;
    flashIdx = 0;
    body_length = 0;
    CRC16_Init(&contex);

    HTTPClient client = ssl_ca_pem ? HTTPClient(ssl_ca_pem, HTTP_GET, url, getFwCalback) : HTTPClient(HTTP_GET, url, getFwCalback);
    client.send(NULL, 0);

    CRC16_Final(&contex, &crc);
    int err = mico_ota_switch_to_new_fw(body_length, crc);
    if (err) status = -1;
    return status;
}

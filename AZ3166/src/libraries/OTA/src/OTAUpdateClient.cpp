// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
#include "OTAUpdateClient.h"

static int status;
static CRC16_Context contex;
static volatile uint32_t flashIdx = 0;
static int body_length = 0;

static void getFwCalback(const char *at, size_t length)
{
    if (at && length)
    {
        if (MicoFlashWrite((mico_partition_t)MICO_PARTITION_OTA_TEMP, &flashIdx, (uint8_t *)at, length) == kNoErr)
        {
            CRC16_Update(&contex, at, length);
            body_length += length;
        }
    }    
}

OTAUpdateClient::OTAUpdateClient()
{
}

int OTAUpdateClient::updateFromUrl(const char *url, const char* ssl_ca_pem)
{
    uint16_t crc = 0;
    flashIdx = 0;
    body_length = 0;
    CRC16_Init(&contex);

    HTTPClient client = ssl_ca_pem ? HTTPClient(ssl_ca_pem, HTTP_GET, url, getFwCalback) : HTTPClient(HTTP_GET, url, getFwCalback);
    const Http_Response *response = client.send(NULL, 0);
    if (response->status_code != 200 || body_length == 0)
    {
        // Download failed
        return 0;
    }
    CRC16_Final(&contex, &crc);
    
    // Set the firmware update flag to underlying system
    if (mico_ota_switch_to_new_fw(body_length, crc) == kNoErr)
    {
        return body_length;
    }
    return 0;
}

int OTAUpdateClient::calculateFirmwareCRC16(int fwSize)
{
    static volatile uint32_t flashIdx = 0;
    uint8_t *checkBuffer = (uint8_t *)malloc(1);
    int i = fwSize;
    uint16_t checkSum = 0;
    CRC16_Context contex;
    CRC16_Init(&contex);
    while (i--)
    {
        MicoFlashRead((mico_partition_t)MICO_PARTITION_OTA_TEMP, &flashIdx, checkBuffer, 1);
        CRC16_Update(&contex, checkBuffer, 1);
    }
    free(checkBuffer);
    CRC16_Final(&contex, &checkSum);
    return checkSum;
}
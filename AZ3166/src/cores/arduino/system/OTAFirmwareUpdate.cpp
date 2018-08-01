// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
#include "http_client.h"
#include "CheckSumUtils.h"
#include "mico.h"
#include "OTAFirmwareUpdate.h"

static CRC16_Context contexCRC16;
static volatile uint32_t flashIdx = 0;
static int body_length = 0;

static void getFwCallback(const char *at, size_t length)
{
    if (at == NULL || length == 0 || body_length < 0)
    {
        return;
    }
    // Write to external flash
    if (MicoFlashWrite((mico_partition_t)MICO_PARTITION_OTA_TEMP, &flashIdx, (uint8_t *)at, length) == kNoErr)
    {
        CRC16_Update(&contexCRC16, at, length);
        body_length += length;
    }
    else
    {
        body_length = -1;
    }
}

int OTADownloadFirmware(const char *url, uint16_t * crc16Checksum, const char* ssl_ca_pem)
{
    uint16_t checkSum = 0;
    flashIdx = 0;
    body_length = 0;
    CRC16_Init(&contexCRC16);

    HTTPClient client = ssl_ca_pem ? HTTPClient(ssl_ca_pem, HTTP_GET, url, getFwCallback) : HTTPClient(HTTP_GET, url, getFwCallback);
    const Http_Response *response = client.send(NULL, 0);
    if (response->status_code != 200)
    {
        // Download failed
        return -1;
    }
    else if (body_length < 0)
    {
        // External flash accessing issue
        return -2;
    }
    else if (body_length == 0)
    {
        // Empty
        return 0;
    }

    // Finalize the CRC16 value
    CRC16_Final(&contexCRC16, &checkSum);

    if (crc16Checksum)
    {
        *crc16Checksum = checkSum;
    }

    return body_length;
}

 int OTAApplyNewFirmware(int fwSize, uint16_t crc16Checksum)
 {
    // Set the firmware update flag to underlying system, after reboot the device will update to the new version
    if (mico_ota_switch_to_new_fw(fwSize, crc16Checksum) == kNoErr)
    {
        return 0;
    }
    // External flash accessing issue
    return -1;
 }
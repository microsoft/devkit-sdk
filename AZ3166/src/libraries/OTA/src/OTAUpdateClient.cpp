// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#include "OTAUpdateClient.h"

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

OTAUpdateClient::OTAUpdateClient(){
}

int OTAUpdateClient::updateFromUrl(const char *url, const char* ssl_ca_pem){
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

bool OTAUpdateClient::firmwarePackageCheckCRC16(const char* fwPackageCheckValue, int fwSize) {
    static volatile uint32_t flashIdx = 0;
    uint8_t *checkBuffer = (uint8_t *)malloc(1);
    int i = fwSize;
    uint16_t checkSum = 0;
    CRC16_Context contex;
    CRC16_Init(&contex);
    while (i--) {
        MicoFlashRead((mico_partition_t)MICO_PARTITION_OTA_TEMP, &flashIdx, checkBuffer, 1);
        CRC16_Update(&contex, checkBuffer, 1);
    }
    free(checkBuffer);
    CRC16_Final(&contex, &checkSum);
    char *checkSumString = CRC16ToString(checkSum);
    LogInfo("CRC16 result: %d, %s", checkSum, checkSumString);
    bool result = (strcmp(checkSumString, fwPackageCheckValue) == 0);
    free(checkSumString);
    return result;
}

char* OTAUpdateClient::CRC16ToString(uint16_t checksum) {
  char *result = (char *)malloc(5);
  memset(result, 0, 5);
  int idx = 4;
  while (idx--) {
    result[idx] = checksum % 16;
    result[idx] = result[idx] > 9 ? result[idx] - 10 + 'A' : result[idx] + '0';
    checksum /= 16;
  }
  return result;
}
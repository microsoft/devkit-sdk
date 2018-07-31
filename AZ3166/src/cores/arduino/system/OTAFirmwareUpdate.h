// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#ifndef __OTA_FIRMWARE_UPDATE_H__
#define __OTA_FIRMWARE_UPDATE_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**
* @brief    Download new firmware from given url.
*
* @param    [in] url                 The url to download firmware from.
*           [out] crc16Checksum      Return the CRC-16 (xmodem) checksum of the downloaded firmware
*           [in] ssl_ca_pem          Certificate of given url.
*
* @return   Return the size of the new firmware on success, otherwise return -1 if encounter network issue, return -2 if encounter external flash accessing issue.
*/
int OTADownloadFirmware(const char *url, uint16_t *crc16Checksum, const char* ssl_ca_pem = NULL);

/*
* @brief    Apply the new firmware, after reboot the Device will update to the new version
*
* @param    [in] fwSize              Size of the firmware.
*           [in] crc16Checksum       CRC-16 checksum value of the firmware
*
* @return   Return 0 on success, otherwise return -1.
*/
int OTAApplyNewFirmware(int fwSize, uint16_t crc16Checksum);

#ifdef __cplusplus
}
#endif

#endif  // __OTA_FIRMWARE_UPDATE_H__

#ifndef __OTA_UTILS_H__
#define __OTA_UTILS_H__
#include "SystemWiFi.h"
#include "iothub_client_ll.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define OTA_STATUS_CURRENT "Current"
#define OTA_STATUS_DOWNLOADING "Downloading"
#define OTA_STATUS_VERIFYING "Verifying"
#define OTA_STATUS_APPLYING "Applying"
#define OTA_STATUS_ERROR "Error"

#define OTA_CURRENT_FW_VERSION "currentFwVersion"
#define OTA_PENDING_FW_VERSION "pendingFwVersion"
#define OTA_FW_UPDATE_STATUS "fwUpdateStatus"
#define OTA_FW_UPDATE_SUBSTATUS "fwUpdateSubstatus"
#define OTA_LAST_FW_UPDATE_STARTTIME "lastFwUpdateStartTime"
#define OTA_LAST_FW_UPDATE_ENDTIME "lastFwUpdateEndTime"

typedef struct {
    char* fwVersion;
    char* fwPackageURI;
    char* fwPackageCheckValue;
    int fwSize;
} FW_INFO;

/**
* @brief    Retrieve the latest firmware information from Azure.
*
* @return   FW_INFO upon success or NULL upon failure.
*/
const FW_INFO* IoTHubClient_GetLatestFwInfo(void);

/**
* @brief    Report the OTA firmware update status to Azure.
*
* @param    OTAStatusMap             status collection.
*
* @return   true upon success or false upon failure.
*/
bool IoTHubClient_ReportOTAStatus(MAP_HANDLE OTAStatusMap);

/**
* @brief    Compare two firmware version string.
*
* @param    fwVersion1               version 1.
* @param    fwVersion2               version 2.
*
* @return    A integer, result = 1 : fwVersion1 > fwVersion2
*                              = 0 : fwVersion1 = fwVersion2
*                              = -1 : fwVersion1 < fwVersion2
*/
int IoTHubClient_FwVersionCompare(const char* fwVersion1, const char* fwVersion2);

#ifdef __cplusplus
}
#endif

#endif
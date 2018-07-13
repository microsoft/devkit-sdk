#ifndef __OTA_UTILS_H__
#define __OTA_UTILS_H__
#include "SystemWiFi.h"
#include "iothub_client_hsm_ll.h"

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

const FW_INFO* IoTHubClient_GetLatestFwInfo(void);

bool IoTHubClient_ReportOTAStatus(MAP_HANDLE OTAStatusMap);

int IoTHubClient_FwVersionCompare(const char* fwVersion1, const char* fwVersion2);

#ifdef __cplusplus
}
#endif

#endif
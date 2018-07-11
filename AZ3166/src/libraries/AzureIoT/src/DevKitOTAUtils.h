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
    char* fwVersion = NULL;
    char* fwPackageURI = NULL;
    char* fwPackageCheckValue = NULL;
    int fwSize;
} FW_INFO;

void fw_info_free_string(FW_INFO &fwInfo);

int IoTHubClient_OTAVersionCompare(const char* fwVersion1, const char* fwVersion2);

bool IoTHubClient_OTAHasNewFw(FW_INFO* fwInfo);

bool IoTHubClient_ReportOTAStatus(const char* key, const char* value);

bool IoTHubClient_ReportOTAStatuses(MAP_HANDLE kvMap);

void ota_callback(const unsigned char *payLoad, size_t size);

#ifdef __cplusplus
}
#endif

#endif
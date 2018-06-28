#ifndef __OTA_UTILS_H__
#define __OTA_UTILS_H__
#include "SystemWiFi.h"
#include "iothub_client_hsm_ll.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
    char* fwVersion = NULL;
    char* fwPackageURI = NULL;
    char* fwPackageCheckValue = NULL;
    int fwSize;
} FW_INFO;

void fw_info_free_string(FW_INFO &fwInfo);

int IoTHubClient_OTAVersionCompare(const char* fwVersion1, const char* fwVersion2);

bool IoTHubClient_OTAHasNewFw(FW_INFO* fwInfo);

IOTHUB_CLIENT_RESULT IoTHubClient_SetCurrentFwInfo(const char* currentFwVersion);

bool IoTHubClient_ReportOTAStatus(const char* currentFwVersion, const char* fwUpdateStatus, const char* pendingFwVersion = NULL, const char* fwUpdateSubstatus = NULL);

void ota_callback(const unsigned char *payLoad, size_t size);

#ifdef __cplusplus
}
#endif

#endif
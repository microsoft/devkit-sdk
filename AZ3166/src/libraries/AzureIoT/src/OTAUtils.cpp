#include "OTAUtils.h"
#include "parson.h"
#include "DevKitMQTTClient.h"

extern char *deviceTwinPayLoad;
extern IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

static char *currentFirmwareVersion;

/**
* @brief	Compare two firmware version string.
*
* @return	A integer, result = 1 : fwVersion1 > fwVersion2
* 			                  = 0 : fwVersion1 = fwVersion2
* 			                  = -1 : fwVersion1 < fwVersion2
*/
int cmpVersionStr(const char* fwVersion1, const char* fwVersion2) {
    int vnum1 = 0, vnum2 = 0;
    size_t ver1Len = strlen(fwVersion1), ver2Len = strlen(fwVersion2);
    for (int i = 0, j = 0; (i < ver1Len || j < ver2Len); ++i, ++j) {
        while (i < ver1Len && fwVersion1[i] != '.') {
            vnum1 = vnum1 * 10 + (fwVersion1[i] - '0');
            i++;
        }
        while (j < ver2Len && fwVersion2[j] != '.') {
            vnum2 = vnum2 * 10 + (fwVersion2[j] - '0');
            j++;
        }
        if (vnum1 > vnum2)
            return 1;
        if (vnum2 > vnum1)
            return -1;
        vnum1 = vnum2 = 0;
    }
    return 0;
}

bool IoTHubClient_OTAHasNewFw(FW_INFO* fwInfo) {
    if (deviceTwinPayLoad == NULL) return false;
    if (iotHubClientHandle != NULL) {
        JSON_Value *root_value;
        root_value = json_parse_string(deviceTwinPayLoad);
        if (json_value_get_type(root_value) != JSONObject)
        {
            if (root_value != NULL)
            {
                json_value_free(root_value);
            }
            LogError("parse %s failed", deviceTwinPayLoad);
            return false;
        }
        JSON_Object *root_object = json_value_get_object(root_value);
        JSON_Object *desired_object = json_object_get_object(root_object, "desired");
        desired_object = desired_object == NULL ? root_object : desired_object;
        if (desired_object != NULL)
        {
            JSON_Object* firmwareVal = json_object_get_object(desired_object, "firmware");
            if (firmwareVal != NULL) {
                if (fwInfo == NULL) fwInfo = new FW_INFO;
                fwInfo -> fwVersion = json_object_get_string(firmwareVal, "fwVersion");
                fwInfo -> fwPackageURI = json_object_get_string(firmwareVal, "fwPackageURI");
                fwInfo -> fwPackageCheckValue = json_object_get_string(firmwareVal, "fwPackageCheckValue");
                fwInfo -> fwSize = json_object_get_number(firmwareVal, "fwSize");
                if (fwInfo -> fwVersion != NULL && fwInfo -> fwPackageURI != NULL && fwInfo -> fwPackageCheckValue != NULL) {
                    LogInfo(fwInfo -> fwVersion);
                    LogInfo(currentFirmwareVersion);
                    return cmpVersionStr(fwInfo -> fwVersion, currentFirmwareVersion) == 1;
                }
                LogInfo("NULL!");
            }
        }
        return false;
    }
    
    return false;
}

IOTHUB_CLIENT_RESULT IoTHubClient_SetCurrentFwInfo(const char* currentFwVersion) {
    IOTHUB_CLIENT_RESULT result;
    if (currentFwVersion == NULL) {
        result = IOTHUB_CLIENT_INVALID_ARG;
        return result;
    }
    free(currentFirmwareVersion);
    size_t size = strlen(currentFwVersion);
    currentFirmwareVersion = (char *)malloc(size + 1);
    if (currentFirmwareVersion == NULL) {
        result = IOTHUB_CLIENT_ERROR;
        return result;
    }
    memcpy(currentFirmwareVersion, currentFwVersion, size);
    currentFirmwareVersion[size] = '\0';
    result = IOTHUB_CLIENT_OK;
    return result;
}

bool IoTHubClient_ReportOTAStatus(const char* currentFwVersion, const char* fwUpdateStatus, const char* pendingFwVersion, const char* fwUpdateSubstatus) {
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    json_object_set_string(root_object, "currentFwVersion", currentFwVersion);
    json_object_set_string(root_object, "fwUpdateStatus", fwUpdateStatus);
    if (pendingFwVersion != NULL) {
        json_object_set_string(root_object, "pendingFwVersion", pendingFwVersion);
    }
    if (fwUpdateSubstatus != NULL) {
        json_object_set_string(root_object, "pendingFwVersion", fwUpdateSubstatus);
    }
    serialized_string = json_serialize_to_string_pretty(root_value);
    json_value_free(root_value);
    return DevKitMQTTClient_ReportState(serialized_string);
}
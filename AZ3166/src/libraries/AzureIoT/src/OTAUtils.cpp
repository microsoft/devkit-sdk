#include "OTAUtils.h"
#include "parson.h"
#include "DevKitMQTTClient.h"
#include "ctype.h"
static char *currentFirmwareVersion = NULL;
static FW_INFO* otaFwInfo = NULL;
/**
* @brief	Compare two firmware version string.
*
* @return	A integer, result = 1 : fwVersion1 > fwVersion2
* 			                  = 0 : fwVersion1 = fwVersion2
* 			                  = -1 : fwVersion1 < fwVersion2
*/
int IoTHubClient_OTAVersionCompare(const char* fwVersion1, const char* fwVersion2) {
    int vnum1 = 0, vnum2 = 0;
    size_t ver1Len = strlen(fwVersion1), ver2Len = strlen(fwVersion2);
    for (int i = 0, j = 0; (i < ver1Len || j < ver2Len); ++i, ++j) {
        while (i < ver1Len && fwVersion1[i] != '.') {
            if (isdigit(fwVersion1[i]))
                vnum1 = vnum1 * 10 + (fwVersion1[i] - '0');
            i++;
        }
        while (j < ver2Len && fwVersion2[j] != '.') {
            if (isdigit(fwVersion2[j]))
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
    if (otaFwInfo == NULL || fwInfo == NULL) return false;
    fwInfo -> fwVersion = strdup(otaFwInfo -> fwVersion);
    fwInfo -> fwPackageURI = strdup(otaFwInfo -> fwPackageURI);
    fwInfo -> fwPackageCheckValue = strdup(otaFwInfo -> fwPackageCheckValue);
    fwInfo -> fwSize = otaFwInfo -> fwSize;
    return true;
}

IOTHUB_CLIENT_RESULT IoTHubClient_SetCurrentFwInfo(const char* currentFwVersion) {
    if (currentFirmwareVersion != NULL)
        free(currentFirmwareVersion);
    currentFirmwareVersion = strdup(currentFwVersion);
    return IOTHUB_CLIENT_OK;
}

bool IoTHubClient_ReportOTAStatus(const char* currentFwVersion, const char* fwUpdateStatus, const char* pendingFwVersion, const char* fwUpdateSubstatus) {
    if (currentFwVersion == NULL || fwUpdateStatus == NULL) return false;
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

void ota_callback(const unsigned char *payLoad, size_t size) {
    if (otaFwInfo != NULL)
        delete otaFwInfo;
    otaFwInfo = new FW_INFO;
    char *deviceTwinPayLoad = (char *)malloc(size + 1);
    if (deviceTwinPayLoad != NULL)
    {
        memcpy(deviceTwinPayLoad, payLoad, size);
        deviceTwinPayLoad[size] = '\0';
    } else {
        return;
    }
    JSON_Value *root_value;
    root_value = json_parse_string(deviceTwinPayLoad);
    if (json_value_get_type(root_value) != JSONObject)
    {
        if (root_value != NULL)
        {
            json_value_free(root_value);
        }
        LogError("parse %s failed", deviceTwinPayLoad);
        return;
    }
    free(deviceTwinPayLoad);
    JSON_Object *root_object = json_value_get_object(root_value);
    JSON_Object *desired_object = json_object_get_object(root_object, "desired");
    desired_object = desired_object == NULL ? root_object : desired_object;
    if (desired_object != NULL)
    {
        JSON_Object* firmwareVal = json_object_get_object(desired_object, "firmware");
        if (firmwareVal != NULL) {
            LogInfo("Get firmware value from device twin.");
            otaFwInfo -> fwVersion = strdup(json_object_get_string(firmwareVal, "fwVersion"));
            otaFwInfo -> fwPackageURI = strdup(json_object_get_string(firmwareVal, "fwPackageURI"));
            otaFwInfo -> fwPackageCheckValue = strdup(json_object_get_string(firmwareVal, "fwPackageCheckValue"));
            otaFwInfo -> fwSize = json_object_get_number(firmwareVal, "fwSize");
            if (otaFwInfo -> fwVersion != NULL && otaFwInfo -> fwPackageURI != NULL && otaFwInfo -> fwPackageCheckValue != NULL) {
                json_value_free(root_value);
                return;
            }
        }
    }
    json_value_free(root_value);
    delete otaFwInfo;
    otaFwInfo = NULL;
}
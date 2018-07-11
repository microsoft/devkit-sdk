#include "DevKitOTAUtils.h"
#include "parson.h"
#include "DevKitMQTTClient.h"
#include "ctype.h"
#include "CheckSumUtils.h"
#include "mico.h"

static FW_INFO otaFwInfo;

void fw_info_free_string(FW_INFO &fwInfo)
{
    if (fwInfo.fwVersion != NULL)
    {
        free(fwInfo.fwVersion);
        fwInfo.fwVersion = NULL;
    }
    if (fwInfo.fwPackageURI != NULL)
    {
        free(fwInfo.fwPackageURI);
        fwInfo.fwPackageURI = NULL;
    }
    if (fwInfo.fwPackageCheckValue != NULL)
    {
        free(fwInfo.fwPackageCheckValue);
        fwInfo.fwPackageCheckValue = NULL;
    }
}

/**
* @brief	Compare two firmware version string.
*
* @return	A integer, result = 1 : fwVersion1 > fwVersion2
* 			                  = 0 : fwVersion1 = fwVersion2
* 			                  = -1 : fwVersion1 < fwVersion2
*/
int IoTHubClient_OTAVersionCompare(const char* fwVersion1, const char* fwVersion2)
{
    if (fwVersion1 == NULL || fwVersion2 == NULL)
    {
        if (fwVersion1 == fwVersion2) return 0;
        return (fwVersion1 ? 1 : -1);
    }
    int vnum1 = 0, vnum2 = 0;
    size_t ver1Len = strlen(fwVersion1), ver2Len = strlen(fwVersion2);
    for (int i = 0, j = 0; (i < ver1Len || j < ver2Len); ++i, ++j)
    {
        while (i < ver1Len && fwVersion1[i] != '.')
        {
            if (isdigit(fwVersion1[i]))
                vnum1 = vnum1 * 10 + (fwVersion1[i] - '0');
            i++;
        }
        while (j < ver2Len && fwVersion2[j] != '.')
        {
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

bool IoTHubClient_OTAHasNewFw(FW_INFO* fwInfo)
{
    if (otaFwInfo.fwVersion == NULL || otaFwInfo.fwPackageURI == NULL || fwInfo == NULL) return false;
    fw_info_free_string(*fwInfo);
    fwInfo -> fwVersion = strdup(otaFwInfo.fwVersion);
    fwInfo -> fwPackageURI = strdup(otaFwInfo.fwPackageURI);
    fwInfo -> fwPackageCheckValue = strdup(otaFwInfo.fwPackageCheckValue);
    fwInfo -> fwSize = otaFwInfo.fwSize;
    return true;
}

bool IoTHubClient_ReportOTAStatus(const char* key, const char* value)
{
    if (key == NULL || value == NULL) return false;
    JSON_Value *firmware_value = json_value_init_object();
    JSON_Object *firmware_object = json_value_get_object(firmware_value);
    char *serialized_string = NULL;
    json_object_set_string(firmware_object, key, value);
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_value(root_object, "firmware", firmware_value);
    serialized_string = json_serialize_to_string_pretty(root_value);
    json_value_free(root_value);
    return DevKitMQTTClient_ReportState(serialized_string);
}


bool IoTHubClient_ReportOTAStatuses(MAP_HANDLE kvMap)
{
    JSON_Value *firmware_value = json_parse_string(STRING_c_str(Map_ToJSON(kvMap)));
    if (firmware_value == NULL) return false;
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_value(root_object, "firmware", firmware_value);
    char *serialized_string = json_serialize_to_string_pretty(root_value);
    json_value_free(root_value);
    return DevKitMQTTClient_ReportState(serialized_string);
}

void ota_callback(const unsigned char *payLoad, size_t size)
{
    fw_info_free_string(otaFwInfo);
    char *deviceTwinPayLoad = (char *)malloc(size + 1);
    if (deviceTwinPayLoad != NULL)
    {
        memcpy(deviceTwinPayLoad, payLoad, size);
        deviceTwinPayLoad[size] = '\0';
    }
    else
    {
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
        free(deviceTwinPayLoad);
        return;
    }
    free(deviceTwinPayLoad);
    JSON_Object *root_object = json_value_get_object(root_value);
    JSON_Object *desired_object = json_object_get_object(root_object, "desired");
    desired_object = desired_object == NULL ? root_object : desired_object;
    if (desired_object != NULL)
    {
        JSON_Object* firmwareVal = json_object_get_object(desired_object, "firmware");
        if (firmwareVal != NULL)
        {
            LogInfo("Get firmware value from device twin.");
            otaFwInfo.fwVersion = strdup(json_object_get_string(firmwareVal, "fwVersion"));
            otaFwInfo.fwPackageURI = strdup(json_object_get_string(firmwareVal, "fwPackageURI"));
            otaFwInfo.fwPackageCheckValue = strdup(json_object_get_string(firmwareVal, "fwPackageCheckValue"));
            otaFwInfo.fwSize = json_object_get_number(firmwareVal, "fwSize");
            if (otaFwInfo.fwVersion == NULL || otaFwInfo.fwPackageURI == NULL)
            {
                fw_info_free_string(otaFwInfo);
            }
        }
    }
    json_value_free(root_value);
}
#include "DevKitOTAUtils.h"
#include "parson.h"
#include "DevKitMQTTClient.h"
#include "ctype.h"
#include "CheckSumUtils.h"
#include "mico.h"

static FW_INFO *latestFwInfo = NULL;

static void fw_info_free(FW_INFO *fwInfo)
{
    if (fwInfo)
    {
        if (fwInfo->fwVersion != NULL)
        {
            free(fwInfo->fwVersion);
        }
        if (fwInfo->fwPackageURI != NULL)
        {
            free(fwInfo->fwPackageURI);
        }
        if (fwInfo->fwPackageCheckValue != NULL)
        {
            free(fwInfo->fwPackageCheckValue);
        }
        free(fwInfo);
    }
}

int IoTHubClient_FwVersionCompare(const char* fwVersion1, const char* fwVersion2)
{
    if (fwVersion1 == NULL || fwVersion2 == NULL)
    {
        if (fwVersion1 == fwVersion2) return 0;
        return (fwVersion1 ? 1 : -1);
    }
    int vnum1 = 0, vnum2 = 0;
    size_t ver1Len = strlen(fwVersion1), ver2Len = strlen(fwVersion2);
    for (int i = 0, j = 0; (i < (int)ver1Len || j < (int)ver2Len); ++i, ++j)
    {
        while (i < (int)ver1Len && fwVersion1[i] != '.')
        {
            if (isdigit(fwVersion1[i]))
                vnum1 = vnum1 * 10 + (fwVersion1[i] - '0');
            i++;
        }
        while (j < (int)ver2Len && fwVersion2[j] != '.')
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

const FW_INFO* IoTHubClient_GetLatestFwInfo(void)
{
    return latestFwInfo;
}

bool IoTHubClient_ReportOTAStatus(MAP_HANDLE OTAStatusMap)
{
    const char *status_string = STRING_c_str(Map_ToJSON(OTAStatusMap));
    char* serialized_string = (char*)malloc(strlen(status_string) + 20);
    sprintf(serialized_string, "{\"firmware\":%s}", status_string);
    bool result = DevKitMQTTClient_ReportState(serialized_string);
    free(serialized_string);
    return result;
}

void ota_callback(const unsigned char *payLoad, size_t size)
{
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
        LogError("Parse %s failed", deviceTwinPayLoad);
        free(deviceTwinPayLoad);
        return;
    }
    
    JSON_Object *root_object = json_value_get_object(root_value);
    JSON_Object *desired_object = json_object_get_object(root_object, "desired");
    desired_object = desired_object == NULL ? root_object : desired_object;
    if (desired_object != NULL)
    {
        JSON_Object* firmwareVal = json_object_get_object(desired_object, "firmware");
        if (firmwareVal != NULL)
        {
            if (latestFwInfo)
            {
                fw_info_free(latestFwInfo);
                latestFwInfo = NULL;
            }
            latestFwInfo = (FW_INFO*)malloc(sizeof(FW_INFO));
            if (latestFwInfo)
            {
                latestFwInfo->fwVersion = strdup(json_object_get_string(firmwareVal, "fwVersion"));
                latestFwInfo->fwPackageURI = strdup(json_object_get_string(firmwareVal, "fwPackageURI"));
                latestFwInfo->fwPackageCheckValue = strdup(json_object_get_string(firmwareVal, "fwPackageCheckValue"));
                latestFwInfo->fwSize = json_object_get_number(firmwareVal, "fwSize");
                if (latestFwInfo->fwVersion == NULL || latestFwInfo->fwPackageURI == NULL)
                {
                    fw_info_free(latestFwInfo);
                    latestFwInfo = NULL;
                }
            }
        }
    }
    
    json_value_free(root_value);
    free(deviceTwinPayLoad);
}
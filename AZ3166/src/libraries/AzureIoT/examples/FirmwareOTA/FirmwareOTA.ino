// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
// To get started please visit https://microsoft.github.io/azure-iot-developer-kit/docs/projects/connect-iot-hub?utm_source=ArduinoExtension&utm_medium=ReleaseNote&utm_campaign=VSCode
#include "AZ3166WiFi.h"
#include "AzureIotHub.h"
#include "DevKitMQTTClient.h"
#include "OTAUtils.h"
#include "mico.h"
#include "OTAUpdateClient.h"
#include "SystemTime.h"

static bool hasWifi = false;
const char* currentFirmwareVersion = "1.3.7";

char *checksumToString(uint16_t checksum) {
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

char *getTimeStamp() {
  time_t t = time(NULL);
  size_t len = sizeof("2011-10-08-07:07:09");
  char* buf = (char*)malloc(len + 1);
  strftime(buf, len, "%Y-%m-%d-%R:%S", gmtime(&t));
  buf[len] = '\0';
  LogInfo("time is %s", buf);
  return(buf);
}
bool enabledOTA = true;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities
static void InitWifi()
{
  Screen.print(2, "Connecting...");
  
  if (WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    Screen.print(1, ip.get_address());
    hasWifi = true;
    Screen.print(2, "Running... \r\n");
  }
  else
  {
    hasWifi = false;
    Screen.print(1, "No Wi-Fi\r\n ");
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino sketch
void setup()
{
  Screen.init();
  Screen.print(0, "IoT DevKit");
  Screen.print(2, "Initializing...");
  
  Screen.print(3, " > Serial");
  Serial.begin(115200);

  // Initialize the WiFi module
  Screen.print(3, " > WiFi");
  hasWifi = false;
  InitWifi();
  if (!hasWifi)
  {
    return;
  }

  Screen.print(3, " > IoT Hub");
  DevKitMQTTClient_SetOption(OPTION_MINI_SOLUTION_NAME, "FirmwareOTA");
  DevKitMQTTClient_Init(true);
  IoTHubClient_SetCurrentFwInfo(currentFirmwareVersion);
  IoTHubClient_ReportOTAStatus(currentFirmwareVersion, "current");
}

void loop()
{
  if (hasWifi && enabledOTA)
  {
    DevKitMQTTClient_Check();
    FW_INFO* fwInfo = new FW_INFO;
    bool hasNewOTA = IoTHubClient_OTAHasNewFw(fwInfo);
    if (hasNewOTA) {
      if (strlen(fwInfo -> fwPackageURI) >= 6 && (strncmp("https:", fwInfo -> fwPackageURI, 6) != 0)) {
        IoTHubClient_ReportOTAStatus(currentFirmwareVersion, "error", fwInfo -> fwVersion, "URINotHTTPS");
      } else {
        if (IoTHubClient_OTAVersionCompare(fwInfo -> fwVersion, currentFirmwareVersion) == 1) {
          Screen.clean();
          Screen.print(0, "New firmware.");
          Screen.print(1, fwInfo -> fwVersion);
          LogInfo(fwInfo -> fwVersion);
          LogInfo(fwInfo -> fwPackageURI);
          LogInfo(fwInfo -> fwPackageCheckValue);
          LogInfo("%d", fwInfo -> fwSize);

          IoTHubClient_ReportOTAStatus(currentFirmwareVersion, "downloading", fwInfo -> fwVersion, NULL, getTimeStamp());
          DevKitMQTTClient_Close();
          Screen.clean();
          Screen.print("Downloading...");
          OTAUpdateClient& otaClient = OTAUpdateClient::getInstance();
          int result = otaClient.updateFromUrl(fwInfo -> fwPackageURI);
          if (result == 0) {
            Screen.print("Download success\nVeryifying...\n");
            DevKitMQTTClient_Init(true);
            IoTHubClient_ReportOTAStatus(currentFirmwareVersion, "verifying", fwInfo -> fwVersion);
            
            static volatile uint32_t flashIdx = 0;
            uint8_t *checkBuffer = (uint8_t *)malloc(1);
            int i = fwInfo -> fwSize;
            uint16_t checkSum = 0;
            CRC16_Context contex;
            CRC16_Init(&contex);
            while (i--) {
              MicoFlashRead((mico_partition_t)MICO_PARTITION_OTA_TEMP, &flashIdx, checkBuffer, 1);
              CRC16_Update(&contex, checkBuffer, 1);
            }
            free(checkBuffer);
            CRC16_Final(&contex, &checkSum);
            char *checkSumString = checksumToString(checkSum);
            LogInfo("CRC16 result: %d, %s", checkSum, checkSumString);
            Screen.clean();
            if (strcmp(checkSumString, fwInfo -> fwPackageCheckValue) == 0) {
              Screen.print("Verify success\n");
              LogInfo("Verify success");
              IoTHubClient_ReportOTAStatus(currentFirmwareVersion, "applying", fwInfo -> fwVersion, NULL, NULL, getTimeStamp());
              mico_system_reboot();
            } else {
              Screen.print("Verify failed\n");
              LogInfo("Verify failed");
              IoTHubClient_ReportOTAStatus(currentFirmwareVersion, "error", fwInfo -> fwVersion, "VerifyFailed");
              enabledOTA = false;
            }
            free(checkSumString);
          } else {
            Screen.print("Download failed\n");
            IoTHubClient_ReportOTAStatus(currentFirmwareVersion, "error", fwInfo -> fwVersion, "DownloadFailed");
            enabledOTA = false;
          }
        }
      }
    }
  }
  delay(10);
}

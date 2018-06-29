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
const char httpsCert[] = 
"-----BEGIN CERTIFICATE-----\r\nMIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\r\n"
"RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\r\nVQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\r\n"
"DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\r\nZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\r\n"
"VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\r\nmD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\r\n"
"IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\r\nmpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\r\n"
"XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\r\ndc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\r\n"
"jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\r\nBE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\r\n"
"DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\r\n9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\r\n"
"jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\r\nEpn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\r\n"
"ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\r\nR9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\r\n-----END CERTIFICATE-----\r\n";

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
          int result = otaClient.updateFromUrl(fwInfo -> fwPackageURI, httpsCert);
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

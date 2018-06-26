// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
// To get started please visit https://microsoft.github.io/azure-iot-developer-kit/docs/projects/connect-iot-hub?utm_source=ArduinoExtension&utm_medium=ReleaseNote&utm_campaign=VSCode
#include "AZ3166WiFi.h"
#include "AzureIotHub.h"
#include "DevKitMQTTClient.h"
#include "OTAUtils.h"
#include "SystemTickCounter.h"
#include "mbed_memory_status.h"

static bool hasWifi = false;
int messageCount = 1;
static bool messageSending = true;
static uint64_t send_interval_ms;
const char* currentFirmwareVersion = "1.3.7";
void printMemoryStatus() {
    mbed_stats_heap_t heap_info;
    mbed_stats_heap_get(&heap_info);
    printf("Current heap: %lu \\ %lu\r\n", heap_info.current_size, heap_info.max_size);
}

bool isClose = false;

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

static void DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payLoad, int size)
{
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
  DevKitMQTTClient_SetDeviceTwinCallback(DeviceTwinCallback);
  IoTHubClient_SetCurrentFwInfo(currentFirmwareVersion);
  IoTHubClient_ReportOTAStatus(currentFirmwareVersion, "current");
  send_interval_ms = SystemTickCounterRead();
}

void loop()
{
  if (hasWifi && !isClose)
  {
    DevKitMQTTClient_Check();
    FW_INFO* fwInfo = new FW_INFO;
    bool hasNewOTA = IoTHubClient_OTAHasNewFw(fwInfo);
    if (hasNewOTA) {
      LogInfo("Has OTA");
      LogInfo(fwInfo -> fwVersion);
      LogInfo(fwInfo -> fwPackageURI);
      LogInfo(fwInfo -> fwPackageCheckValue);
      LogInfo("%d", fwInfo -> fwSize);
      IoTHubClient_ReportOTAStatus(currentFirmwareVersion, "downloading", fwInfo -> fwVersion);
      DevKitMQTTClient_Close();
      isClose = true;
    }
  }
  delay(10);
}

// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
#include "AZ3166WiFi.h"
#include "AzureIotHub.h"
#include "IoTHubMQTTClient.h"
#include "DPSClient.h"

#include "config.h"
#include "utility.h"
#include "SystemTickCounter.h"
#include "SystemVariables.h"

extern char* Global_Device_Endpoint;
extern char* ID_Scope;

// Indicate whether WiFi is ready
static bool hasWifi = false;
int messageCount = 1;
static bool messageSending = true;
static uint64_t send_interval_ms;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities
static void InitWiFi()
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
  LogInfo("Enter DeviceTwinCallback");
  char *temp = (char *)malloc(size + 1);
  if (temp == NULL)
  {
    return;
  }
  memcpy(temp, payLoad, size);
  temp[size] = '\0';
  parseTwinMessage(updateState, temp);
  free(temp);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino sketch
void setup() {
  // put your setup code here, to run once:

  Screen.init();
  Screen.print(0, "IoT DevKit");
  Screen.print(2, "Initializing...");

  Screen.print(3, " > Serial");
  Serial.begin(115200);
  
  // Initialize the WiFi module
  Screen.print(3, " > WiFi");
  hasWifi = false;
  InitWiFi();
  if (!hasWifi)
  {
    return;
  }

  Screen.print(3, " > DPS");
  // Transfer control to firmware
  if(DPSClientStart(Global_Device_Endpoint, ID_Scope, GetBoardID()))
  {
    Screen.print(2, "DPS connected!\r\n");
  }
  else
  {
    Screen.print(2, "DPS Failed!\r\n");
    return;
  }

  Screen.print(3, " > IoT Hub(DPS)");
  IoTHubMQTT_Init(true);
  IoTHubMQTT_SetDeviceTwinCallback(DeviceTwinCallback);
}

void loop()
{
  IoTHubMQTT_Check();
  delay(200);
}

// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
// To get started please visit https://microsoft.github.io/azure-iot-developer-kit/docs/projects/remote-monitoring/?utm_source=ArduinoExtension&utm_medium=ReleaseNote&utm_campaign=VSCode
#include "AZ3166WiFi.h"
#include "AzureIotHub.h"
#include "DevKitMQTTClient.h"

#include "Sensor.h"

static bool hasWifi = false;

static DevI2C *ext_i2c;
static HTS221Sensor *ht_sensor;

static char wifiBuff[128];

#define RECONNECT_THRESHOLD 3

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities
void sendDeviceInfo()
{
    char *deviceInfo = "{\"ObjectType\":\"DeviceInfo\",\"Version\":\"1.0\",\"IsSimulatedDevice\":false,\"DeviceProperties\":{\"DeviceID\":\"AZ3166\",\"HubEnabledState\":true}}";
    DevKitMQTTClient_SendEvent(deviceInfo);
}

void InitWiFi()
{
  Screen.print("WiFi \r\n \r\nConnecting...\r\n             \r\n");
  
  if(WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    sprintf(wifiBuff, "WiFi \r\n %s\r\n %s \r\n \r\n",WiFi.SSID(),ip.get_address());
    Screen.print(wifiBuff);
    digitalWrite(LED_WIFI, 1);
    DevKitMQTTClient_Init();
    DevKitMQTTClient_SetOption(OPTION_MINI_SOLUTION_NAME, "RemoteMonitoring");
    sendDeviceInfo();
    hasWifi = true;
  }
  else
  {
    sprintf(wifiBuff, "WiFi  \r\n             \r\nNo connection\r\n                 \r\n");
    Screen.print(wifiBuff);
  }
}

void showHumidTempSensor()
{
  ht_sensor->reset();
  float temperature = 0;
  ht_sensor->getTemperature(&temperature);
  //convert from C to F
  temperature = temperature*1.8 + 32;
  float humidity = 0;
  ht_sensor->getHumidity(&humidity);
  
  char buff[128];
  sprintf(buff, "Environment \r\n Temp:%sF    \r\n Humidity:%s%% \r\n          \r\n",f2s(temperature, 1), f2s(humidity, 1));
  Screen.print(buff);
  
  char sensorData[100];
  sprintf_s(sensorData, sizeof(sensorData), "{\"DeviceId\":\"DevKit\",\"Temperature\":%s,\"Humidity\":%s}", f2s(temperature, 1), f2s(humidity, 1));
  DevKitMQTTClient_SendEvent(sensorData);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino sketch
void setup() {
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_AZURE, OUTPUT);
  pinMode(LED_USER, OUTPUT);

  ext_i2c = new DevI2C(D14, D15);
  
  ht_sensor = new HTS221Sensor(*ext_i2c);
  ht_sensor->init(NULL);
  
  //Scan networks and print them into console
  int numSsid = WiFi.scanNetworks();
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
     Serial.print(thisNet);
     Serial.print(") ");
     Serial.print(WiFi.SSID(thisNet));
     Serial.print("\tSignal: ");
     Serial.print(WiFi.RSSI(thisNet));
     Serial.print("\tEnc type: ");
     Serial.println(WiFi.encryptionType(thisNet));
  }   

  InitWiFi();
  if (!hasWifi)
  {
    return;
  }
  
  LogTrace("RemoteMonitoringSetup", NULL);
}


void loop() {
  // put your main code here, to run repeatedly:
  if(hasWifi)
  {
    showHumidTempSensor();
    digitalWrite(LED_AZURE, 1);
    delay(500);
    digitalWrite(LED_AZURE, 0);
    delay(5000);
  }
  else
  {
    delay(100);
  }
}

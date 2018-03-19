// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
// To get started please visit https://microsoft.github.io/azure-iot-developer-kit/docs/projects/remote-monitoring/?utm_source=ArduinoExtension&utm_medium=ReleaseNote&utm_campaign=VSCode
#include "Arduino.h"
#include "Sensor.h"
#include "AzureIotHub.h"
#include "AZ3166WiFi.h"
#include "DevKitMQTTClient.h"
#include "Telemetry.h"
#include "SystemTime.h"
#include "SystemTickCounter.h"

#define INTERVAL 5000

static bool isConnected = false;

DevI2C *ext_i2c;
HTS221Sensor *ht_sensor;
LPS22HBSensor *lp_sensor;

char wifiBuff[128];

float temperature = 50;
char temperatureUnit = 'F';
float humidity = 50;
char humidityUnit = '%';
float pressure = 55;
const char *pressureUnit = "psig";

const char *roomSchema = "chiller-sensors;v1";
const char *interval = "00:00:05";
const char *deviceType = "Chiller";
const char *deviceFirmware = "1.0.0";
const char *deviceFirmwareUpdateStatus = "";
const char *deviceLocation = "Madrid";
const double deviceLatitude = 40.418371;
const double deviceLongitude =-3.797997;

#define RECONNECT_THRESHOLD 3

const char *twinProperties="{\"Protocol\": \"MQTT\", \"SupportedMethods\": \"Reboot,FirmwareUpdate,LedColor\", \"Telemetry\": { \"%s\": {\"Interval\": \"%s\",\"MessageTemplate\": \"{\\\"temperature\\\":${temperature},\\\"temperature_unit\\\":\\\"${temperature_unit}\\\", \\\"humidity\\\":${humidity},\\\"humidity_unit\\\":\\\"${humidity_unit}\\\",\\\"pressure\\\":${pressure},\\\"pressure_unit\\\":\\\"${pressure_unit}\\\"}\",\"MessageSchema\": {\"Name\": \"%s\",\"Format\": \"JSON\",\"Fields\": {\"temperature\": \"Double\", \"temperature_unit\": \"Text\",\"humidity\": \"Double\",\"humidity_unit\": \"Text\",\"pressure\": \"Double\",\"pressure_unit\": \"Text\" } } } },\"Type\": \"%s\",\"Firmware\": \"%s\",\"Model\":\"AZ3166\",\"FirmwareUpdateStatus\": \"%s\",\"Location\": \"%s\",\"Latitude\": %f,\"Longitude\": %f}";

bool sendDeviceInfo()
{
  char reportedProperties[2048];
  snprintf(reportedProperties,2048, twinProperties,roomSchema, interval ,roomSchema,deviceType,deviceFirmware,deviceFirmwareUpdateStatus,deviceLocation,deviceLatitude,deviceLongitude);
  return DevKitMQTTClient_ReportState(reportedProperties);
}

void twinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payLoad, int length){
  LogInfo("*** Twin State: %s",updateState?"Complete":"Partial");
}

RGB_LED rgbLed;
uint8_t colors[][3] = {
        {255, 0, 0},                // red
        {0, 255, 0},                // green
        {0, 0, 255},                // blue
        {0, 0, 0},
        {255, 255, 0},
        {0, 255, 255},
        {255, 0, 255},
        {255, 255, 255}
    };
uint8_t colorindex;
bool doReset=false;

int device_method_callback(const char *methodName, const unsigned char *payload, int length, unsigned char **response, int *responseLength){

  LogInfo("*** Remote method: %s",methodName);  

  if(strcmp(methodName,"LedColor")==0){
    int n=sizeof(colors)/sizeof(colors[0]);
    uint8_t *color=colors[(colorindex++)%n];
    rgbLed.setColor(color[0],color[1],color[2]);

    const char *ok="{\"result\":\"OK\"}";
    *responseLength=strlen(ok);
    *response = (unsigned char*)malloc(*responseLength);
    strncpy((char *)(*response), ok, *responseLength);
    return 200;
  }

  if(strcmp(methodName,"Reboot")==0){
    doReset=true;
    
    const char *reset="{\"result\":\"RESET\"}";    
    *responseLength=strlen(reset);
    *response = (unsigned char*)malloc(*responseLength);
    strncpy((char *)(*response), reset, *responseLength);
    return 201;
  }

  LogError("*** Remote method: %s not found",methodName);
  return 500;
}

void InitWiFi()
{
  Screen.print("WiFi \r\n \r\nConnecting...\r\n             \r\n");
  
  if(WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    sprintf(wifiBuff, "WiFi \r\n %s\r\n %s \r\n \r\n",WiFi.SSID(),ip.get_address());
    Screen.print(wifiBuff);
    isConnected = true;
  }
  else
  {
    sprintf(wifiBuff, "WiFi  \r\n             \r\nNo connection\r\n                 \r\n");
    Screen.print(wifiBuff);
  }
}

void sendData(const char *data, const char *schema){
  time_t t = time(NULL);
  char buf[sizeof "2011-10-08T07:07:09Z"];
  strftime(buf, sizeof buf, "%FT%TZ", gmtime(&t));

  EVENT_INSTANCE* message = DevKitMQTTClient_Event_Generate(data, MESSAGE);

  DevKitMQTTClient_Event_AddProp(message, "$$CreationTimeUtc", buf);
  DevKitMQTTClient_Event_AddProp(message, "$$MessageSchema", schema);
  DevKitMQTTClient_Event_AddProp(message, "$$ContentType", "JSON");
  
  DevKitMQTTClient_SendEventInstance(message);
}

void showSensors()
{
  try
  {
    ht_sensor->reset();
    ht_sensor->getTemperature(&temperature);
    //convert from C to F
    temperature = temperature*1.8 + 32;

    ht_sensor->getHumidity(&humidity);

    lp_sensor->getPressure(&pressure);
    
    char buff[128];
    sprintf(buff, "Environment \r\n Temp:%s%c    \r\n Humidity:%s%c  \r\n Atm: %s%s",f2s(temperature, 1),temperatureUnit, f2s(humidity, 1), humidityUnit, f2s(pressure,1), pressureUnit);
    Screen.print(buff);

    char sensorData[200];
    sprintf_s(sensorData, sizeof(sensorData), "{\"temperature\":%s,\"temperature_unit\":\"%c\",\"humidity\":%s,\"humidity_unit\":\"%c\",\"pressure\":%s,\"pressure_unit\":\"%s\"}", f2s(temperature, 1), temperatureUnit,f2s(humidity, 1), humidityUnit,f2s(pressure, 1), pressureUnit);
    sendData(sensorData,roomSchema);
  }
  catch(int error)
  {
    LogError("*** Read sensor failed: %d",error);
  }
}

void setup() {

  ext_i2c = new DevI2C(D14, D15);
  
  ht_sensor = new HTS221Sensor(*ext_i2c);
  ht_sensor->init(NULL);

  lp_sensor= new LPS22HBSensor(*ext_i2c);
  lp_sensor->init(NULL);
  
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

  if (isConnected)
  {
    // Microsoft collects data to operate effectively and provide you the best experiences with our products. 
    // We collect data about the features you use, how often you use them, and how you use them.
    send_telemetry_data_async("", "RemoteMonitoringSetupV2", "");

    //setup the MQTT Client
    DevKitMQTTClient_SetDeviceMethodCallback(&device_method_callback);
    DevKitMQTTClient_SetDeviceTwinCallback(&twinCallback);
    DevKitMQTTClient_SetOption(OPTION_MINI_SOLUTION_NAME, "RemoteMonitoringV2");
    DevKitMQTTClient_Init(true); //set to true to use twin
    
    // Send the Twin data for the Remote Monitoring
    bool infoSent=sendDeviceInfo();
    LogInfo("*** Twin update: %s",infoSent?"yes":"no");
  }
}



static uint64_t send_interval_ms;
static uint64_t reset_interval_ms;

void loop() {
  // put your main code here, to run repeatedly:
  if(isConnected)
  {
    if((int)(SystemTickCounterRead() - send_interval_ms)>INTERVAL)
    {
      showSensors();
      send_interval_ms = SystemTickCounterRead();
    }

    if((int)(SystemTickCounterRead() - reset_interval_ms)>INTERVAL){
      if(doReset){
         NVIC_SystemReset();
      }
      reset_interval_ms = SystemTickCounterRead();
    }
  }
}

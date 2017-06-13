#include "Arduino.h"
#include "lps22hb.h"
#include "AzureIotHub.h"
#include "OLEDDisplay.h"
#include "AZ3166WiFi.h"
#include "_iothub_client_sample_mqtt.h"

static bool isConnected = false;

DevI2C *ext_i2c;
HTS221Sensor *ht_sensor;

char wifiBuff[128];

#define RECONNECT_THRESHOLD 3

static void SendEventToIoTHub(const char* msg)
{
  iothub_client_sample_send_event((const unsigned char *)msg);
  iothub_client_sample_mqtt_loop();
}

void sendDeviceInfo()
{
    char *deviceInfo = "{\"ObjectType\":\"DeviceInfo\",\"Version\":\"1.0\",\"IsSimulatedDevice\":false,\"DeviceProperties\":{\"DeviceID\":\"AZ3166\",\"HubEnabledState\":true}}";
    SendEventToIoTHub(deviceInfo);
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
    iothub_client_sample_mqtt_init();
    sendDeviceInfo();
    isConnected = true;
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
  SendEventToIoTHub(sensorData);
}

/* float to string
 * f is the float to turn into a string
 * p is the precision (number of decimals)
 * return a string representation of the float.
 */
char *f2s(float f, int p){
  char * pBuff;                         // use to remember which part of the buffer to use for dtostrf
  const int iSize = 10;                 // number of bufffers, one for each float before wrapping around
  static char sBuff[iSize][20];         // space for 20 characters including NULL terminator for each float
  static int iCount = 0;                // keep a tab of next place in sBuff to use
  pBuff = sBuff[iCount];                // use this buffer
  if(iCount >= iSize -1){               // check for wrap
    iCount = 0;                         // if wrapping start again and reset
  }
  else{
    iCount++;                           // advance the counter
  }
  return dtostrf(f, 0, p, pBuff);       // call the library function
}

/*
 * As there is a problem of sprintf %f in Arduino,
   follow https://github.com/blynkkk/blynk-library/issues/14 to implement dtostrf
 */
char * dtostrf(double number, signed char width, unsigned char prec, char *s) {
    if(isnan(number)) {
        strcpy(s, "nan");
        return s;
    }
    if(isinf(number)) {
        strcpy(s, "inf");
        return s;
    }

    if(number > 4294967040.0 || number < -4294967040.0) {
        strcpy(s, "ovf");
        return s;
    }
    char* out = s;
    // Handle negative numbers
    if(number < 0.0) {
        *out = '-';
        ++out;
        number = -number;
    }
    // Round correctly so that print(1.999, 2) prints as "2.00"
    double rounding = 0.5;
    for(uint8_t i = 0; i < prec; ++i) {
      rounding /= 10.0;
    }
    number += rounding;

    // Extract the integer part of the number and print it
    unsigned long int_part = (unsigned long) number;
    double remainder = number - (double) int_part;
    out += sprintf(out, "%d", int_part);

    // Print the decimal point, but only if there are digits beyond
    if(prec > 0) {
        *out = '.';
        ++out;
    }

    while(prec-- > 0) {
        remainder *= 10.0;
        if((int)remainder == 0){
                *out = '0';
                 ++out;
        }
    }
    sprintf(out, "%d", (int) remainder);
    return s;
}


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
}


void loop() {
  // put your main code here, to run repeatedly:
  if(isConnected)
  {
    showHumidTempSensor();
    digitalWrite(LED_AZURE, 1);
    delay(500);
    digitalWrite(LED_AZURE, 0);
    delay(5000);
  }
}
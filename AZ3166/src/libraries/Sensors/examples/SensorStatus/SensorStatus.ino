#include "AZ3166WiFi.h"
#include "Sensor.h"
#include "SystemVersion.h"
#include "http_client.h"
#include "telemetry.h"

#define NUMSENSORS 5  // 5 sensors to display

// 0 - Humidity & Temperature Sensor
// 1 - Pressure Sensor
// 2 - Magnetic Sensor
// 3 - Gyro Sensor
// 4 - Motion (Acceleration) Sensor
static int status;
static bool showSensor;
static bool isConnected;
static unsigned char counter;

static struct _tagRGB
{
  int red;
  int green;
  int blue;
} _rgb[] =
{
  { 255,   0,   0 },
  {   0, 255,   0 },
  {   0,   0, 255 },
};

static RGB_LED rgbLed;
static int color = 0;
static int led = 0;

DevI2C *ext_i2c;
LSM6DSLSensor *acc_gyro;
HTS221Sensor *ht_sensor;
LIS2MDLSensor *magnetometer;
IRDASensor *IrdaSensor;
LPS22HBSensor *pressureSensor;

int axes[3];
char wifiBuff[128];
char firmwareBuff[128];
char firmwareTelemetryBuff[64];

const char GITHUB_CERT[] =
"-----BEGIN CERTIFICATE-----\r\nMIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\r\nMQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\nd3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\r\nZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\r\nMAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\r\nLmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\r\nRVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\r\n+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\r\nPNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\r\nxChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\r\nIk5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\r\nhzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\r\nEsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\r\nMAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\r\nFLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\r\nnzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\r\neM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\r\nhS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\r\nYzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\r\nvEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\r\n+OkuE6N36B9K\r\n-----END CERTIFICATE-----\r\n";
static const char *FIRMWARE_VERSION_PATH = "https://microsoft.github.io/azure-iot-developer-kit/firmware.txt";

void getLatestFirmwareVersion()
{
  HTTPClient client = HTTPClient(GITHUB_CERT, HTTP_GET, FIRMWARE_VERSION_PATH);
  const Http_Response *response = client.send(NULL, 0);
  if (response != NULL)
  {
    snprintf(firmwareTelemetryBuff, 64, "Local: %s, Remote: %s", getDevkitVersion(), response->body);

    // Microsoft collects data to operate effectively and provide you the best experiences with our products.
    // We collect data about the features you use, how often you use them, and how you use them.
    send_telemetry_data("", "SensorStatusFirmwareVersionSucceeded", firmwareTelemetryBuff);

    if (strcmp(response->body, getDevkitVersion()) == 0)
    {
      snprintf(firmwareBuff, 128, "IoT DevKit\r\nCurrent: %s\r\n \r\nBtn B: Sensors\r\n", getDevkitVersion());
    }
    else
    {
      snprintf(firmwareBuff, 128, "IoT DevKit\r\nCurrent: %s\r\nLatest: %s\r\nBtn B: Sensors\r\n", getDevkitVersion(), response->body);
    }
  }
  else
  {
    snprintf(firmwareTelemetryBuff, 64, "Local: %s", getDevkitVersion());

    // Microsoft collects data to operate effectively and provide you the best experiences with our products.
    // We collect data about the features you use, how often you use them, and how you use them.
    send_telemetry_data("", "SensorStatusFirmwareVersionFailed", firmwareTelemetryBuff);

    snprintf(firmwareBuff, 128, "IoT DevKit\r\nCurrent: %s\r\nLatest: N/A\r\nBtn B: Sensor\r\n", getDevkitVersion());
  }
  Screen.print(firmwareBuff);
}

void InitWiFi()
{
  Screen.print("WiFi \r\n \r\nConnecting...\r\n             \r\n");
  
  if(WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    snprintf(wifiBuff, 128, "WiFi Connected\r\n%s\r\n%s\r\n \r\n",WiFi.SSID(),ip.get_address());
    Screen.print(wifiBuff);
    getLatestFirmwareVersion();
    isConnected = true;
  }
  else
  {
    snprintf(wifiBuff, 128, "No WiFi\r\nEnter AP Mode\r\nto config\r\n                 \r\n");
    Screen.print(wifiBuff);
  }
}

void showMotionGyroSensor()
{
  acc_gyro->getGAxes(axes);
  char buff[128];
  snprintf(buff, 128, "Gyroscope \r\n    x:%d   \r\n    y:%d   \r\n    z:%d  ", axes[0], axes[1], axes[2]);
  Screen.print(buff);
}

void showMotionAccelSensor()
{
  acc_gyro->getXAxes(axes);
  char buff[128];
  snprintf(buff, 128, "Accelorometer \r\n    x:%d   \r\n    y:%d   \r\n    z:%d  ", axes[0], axes[1], axes[2]);
  Screen.print(buff);
}

void showPressureSensor()
{
  float pressure = 0;
  float temperature = 0;
  pressureSensor -> getPressure(&pressure);
  pressureSensor -> getTemperature(&temperature);
  char buff[128];
  snprintf(buff, 128, "Environment\r\nPressure: \r\n    %shPa\r\nTemp: %sC \r\n",f2s(pressure, 2), f2s(temperature, 1));
  Screen.print(buff);
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
  snprintf(buff, 128, "Environment \r\n Temp:%sF    \r\n Humidity:%s%% \r\n          \r\n",f2s(temperature, 1), f2s(humidity, 1));
  Screen.print(buff);
}

void showMagneticSensor()
{
  magnetometer->getMAxes(axes);
  char buff[128];
  snprintf(buff, 128, "Magnetometer  \r\n    x:%d     \r\n    y:%d     \r\n    z:%d     ", axes[0], axes[1], axes[2]);
  Screen.print(buff);
}

bool IsButtonClicked(unsigned char ulPin)
{
    pinMode(ulPin, INPUT);
    int buttonState = digitalRead(ulPin);
    if(buttonState == LOW)
    {
        return true;
    }
    return false;
}

void setup() {
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_AZURE, OUTPUT);
  pinMode(LED_USER, OUTPUT);
  rgbLed.turnOff();

  ext_i2c = new DevI2C(D14, D15);
  acc_gyro = new LSM6DSLSensor(*ext_i2c, D4, D5);
  acc_gyro->init(NULL);
  acc_gyro->enableAccelerator();
  acc_gyro->enableGyroscope();
  
  ht_sensor = new HTS221Sensor(*ext_i2c);
  ht_sensor->init(NULL);

  magnetometer = new LIS2MDLSensor(*ext_i2c);
  magnetometer->init(NULL);

  IrdaSensor = new IRDASensor();
  IrdaSensor->init();

  pressureSensor = new LPS22HBSensor(*ext_i2c);
  pressureSensor -> init(NULL);
  
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

  status = 4;
  counter = 0;
  showSensor = false;
  isConnected = false;
  InitWiFi();
}


void loop() {
  // put your main code here, to run repeatedly:

  /*Blink around every 0.5 sec*/
  counter++;
  int irda_status = IrdaSensor->IRDATransmit(&counter, 1, 100 );
  if(irda_status != 0)
  {
    Serial.println("Unable to transmit through IRDA");
  }

  if(counter > 5)
  {
      digitalWrite(LED_WIFI, led);
      digitalWrite(LED_AZURE, led);
      digitalWrite(LED_USER, led);
      led = !led;
    
      rgbLed.setColor(_rgb[color].red, _rgb[color].green, _rgb[color].blue);
      color = (color + 1) % (sizeof(_rgb) / sizeof(struct _tagRGB));
      counter = 0;
  }
 
  if(isConnected && IsButtonClicked(USER_BUTTON_B))
  {
      status = (status + 1) % NUMSENSORS;
      showSensor = true;
      delay(50);
  }

  if(isConnected && IsButtonClicked(USER_BUTTON_A))
  {
      showSensor = false;
      Screen.print("Who said\r\nCogito ergo\r\nsum?\r\n \r\n");
      delay(50);
  }

  if(showSensor)
  {
    switch(status)
    {
      case 0:
        showHumidTempSensor();
        break;
      case 1:
        showPressureSensor();
        break; 
      case 2:
        showMagneticSensor();
        break;
      case 3:
        showMotionGyroSensor();
        break;
      case 4:
        showMotionAccelSensor();
        break;
    }
	delay(100);
  }
  
}

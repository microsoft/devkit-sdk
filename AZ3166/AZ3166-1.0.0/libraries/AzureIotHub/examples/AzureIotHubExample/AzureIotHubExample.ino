#include "_iothub_client_sample_mqtt.h"
#include <stdlib.h>
#include "WiFi.h"
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <HTS221Sensor.h>
#include <lis2mdl_class.h>
#include <lps25h_class.h>
#include <LSM6DSLSensor.h>
#include "AzureIotHub.h"
#include <OLEDDisplay.h>

/*** I2C ***/
/* Use Arduino I2C Connectors */
#define IKS01A2_PIN_I2C_SDA (D14)
#define IKS01A2_PIN_I2C_SCL (D15)

/* LSM6DSL INT1 */
#define IKS01A2_PIN_LSM6DSL_INT1 (D4)
/* LSM6DSL INT2 */
#define IKS01A2_PIN_LSM6DSL_INT2 (D5)

DevI2C *ext_i2c;
LSM6DSLSensor *acc_gyro;
int32_t axes[3];

const int32_t acc_threshold = 1400; // about 0.98 * 1000 * 1.4

static char msgText[1024];
static char temp[100];

#define buttonPin USER_BUTTON_A
#define heartBeatPin RGB_R
#define lightPin RGB_B

bool buttonStateChanged = false;
bool shakeStateChanged = false;
volatile byte buttonState = HIGH;
volatile byte shakeState = LOW;
bool heartBeat = false;

OLEDDisplay oled;

void showMessage(const char *tweet)
{
  Serial.println("*****************************TurnLightOn********************************");
  digitalWrite(lightPin, LOW);
  oled.clean();
  oled.print(tweet, true);
}

void buttonStateHook()
{
  auto prev = buttonState;
  buttonState = digitalRead(buttonPin);
  buttonStateChanged = prev != buttonState;
}

void shakeStateHook()
{
  auto prev = shakeState;
  acc_gyro->get_x_axes(axes);
  shakeState = abs(axes[0]) + abs(axes[1]) + abs(axes[2]) > acc_threshold;
  shakeStateChanged = prev != shakeState;
}
void reportPulse()
{
  digitalWrite(lightPin, HIGH);
  Serial.println("*****************************SendEvent********************************");
  sprintf(msgText, "{\"topic\":\"%s\", \"DeviceID\":\"%s\"}", "iot", "myDevice1");
  iothub_client_sample_send_event((const unsigned char *)msgText);
}
void InitWiFi()
{
  Screen.print("Azure IoT DevKit\r\n \r\nConnecting...\r\n");

  if (WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    Screen.print(1, ip.get_address());
    hasWifi = true;
    Screen.print(2, "Running...      \r\n");
  }
  else
  {
    Screen.print(1, "No Wi-Fi\r\n                ");
  }
}
void setup()
{
  pinMode(heartBeatPin, OUTPUT);
  pinMode(lightPin, OUTPUT);
  pinMode(buttonPin, INPUT);

  Serial.begin(115200);
  Serial.println("start\r\n");
  digitalWrite(lightPin, HIGH);
  InitWiFi();
  if (!hasWifi)
  {
    return;
  }
  ext_i2c = new DevI2C(D14, D15);
  acc_gyro = new LSM6DSLSensor(*ext_i2c, D4, D5);
  acc_gyro->init(NULL);
  acc_gyro->enable_x();
  acc_gyro->enable_g();

  iothub_client_sample_mqtt_init();
  oled.clean();
  oled.print("Shake!\n ~ Shake!");
  reportPulse();
  Serial.println("reported");
}

void loop()
{
  buttonStateHook();
  shakeStateHook();
  if ((buttonStateChanged && buttonState == LOW) || (shakeStateChanged && shakeState == HIGH))
  {
    buttonStateChanged = shakeStateChanged = false;
    reportPulse();
  }
  heartBeat = !heartBeat;
  digitalWrite(heartBeatPin, heartBeat ? HIGH : LOW);
  if (hasWifi)
  {
    iothub_client_sample_mqtt_loop();
  }
  else
  {
    delay(100);
  }
}
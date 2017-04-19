#include "RGB_LED.h"
#include "LSM6DSLSensor.h"

static struct _tagRGB
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
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
int32_t axes[3];

void setup() {
  Screen.print(0, "Azure IoT DevKit");
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_AZURE, OUTPUT);
  pinMode(LED_USER, OUTPUT);
  rgbLed.turnOff();

  ext_i2c = new DevI2C(D14, D15);
  acc_gyro = new LSM6DSLSensor(*ext_i2c, D4, D5);
  acc_gyro->init(NULL);
  acc_gyro->enable_x();
  acc_gyro->enable_g();
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_WIFI, led);
  digitalWrite(LED_AZURE, led);
  digitalWrite(LED_USER, led);
  led = !led;
  
  rgbLed.setColor(_rgb[color].red, _rgb[color].green, _rgb[color].blue);
  color = (color + 1) % (sizeof(_rgb) / sizeof(struct _tagRGB));

  acc_gyro->get_x_axes(axes);
  char buff[128];
  sprintf(buff, "Azure IoT DevKit\r\n     x:%d           \r\n     y:%d           \r\n     z:%d             ", axes[0], axes[1], axes[2]);
  Screen.print(buff);
  
  delay(500);
}



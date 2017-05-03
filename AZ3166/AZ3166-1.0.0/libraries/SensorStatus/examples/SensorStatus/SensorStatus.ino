#include "RGB_LED.h"
#include "AZ3166WiFi.h"
#include "lps22hb.h"

#define NUMSENSORS 4  // 4 sensors to display

// 0 - Motion&Gyro Sensor
// 1 - Pressure Sensor
// 2 - Humidity & Temperature Sensor
// 3 - Magnetic Sensor
static int status;


static int counter;

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
HTS221Sensor *ht_sensor;
LIS2MDL *magnetometer;

int32_t axes[3];


void showMotionGyroSensor()
{
  acc_gyro->get_x_axes(axes);
  char buff[128];
  sprintf(buff, "Azure IoT DevKit\r\nGyroscopeX:%d   \r\nGyroscopeY:%d   \r\nGyroscopeZ:%d  ", axes[0], axes[1], axes[2]);
  Screen.print(buff);
}

void showPressureSensor()
{
  float pressure = 0;
  float temperature = 0;
  lps25hb_Read_Data(&temperature, &pressure);
  char buff[128];
  sprintf(buff, "Azure IoT DevKit\r\n Pressure:%s      \r\n                 \r\n             \r\n",f2s(pressure, 2));
  Screen.print(buff);
}

void showHumidTempSensor()
{
  ht_sensor->reset();
  float temperature = 0;
  ht_sensor->getTemperature(&temperature);
  float humidity = 0;
  ht_sensor->getHumidity(&humidity);
  
  char buff[128];
  sprintf(buff, "Azure IoT DevKit\r\nTemperature:%s     \r\nHumidity:%s    \r\n          \r\n",f2s(temperature, 1), f2s(humidity, 2));
  Screen.print(buff);
}

void showMagneticSensor()
{
  magnetometer->get_m_axes(axes);
  char buff[128];
  sprintf(buff, "Azure IoT DevKit\r\nMagnetic x:%d     \r\nMagnetic y:%d     \r\nMagnetic z:%d     ", axes[0], axes[1], axes[2]);
  Screen.print(buff);
}

bool IsButtonClicked(uint32_t ulPin)
{
    pinMode(ulPin, INPUT);
    int buttonState = digitalRead(ulPin);
    if(buttonState == LOW)
    {
        return true;
    }
    return false;
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
    for(uint8_t i = 0; i < prec; ++i)
        rounding /= 10.0;
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
  lps25hb_sensor_init( );
  
  ht_sensor = new HTS221Sensor(*ext_i2c);
  ht_sensor->init(NULL);

  magnetometer = new LIS2MDL(*ext_i2c);
  magnetometer->init(NULL);
  
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

  status = 0;
  counter = 0;
}


void loop() {
  // put your main code here, to run repeatedly:

  /*Blink around every 0.5 sec*/
  counter++;
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

  if(IsButtonClicked(USER_BUTTON_A))
  {
      Screen.print("Azure IoT DevKit\r\n Button A   \r\n is clicked    \r\n          \r\n");
      status = (status - 1 + NUMSENSORS) % NUMSENSORS;
      delay(50);
  }
  else if(IsButtonClicked(USER_BUTTON_B))
  {
      Screen.print("Azure IoT DevKit\r\n Button B   \r\n is clicked    \r\n         \r\n");
      status = (status + 1) % NUMSENSORS;
      delay(50);
  }

  switch(status)
  {
    case 0:
        showMotionGyroSensor();
        break;
    case 1:
        showPressureSensor();
        break; 
    case 2:
        showHumidTempSensor();
        break;
    case 3:
        showMagneticSensor();
        break; 
  }
  delay(50);
}



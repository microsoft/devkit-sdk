#include <ArduinoUnit.h>
#include "HTS221Sensor.h"
#include "lis2mdlSensor.h"
#include "LSM6DSLSensor.h"
#include "RGB_LED.h"

#define RetVal_OK           0
#define LOOP_DELAY          500

DevI2C *i2c;
HTS221Sensor *hts221;
LIS2MDLSensor *lis2mdl;
LSM6DSLSensor *lsm6dsl;
float humidity = 0;
float temperature = 0;
uint8_t id;
int axes[3];
int16_t raw[3];
float data;
RGB_LED rgbLed;
uint8_t color[][3] = {{255, 0, 0},  // red
                      {0, 255, 0},  // green
                      {0, 0, 255},   // blue
                      {0, 0, 0},
                      {255, 255, 0},
                      {0, 255, 255},
                      {255, 0, 255},
                      {255, 255, 255}
                     };

void setup() {
    Serial.println(__FILE__);
    
    i2c = new DevI2C(D14, D15);

    // init the hts221 sensor
    hts221 = new HTS221Sensor(*i2c); 
    assertEqual(hts221 -> init(NULL), RetVal_OK);

    // init lis2mdl sensor
    lis2mdl = new LIS2MDLSensor(*i2c);
    assertEqual(lis2mdl -> init(NULL), RetVal_OK);
    
    // init lsm6dsl sensor
    lsm6dsl = new LSM6DSLSensor(*i2c, D4, D5);
    assertEqual(lsm6dsl -> init(NULL), RetVal_OK);
}

void loop() {
    Test::run();
}

test(sensor_hts221)
{
    // enable
    assertEqual(hts221 -> enable(), RetVal_OK);

    // read id
    assertEqual(hts221 -> readId(&id), RetVal_OK);
    
    // get humidity
    assertEqual(hts221 -> getHumidity(&humidity), RetVal_OK);
    assertMoreOrEqual(humidity, 0);
    assertLessOrEqual(humidity, 100);
    
    // get temperature
    assertEqual(hts221 -> getTemperature(&temperature), RetVal_OK);
    assertMoreOrEqual(temperature, 0);
    assertLessOrEqual(temperature, 100);

    // disable the sensor
    assertEqual(hts221 -> disable(), RetVal_OK);

    // reset
    assertEqual(hts221 -> reset(), RetVal_OK);
    
    delay(LOOP_DELAY);
}

test(sensor_lis2mdl)
{
    // read id
    assertEqual(lis2mdl->readId(&id), RetVal_OK);
    
    // get_m_axes
    assertEqual(lis2mdl->getMAxes(axes), RetVal_OK);

    delay(LOOP_DELAY);
}

test(sensor_lsm6dsl)
{
    // Accelerometer test
    accelerometer_test();
    
    // Gyroscope test
    gyroscope_test();

    delay(LOOP_DELAY);
}

void accelerometer_test(){   
    // read id
    assertEqual(lsm6dsl->readId(&id), RetVal_OK);

    // getXAxes
    assertEqual(lsm6dsl->getXAxes(axes), RetVal_OK);

    // getXSensitivity
    assertEqual(lsm6dsl->getXSensitivity(&data), RetVal_OK);
}

void gyroscope_test(){
    // getGAxes
    assertEqual(lsm6dsl->getGAxes(axes), RetVal_OK);

    // getGSensitivity
    assertEqual(lsm6dsl->getGSensitivity(&data), RetVal_OK);
}

test(sensor_rgbled)
{
    for(int i = 0; i< 8; ++i)
    {
      Serial.printf("Red: %d, Green: %d, Blue: %d\n", color[i][0], color[i][1], color[i][2]);      
      rgbLed.setColor(color[i][0], color[i][1], color[i][2]);
      delay(LOOP_DELAY);
    }
    
    Serial.println("Turn off");
    rgbLed.turnOff();

    delay(LOOP_DELAY);
}

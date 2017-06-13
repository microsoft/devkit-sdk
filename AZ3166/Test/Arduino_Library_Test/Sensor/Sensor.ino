#include "HTS221Sensor.h"
//#include "IrDASensor.h"
#include "lis2mdlSensor.h"
#include "LSM6DSLSensor.h"
#include "RGB_LED.h"

#define RetVal_OK           0
#define LOOP_DELAY          500

DevI2C *i2c;
HTS221Sensor *hts221;
//IRDASensor *irad
LIS2MDLSensor *lis2mdl;
LSM6DSLSensor *lsm6dsl;
float humidity = 0;
float temperature = 0;
uint8_t id;
int counter = 1;
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
    Serial.println(">> Start");
    Serial.println(__FILE__);
    
    i2c = new DevI2C(D14, D15);

    // init the hts221 sensor
    hts221 = new HTS221Sensor(*i2c); 
    if(hts221 -> init(NULL) != RetVal_OK)
    {
        Serial.println("[HTS221Sensor]: Error: Failed to init the sensor");
        return;
    } 

    // init the irad sensor
    //irad = new IRDASensor(); 
    //if(irad -> init(NULL) != RetVal_OK)
    //{
    //    Serial.println("[IRDASensor]: Error: Failed to init the sensor");
    //    return;
    //}

    // init lis2mdl sensor
    lis2mdl = new LIS2MDLSensor(*i2c);
    if(lis2mdl->init(NULL) != RetVal_OK)
    {
        Serial.println("[LIS2MDL]: Error: Failed to init LIS2MDL");
        return;
    }
    
    // init lsm6dsl sensor
    lsm6dsl = new LSM6DSLSensor(*i2c, D4, D5);
    if(lsm6dsl->init(NULL) != RetVal_OK)
    {
        Serial.println("[LSM6DSL Sensor]: Error: Failed to init the LSM6DSL sensor");
        return;
    }

}

void loop() {
  while(counter <= 5)
  {
    Serial.printf(">> Start (%d)\r\n", counter);
    runCase();
    Serial.printf(">> End (%d)\r\n", counter); 

    if(counter == 5)
    {
        Serial.println(">> End");
    }
    
    counter++;
  }
}

void runCase()
{
    Serial.println("Verify hts221 sensor");
    check_sensorhts221();

    Serial.println("Verify lis2mdl sensor");
    check_sensorlis2mdl();

    Serial.println("Verify lsm6dsl sensor");
    check_sensorlsm6dsl();

    Serial.println("Verify rgb_led sensor");
    check_sensorrgbled();
}

void check_sensorhts221()
{
    // enable
  if(hts221 -> enable() != RetVal_OK)
  {
    Serial.println("[HTS221Sensor]: Error: Failed to enable the sensor");
    return;
  }

  // read id
  if(hts221 -> readId(&id) != RetVal_OK)
  {    
    Serial.println("[HTS221Sensor]: Error: Failed to read ID");    
  }  
  else
  {
    Serial.print("ID: ");
    Serial.println(id);
  }
  
  // get humidity
  if(hts221 -> getHumidity(&humidity) != RetVal_OK)
  {
    Serial.println("[HTS221Sensor]: Error: Failed to get humidity");    
  }  
  else
  {
    Serial.print("Humidity: ");
    Serial.println(humidity);

    if(humidity < 0 || humidity > 100)
    {
      Serial.println("[HTS221Sensor]: Error: Humidity should be between 0 and 100");    
    }
  }
  
  // get temperature
  if(hts221 -> getTemperature(&temperature) != RetVal_OK)
  {
    Serial.println("[HTS221Sensor]: Error: Failed to get temperature");    
  }
  else
  {
    Serial.print("Temperature: ");
    Serial.println(temperature);

    if(temperature < 0 || temperature > 100)
    {
      Serial.println("[HTS221Sensor]: Error: Temperature should be between 0 and 100");
    }
  }

  // disable the sensor
  if(hts221 -> disable() != RetVal_OK)
  {
    Serial.println("[HTS221Sensor]: Error: Failed to disable the sensor");
    return;
  }

  // reset
  if(hts221 -> reset() != RetVal_OK)
  {
    Serial.println("[HTS221Sensor]: Error: Failed to reset the sensor");
    return;
  }
  
  delay(LOOP_DELAY);
}

void check_irad()
{

}
void check_sensorlis2mdl()
{
    // read id
    if(lis2mdl->readId(&id) != RetVal_OK)
    {
        Serial.println("[LIS2MDL]: Error: Failed to read id");
    }
    else
    {
        Serial.printf("Id: %d\n", id);
    }
    
    // get_m_axes
    if(lis2mdl->get_m_axes(axes) != RetVal_OK)
    {
        Serial.println("[LIS2MDL]: Error: Failed to get m axes");
    }
    else
    {
        Serial.printf("Axes: x - %d, y - %d, z - %d\n", axes[0], axes[1], axes[2]);
    }

    delay(LOOP_DELAY);
}

void check_sensorlsm6dsl()
{
    // Accelerometer test
    accelerometer_test();
    
    // Gyroscope test
    gyroscope_test();

    delay(LOOP_DELAY);
}

void accelerometer_test(){
    // enable Accelerator
    //if(lsm6dsl->enableAccelerator() !=  RetVal_OK)
    //{
     // Serial.println("[LSM6DSL Sensor]: Error: Failed to enable accelerator");
     // return;
    //}
    
    // read id
    if(lsm6dsl->readId(&id) != RetVal_OK)
    {
        Serial.println("[LSM6DSL Sensor]: Error: Failed to read id");
    }
    else
    {
        Serial.printf("Id: %d\n", id);
    }

    // getXAxes
    if(lsm6dsl->getXAxes(axes) != RetVal_OK)
    {
        Serial.println("[LSM6DSL Sensor]: Error: Failed to get x axes");
    }
    else
    {
        Serial.printf("Axes: x - %d, y - %d, z - %d\n", axes[0], axes[1], axes[2]);
    }

    // getXSensitivity
    if(lsm6dsl->getXSensitivity(&data) != RetVal_OK)
    {
        Serial.println("[LSM6DSL Sensor]: Error: Failed to get x sensitivity");
    }
    else
    {
        Serial.print("Sensitivity: ");
        Serial.println(data);
    }
}

void gyroscope_test(){
    // enable Gyroscope
 //   if(lsm6dsl->enableGyroscope() != RetVal_OK)
    //{
     //   Serial.println("[LSM6DSL Sensor]: Error: Failed to call enable Gyroscope");
     //   return;
    //}
    
    // getGAxes
    if(lsm6dsl->getGAxes(axes) != RetVal_OK)
    {
        Serial.println("[LSM6DSL Sensor]: Error: Failed to get g axes");
    }
    else
    {
        Serial.printf("Axes: x - %d, y - %d, z - %d\n", axes[0], axes[1], axes[2]);
    }

    // getGSensitivity
    if(lsm6dsl->getGSensitivity(&data) != RetVal_OK)
    {
        Serial.println("[LSM6DSL Sensor]: Error: Failed to get g sensitivity");
    }
    else
    {
        Serial.print("Sensitivity: ");
        Serial.println(data);
    }
}

void check_sensorrgbled(){
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

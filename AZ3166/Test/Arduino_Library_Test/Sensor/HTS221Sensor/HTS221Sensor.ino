#include "HTS221Sensor.h"

#define RetVal_OK           0
#define LOOP_DELAY          100

DevI2C *i2c;
HTS221Sensor *sensor;
float humidity = 0;
float temperature = 0;
uint8_t id;
int counter = 1;

void setup() {
  Serial.println(">> Start");
    Serial.println(__FILE__);
  
  i2c = new DevI2C(D14, D15);
  sensor = new HTS221Sensor(*i2c); 

  // init the sensor
  if(sensor -> init(NULL) != RetVal_OK)
  {
    Serial.println("[HTS221Sensor]: Error: Failed to init the sensor");
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
  // enable
  if(sensor -> enable() != RetVal_OK)
  {
    Serial.println("[HTS221Sensor]: Error: Failed to enable the sensor");
    return;
  }

  // read id
  if(sensor -> readId(&id) != RetVal_OK)
  {    
    Serial.println("[HTS221Sensor]: Error: Failed to read ID");    
  }  
  else
  {
    Serial.print("ID: ");
    Serial.println(id);
  }
  
  // get humidity
  if(sensor -> getHumidity(&humidity) != RetVal_OK)
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
  if(sensor -> getTemperature(&temperature) != RetVal_OK)
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
  if(sensor -> disable() != RetVal_OK)
  {
    Serial.println("[HTS221Sensor]: Error: Failed to disable the sensor");
    return;
  }

  // reset
  if(sensor -> reset() != RetVal_OK)
  {
    Serial.println("[HTS221Sensor]: Error: Failed to reset the sensor");
    return;
  }
  
  delay(LOOP_DELAY);
}


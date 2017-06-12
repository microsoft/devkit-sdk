#include "lis2mdl_class.h"

#define RetVal_OK           0
#define LOOP_DELAY          100

DevI2C *i2c; 
LIS2MDL *lis2mdl;
int axes[3];
int16_t raw[3];
uint8_t id;
int counter  = 1;

void setup(){
    Serial.println(">> Start");
    Serial.println(__FILE__);

    i2c = new DevI2C(D14, D15);
    lis2mdl = new LIS2MDL(*i2c);

    // init
    if(lis2mdl->init(NULL) != RetVal_OK)
    {
        Serial.println("[LIS2MDL]: Error: Failed to init LIS2MDL");
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


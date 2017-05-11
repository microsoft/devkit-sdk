#include "lps25h_class.h"

#define RetVal_OK 0

DevI2C *i2c; 
LPS25H *lps25h;
float data;
PRESSURE_InitTypeDef *initType;

void setup(){
    i2c = new DevI2C(D14, D15);
    lps25h = new LPS25H(*i2c);

    initType->OutputDataRate =1;
    initType->PressureResolution = 10;
    initType->TemperatureResolution = 10;
    initType->DiffEnable = 1;
    initType->BlockDataUpdate = 1;
    initType->SPIMode = 1;

    // init
    if(lps25h->init(initType) != RetVal_OK)
    {
        Serial.println("[LPS25H]: Error: Failed to init LPS25H");
        return;
    }
}

void loop(){
    Serial.println("[LPS25H]: Test LPS25H library");
    
    if(lps25h->reset() != RetVal_OK)
    {
      Serial.println("[LPS25H]: Error: Failed to reset LPS25H");
      return;
    }
    
    // get pressure
    if(lps25h->getPressure(&data) != RetVal_OK)
    {
        Serial.println("[LPS25H]: Error: Failed to get the pressure");
    }
    else
    {
        Serial.print("Pressure: ");
        Serial.println(data);
    }

    // get temperature
    if(lps25h->getTemperature(&data) != RetVal_OK)
    {
        Serial.println("[LPS25H]: Error: Failed to get the temperature");
    }
    else
    {
        Serial.print("Temperature: ");
        Serial.println(data);
    }

    Serial.println("[LPS25H]: Done");
    delay(2000);
}

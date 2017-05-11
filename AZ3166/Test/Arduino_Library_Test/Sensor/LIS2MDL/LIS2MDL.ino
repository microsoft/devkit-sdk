#include "lis2mdl_class.h"
#define RetVal_OK 0

DevI2C *i2c; 
LIS2MDL *lis2mdl;
int32_t axes[3];
int16_t raw[3];
uint8_t id;

void setup(){
    i2c = new DevI2C(D14, D15);
    lis2mdl = new LIS2MDL(*i2c);

    // init
    if(lis2mdl->init(NULL) != RetVal_OK)
    {
        Serial.println("[LIS2MDL]: Error: Failed to init LIS2MDL");
        return;
    }
}

void loop(){
    Serial.println("[LIS2MDL]: Test LIS2MDL library");

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

    // get_m_axes_raw
    if(lis2mdl->get_m_axes_raw(raw) != RetVal_OK)
    {
        Serial.println("[LIS2MDL]: Error: Failed to get m axes raw");
    }
    else
    {
        Serial.printf("Raw: x - %d, y - %d, z - %d\n", raw[0], raw[1], raw[2]);
    }

    Serial.println("[LIS2MDL]: Done");
    delay(2000);
}

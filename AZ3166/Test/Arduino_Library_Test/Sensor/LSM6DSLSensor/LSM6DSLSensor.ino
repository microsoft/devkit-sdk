#include "LSM6DSLSensor.h"

#define RetVal_OK 0

DevI2C *i2c; 
LSM6DSLSensor *sensor;
int32_t axes[3];
int16_t raws[3];
float data;

void setup(){
    i2c = new DevI2C(D14, D15);
    sensor = new LSM6DSLSensor(*i2c, D4, D5);
    
    // init
    if(sensor->init(NULL) != RetVal_OK)
    {
        Serial.println("[LSM6DSL Sensor]: Error: Failed to init the LSM6DSL sensor");
        return;
    }
}

void loop(){
    Serial.println("[LSM6DSL Sensor]: Test LSM6DSL library");

    // Accelerometer test
    accelerometer_test();
    
    // Gyroscope test
    gyroscope_test();

    Serial.println("[LSM6DSL Sensor]: Done");
    delay(1000);
}

void accelerometer_test(){
    // enable_x
    if(sensor->enable_x() !=  RetVal_OK)
    {
      Serial.println("[LSM6DSL Sensor]: Error: Failed to call enable_x()");
      return;
    }
    
    // get_x_axes
    if(sensor->get_x_axes(axes) != RetVal_OK)
    {
        Serial.println("[LSM6DSL Sensor]: Error: Failed to get x axes");
    }
    else
    {
        Serial.printf("Accelerator: x: %d, y: %d, z: %d\n", axes[0], axes[1], axes[2]);
    }

    // get_x_sensitivity
    if(sensor->get_x_sensitivity(&data) != RetVal_OK)
    {
        Serial.println("[LSM6DSL Sensor]: Error: Failed to get x sensitivity");
    }
    else
    {
        Serial.print("Sensitivity: ");
        Serial.println(data);
    }
     
    // get_x_axes_raw
    if(sensor->get_x_axes_raw(raws) != RetVal_OK)
    {
        Serial.println("[LSM6DSL Sensor]: Error: Failed to get x axes raw");
    }
    else
    {
        Serial.printf("Raw: x: %d, y: %d, z: %d\n", raws[0], raws[1], raws[2]);
    }    
}

void gyroscope_test(){
    // enable_g
    if(sensor->enable_g() != RetVal_OK)
    {
        Serial.println("[LSM6DSL Sensor]: Error: Failed to call enable_g()");
        return;
    }
    
    // get_g_axes
    if(sensor->get_g_axes(axes) != RetVal_OK)
    {
        Serial.println("[LSM6DSL Sensor]: Error: Failed to get g axes");
    }
    else
    {
        Serial.printf("Gyroscope: x: %d, y: %d, z: %d\n", axes[0], axes[1], axes[2]);
    }

    // get_g_sensitivity
    if(sensor->get_g_sensitivity(&data) != RetVal_OK)
    {
        Serial.println("[LSM6DSL Sensor]: Error: Failed to get g sensitivity");
    }
    else
    {
        Serial.print("Sensitivity: ");
        Serial.println(data);
    }
    
    // get_g_axes_raw
    if(sensor->get_g_axes_raw(raws) != RetVal_OK)
    {
        Serial.println("[LSM6DSL Sensor]: Error: Failed to get g axes raw");
    }
    else
    {
        Serial.printf("Raw: x: %d, y: %d, z: %d\n", raws[0], raws[1], raws[2]);
    }    
}

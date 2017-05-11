#include "RGB_LED.h"

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
    
void setup(){

}

void loop(){
    Serial.println("[RGB_LED]: Test RGB_LED Library");

    for(int i = 0; i< 8; ++i)
    {
      Serial.printf("Red: %d, Green: %d, Blue: %d\n", color[i][0], color[i][1], color[i][2]);      
      rgbLed.setColor(color[i][0], color[i][1], color[i][2]);
      delay(1000);
    }
    
    Serial.println("Turn off");
    rgbLed.turnOff();
    delay(1000);

    Serial.println("[RGB_LED]: Done");
}

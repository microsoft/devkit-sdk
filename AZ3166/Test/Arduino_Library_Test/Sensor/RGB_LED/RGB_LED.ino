#include "RGB_LED.h"

#define LOOP_DELAY          100

RGB_LED rgbLed;
int counter = 1;
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
  Serial.println(">> Start");
  Serial.println(__FILE__);
}

void loop() {
  while(counter <= 5)
  {
    Serial.printf(">> Start (%d)\r\n", counter);
    runCase();
    Serial.printf(">> End (%d)\r\n", counter); 

    if(counter == 5)
    {
      Serial.println();
      Serial.println(">> End");
    }
    
    counter++;
  }
}

void runCase()
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


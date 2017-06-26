#include <ArduinoUnit.h>
#define LOOP_DELAY          500

void setup(){
    Serial.println(__FILE__);
    
    Screen.init(); 
}

void loop() {
  Test::run();
}

test(oled)
{
  Serial.println("Print a string with wrapped = false");
  Screen.print("This is OLEDDisplay Testing", false);    
  delay(LOOP_DELAY);

  Serial.println("Print a string with wrapped = true");
  Screen.print("long string; \nlong string;\nlong string;\nlong string;", true);    
  delay(LOOP_DELAY);

  Serial.println("Print a string with specified line Number");
  for(int i =0; i<=3; i++)
  {
    char buf[100];
    sprintf(buf, "This is row %d", i);
    Screen.print(i, buf);
  } 
  delay(LOOP_DELAY);

  Serial.println("Clean up");
  Screen.clean();

  delay(LOOP_DELAY);      
}
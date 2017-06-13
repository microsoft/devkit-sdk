#define LOOP_DELAY          500

int counter = 1;

void setup(){
    Serial.println(">> Start");
    Serial.println(__FILE__);
    
    Screen.init(); 
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
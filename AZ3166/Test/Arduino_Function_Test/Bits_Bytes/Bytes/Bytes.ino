#define LOOP_DELAY          500

int counter = 1;

void setup(){
  Serial.println(">> Start");
  Serial.println(__FILE__);
}

typedef struct BytesData{
    unsigned int test_data;
    byte lb;
    byte hb;
  }BytesData;

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
    BytesData bytes[3]= {
      {1022, 254, 3}, 
      {1, 1, 0},  
      {2048,0,8}
    };
    byte lb, hb;

    for(int i=0; i<3; ++i)
    {
      //LowByte returns the low Byte(that is righmost) of a variable(e.g.a word)
      lb= lowByte(bytes[i].test_data);
      if(lb != bytes[i].lb)
      {
        Serial.printf("Error: lowByte(): For %d, the low byte should be %d", bytes[i].test_data, bytes[i].lb);
        Serial.println();
      }

      //HighByte returns the higher Byte(that is leftmost) of a variable to 2byte(16bit) or the second lowest byte of a larger data type.
      hb= highByte(bytes[i].test_data);
      if(hb != bytes[i].hb)
      {
        Serial.printf("Error: highByte(): For %d, the high byte should be %d", bytes[i].test_data, bytes[i].hb);
        Serial.println();
      }
    }

    delay(LOOP_DELAY);
}

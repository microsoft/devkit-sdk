void setup(){
  Serial.begin(115200);
}

typedef struct BytesData{
    unsigned int test_data;
    byte lb;
    byte hb;
  }BytesData;

void loop(){
    Serial.println("[Bytes]: Test Bytes related functions");
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
        Serial.printf("[Bytes]: Error: lowByte(): For %d, the low byte should be %d", bytes[i].test_data, bytes[i].lb);
        Serial.println();
      }

      //HighByte returns the higher Byte(that is leftmost) of a variable to 2byte(16bit) or the second lowest byte of a larger data type.
      hb= highByte(bytes[i].test_data);
      if(hb != bytes[i].hb)
      {
        Serial.printf("[Bytes]: Error: highByte(): For %d, the high byte should be %d", bytes[i].test_data, bytes[i].hb);
        Serial.println();
      }
    }

    Serial.println("[Bytes]: Done");
    delay(1000);
}

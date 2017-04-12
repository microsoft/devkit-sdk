void setup(){

}

void loop(){
    Serial.println("[Bytes]: Test Bytes related functions");
    unsigned int x =1022;
    
    //LowByte returns the low Byte(that is righmost) of a variable(e.g.a word)
    byte lb= lowByte(x);
    if(lb != 254)
    {
      Serial.println("[Bytes]: Error: lowByte(): For 1022, the low byte should be 11111110 (254)");
    }

    //HighByte returns the higher Byte(that is leftmost) of a variable to 2byte(16bit) or the second lowest byte of a larger data type.
    byte hb= highByte(x);
    if(hb != 3)
    {
      Serial.println("[Bytes]: Error: highByte(): For 1022, the high byte should be 11 (3)");
    }

    Serial.println("[Bytes]: Done");
    delay(1000);
}

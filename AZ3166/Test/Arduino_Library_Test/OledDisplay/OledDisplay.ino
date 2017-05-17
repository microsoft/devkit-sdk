void setup(){
    Screen.init();   
}

void loop(){
    Serial.println("[OLEDDisplay]: Test OLEDDisplay library");

    Screen.print("This is OLEDDisplay Testing", false);    
    delay(1000);

    Screen.print("long string; \nlong string;\nlong string;\nlong string;", true);    
    delay(1000);

    for(int i =0; i<=3; i++)
    {
      char buf[100];
      sprintf(buf, "This is row %d", i);
      Screen.print(i, buf);
    } 
    delay(1000);
    
    Screen.clean();

    Serial.println("[OLEDDisplay]: Done");
    delay(1000);
}
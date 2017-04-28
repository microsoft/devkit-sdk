#include "AZ3166WiFi.h"

void setup() {
  // put your setup code here, to run once:
  byte mac[6];
  Serial.println( "start soft ap!" );

  char ap_name[24];
  WiFi.macAddress(mac);
  sprintf(ap_name, "AZ3166_%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  int ret = WiFi.beginAP(ap_name, "12345678");
  if ( ret != WL_CONNECTED) {
    Serial.println("Soft ap creation failed");
    return ;
  }
  Serial.println("soft ap started\r\n");
  httpd_server_start();

  // you can connect to the board and set wifi at "http://192.168.0.1:999/" now
}

void loop() {
  // put your main code here, to run repeatedly:

}

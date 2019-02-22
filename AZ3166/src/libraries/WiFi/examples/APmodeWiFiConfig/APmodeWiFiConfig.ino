#include "AZ3166WiFi.h"

char ssid[] = "yournetworkname";      //  your network SSID (name)
char pass[] = "yourpassword";   // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

void setup() {
  // put your setup code here, to run once:
  byte mac[6];
  Serial.println( "start soft ap!" );

  char ap_name[24];
  WiFi.macAddress(mac);
  sprintf(ap_name, "AZ3166_%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  int ret = WiFi.beginAP(ap_name, "");
  if ( ret != WL_CONNECTED) {
    Serial.println("Soft ap creation failed");
    return ;
  }
  Serial.println("soft ap started\r\n");

  // Start Station mode at the same time
  const char* fv = WiFi.firmwareVersion();
  Serial.printf("Wi-Fi firmware: %s\r\n", fv);
  
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  Serial.println("Connected to wifi");
  printCurrentNet();

}

void loop() {
  // check the network connection once every 10 seconds:
  delay(10000);
  printCurrentNet();

}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  Serial.print(bssid[5], BASE_HEX);
  Serial.print(":");
  Serial.print(bssid[4], BASE_HEX);
  Serial.print(":");
  Serial.print(bssid[3], BASE_HEX);
  Serial.print(":");
  Serial.print(bssid[2], BASE_HEX);
  Serial.print(":");
  Serial.print(bssid[1], BASE_HEX);
  Serial.print(":");
  Serial.println(bssid[0], BASE_HEX);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  int encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, BASE_HEX);
  Serial.println();
}

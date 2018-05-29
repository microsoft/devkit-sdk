#include "AZ3166WiFi.h"
#include "OTAClass.h"

static bool isConnected = false;
char* OTA_Url = "https://azureboard2.azureedge.net/prod/devkit-firmware-latest.ota.bin";

void InitWiFi()
{
  Screen.print("WiFi \r\n \r\nConnecting...\r\n             \r\n");

  if(WiFi.begin() == WL_CONNECTED)
  {
    isConnected = true;
  }
}

void setup() {
  isConnected = false;
  InitWiFi();
  OTAClass& ota = OTAClass::getInstance();
  int result = ota.OTAFromUrl(OTA_Url);
  if (result == 0) {
    Screen.print("OTA success\nReset to start\nnew firmware\n");
  }
  else {
    Screen.print("OTA failed\n");
  }
}


void loop() {

}

#include "AZ3166WiFi.h"
#include "OTAUpdateClient.h"

static bool isConnected = false;
char* Firmware_Url = "https://azureboard2.azureedge.net/prod/devkit-firmware-latest.ota.bin";

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
  if (isConnected == false)
  {
    return;
  }

  OTAUpdateClient& ota = OTAUpdateClient::getInstance();
  int result = ota.updateFromUrl(Firmware_Url);
  if (result == 0) {
    Screen.print("Update success\nReset to start\nnew firmware\n");
  }
  else {
    Screen.print("Update failed\n");
  }
}


void loop() {

}

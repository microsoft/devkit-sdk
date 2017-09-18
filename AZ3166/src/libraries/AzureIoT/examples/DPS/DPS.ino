#include "DiceInit.h"
#include "AZ3166WiFi.h"
#include "OledDisplay.h"

static int status;

// Indicate whether WiFi is ready
static bool hasWifi = false;

static void InitWiFi()
{
  Screen.clean();
  Screen.print(2, "Connecting...");
  
  if (WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    Screen.print(1, ip.get_address());
    hasWifi = true;
    Screen.print(2, "Running... \r\n");
  }
  else
  {
    hasWifi = false;
    Screen.print(1, "No Wi-Fi\r\n ");
  }
}

void setup() {
  // put your setup code here, to run once:

  Screen.init();
  Screen.print(2, "Initializing...");

  Screen.print(3, " > Serial");
  Serial.begin(115200);
  
  // Initialize the WiFi module
  Screen.print(3, " > WiFi");
  hasWifi = false;
  InitWiFi();
  if (!hasWifi)
  {
    return;
  }

  delay(100);
  Serial.println("Start to run Dice+RIoT application.\r\n");
  delay(100);
  status = StartDiceInit();
  delay(10);
  if (status == 0){
    Serial.println("Finish Dice+RIoT+DPS application successfully.\r\n");
    Screen.print(2, "DPS connected!\r\n");
  }
  else{
    Screen.print(2, "DPS Failed!\r\n");
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}

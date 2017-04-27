#include "http_client.h"
#include "OLEDDisplay.h"
#include "RGB_LED.h"
#include "AZ3166WiFi.h"

#define RGB_LED_BRIGHTNESS  16
#define LOOP_DELAY          1000

static boolean hasWifi;

static RGB_LED rgbLed;

static void InitWiFi()
{
  if (WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    Screen.print(1, ip.get_address());
    hasWifi = true;
  }
  else
  {
    Screen.print(1, "No Wi-Fi           ");
  }
}

static void InitBoard(void)
{
  Screen.clean();
  Screen.print(0, "Azure IoT DevKit     ");
  Screen.print(2, "Initializing...      ");
  
  Screen.print(3, " > Serial            ");
  Serial.begin(115200);
  
  // Initialize the WiFi module
  Screen.print(3, " > WiFi              ");
  hasWifi = false;
  InitWiFi();
  
  // Initialize LEDs
  Screen.print(3, " > LEDs              ");
  rgbLed.turnOff();
  
  // Initialize button
  Screen.print(3, " > Button            ");
  pinMode(USER_BUTTON_A, INPUT);
  pinMode(USER_BUTTON_B, INPUT);
}

void setup()
{
  InitBoard();
  if (!hasWifi)
  {
    Screen.print(3, " > Fault         ");
    return;
  }
}

void dump_response(const Http_Response* res)
{
    Serial.printf("Status: %d - %s\r\n", res->status_code, res->status_message);
    Serial.printf("Body: ");
    if (res->body != NULL)
    {
      Serial.println(res->body);
    }
    else
    {
        Serial.println("<empty>");
    }
}

void loop()
{
  if (hasWifi)
  {
    rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
    HTTPClient httpClient(HTTP_GET, "https://developer.mbed.org/media/uploads/mbed_official/hello.txt");
    const Http_Response* result = httpClient.send();
    if (result == NULL) 
    {
      Serial.printf("HttpRequest failed (error code %d)\n", httpClient.get_error());
    }
    else
    {
      Serial.printf("\n----- HTTPS GET response -----\n");
      dump_response(result);
    }
  }
  delay(LOOP_DELAY);
}

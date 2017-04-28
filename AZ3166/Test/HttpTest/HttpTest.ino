#include "http_client.h"
#include "OLEDDisplay.h"
#include "RGB_LED.h"
#include "AZ3166WiFi.h"
#include "mbed_memory_status.h"

#define RGB_LED_BRIGHTNESS  16
#define LOOP_DELAY          5000

static boolean hasWifi;

static RGB_LED rgbLed;

const char SSL_CA_PEM[] = "-----BEGIN CERTIFICATE-----\n"
    "MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n"
    "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"
    "DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n"
    "SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n"
    "GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n"
    "AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n"
    "q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n"
    "SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n"
    "Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n"
    "a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n"
    "/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n"
    "AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n"
    "CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n"
    "bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n"
    "c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n"
    "VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n"
    "ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n"
    "MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n"
    "Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n"
    "AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n"
    "uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n"
    "wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n"
    "X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n"
    "PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n"
    "KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n"
    "-----END CERTIFICATE-----\n";
    
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
  Screen.print(2, "Running...           ");
  Screen.print(3, "                     ");
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

extern void telemetry_enqueue(const char *iothub, const char *event, const char *message);

void loop()
{
  if (hasWifi)
  {
    Screen.print(3, "  > Test HTTP requst ");
    HTTPClient *httpClient = new HTTPClient(SSL_CA_PEM, HTTP_GET, "https://httpbin.org/status/418");
    rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
    const Http_Response* result = httpClient->send();
    if (result == NULL) 
    {
      Serial.printf("HttpRequest failed (error code %d)\r\n", httpClient->get_error());
    }
    else
    {
      Serial.printf("\r\n----- HTTPS GET response -----\r\n");
      dump_response(result);
    }
    delete httpClient;
    rgbLed.setColor(0, 0, RGB_LED_BRIGHTNESS);
    Screen.print(3, "  > Done             ");
  }
  delay(LOOP_DELAY);
  print_heap_info();
}

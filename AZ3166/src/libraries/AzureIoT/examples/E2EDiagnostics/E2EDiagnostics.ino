#include "AzureIotHub.h"
#include "AZ3166WiFi.h"
#include "config.h"
#include "iothub_client_sample_mqtt.h"

static bool hasWifi = false;
static int messageCount = 1;

void initWifi()
{
    Screen.print("IoT DevKit\r\n \r\nConnecting...\r\n");

    if (WiFi.begin() == WL_CONNECTED)
    {
        IPAddress ip = WiFi.localIP();
        Screen.print(1, ip.get_address());
        hasWifi = true;
        Screen.print(2, "Running... \r\n");
    }
    else
    {
        Screen.print(1, "No Wi-Fi\r\n ");
    }
}

void setup()
{
    hasWifi = false;
    initWifi();
    if (!hasWifi)
    {
        LogInfo("Please make sure the wifi connected!");
        return;
    }

    Serial.begin(9600);
    sensorInit();
    iothubInit();
}

void loop()
{
    char messagePayload[MESSAGE_MAX_LEN];
    readMessage(messageCount, messagePayload);
    Serial.println(messagePayload);
    iothubSendMessage((const unsigned char *)messagePayload);
    messageCount++;
    iothubLoop();
    delay(10);
}

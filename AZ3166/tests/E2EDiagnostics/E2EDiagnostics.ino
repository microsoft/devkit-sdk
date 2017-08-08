#include "AzureIotHub.h"
#include "AZ3166WiFi.h"
#include "config.h"
#include "iothub_client_sample_mqtt.h"
#include "Telemetry.h"

static bool hasWifi = false;
static bool ready = false;

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
    Serial.begin(115200);
    hasWifi = false;
    initWifi();
    if (!hasWifi)
    {
        LogInfo("Please make sure the wifi connected!");
        return;
    }

    // Microsoft collects data to operate effectively and provide you the best experiences with our products. 
    // We collect data about the features you use, how often you use them, and how you use them.
    send_telemetry_data_async("", "E2EDiagnosticsSampleSetup", "");

    sensorInit();
    ready = iothubInit();
}

void loop()
{
    if(!ready)
    {
        delay(1000);
        return;
    }

    iothubSendMessage();
    iothubLoop();
    delay(1000);
}

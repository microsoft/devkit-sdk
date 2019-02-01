// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#include "Arduino.h"
#include "console_cli.h"
#include "EMW10xxInterface.h"
#include "mbed.h"
#include "mbed_stats.h"
#include "mico_system.h"
#include "SystemLock.h"
#include "SystemTickCounter.h"
#include "SystemWiFi.h"

static bool Initialization(void)
{
    mbed_stats_heap_t heap_stats;
    mbed_stats_heap_get(&heap_stats);

    init_system_lock();

#if defined(USBCON)
    USBDevice.attach();
#endif

    Serial.print("\r\n************************************************");
    Serial.print("\r\n** MXChip - Microsoft IoT Developer Kit **");
    Serial.print("\r\n************************************************\r\n");

    // Initialize the system tickcounter
    SystemTickCounterInit();

    // Initialize the OLED screen
    Screen.init();

    // Turn off WiFi led
    DigitalOut LedWifi(LED_WIFI);
    LedWifi = 0;

    // Turn off Azure led
    DigitalOut LedAzure(LED_AZURE);
    LedAzure = 0;

    // Turn off User led
    DigitalOut LedUser(LED_USER);
    LedUser = 0;

    // Turn off RGB led
    PwmOut _red(PB_4);
    PwmOut _green(PB_3);
    PwmOut _blue(PC_7);
    _red.period(0.001);
    _green.period(0.001);
    _blue.period(0.001);
    _red.write(0.0f);
    _green.write(0.0f);
    _blue.write(0.0f);

    return true;
}

static bool IsConfigurationMode()
{
    pinMode(USER_BUTTON_A, INPUT);
    int buttonState = digitalRead(USER_BUTTON_A);
    if (buttonState == LOW)
    {
        return true;
    }
    return false;
}

static bool IsAPMode()
{
    pinMode(USER_BUTTON_B, INPUT);
    int buttonState = digitalRead(USER_BUTTON_B);
    if (buttonState == LOW)
    {
        return true;
    }
    return false;
}

static void EnterConfigurationMode()
{
    pinMode(USER_BUTTON_A, INPUT);

    Screen.print("IoT DevKit\r\n \r\nConfiguration\r\n");

    if (!InitSystemWiFi())
    {
        return;
    }

    char id[24] = "id:";
    id[3 + GetMACWithoutColon(id + 3)] = 0;
    Screen.print(1, id);

    // Enter configuration mode
    cli_main();
}

static void EnterAPMode()
{
    pinMode(USER_BUTTON_B, INPUT);

    Screen.print("IoT DevKit");

    if (!InitSystemWiFi())
    {
        Serial.println("Set wifi AP Mode failed");
        return;
    }

    const char *ap_name = GetBoardAPName();

    int ret = SystemWiFiAPStart(ap_name, "");
    if (ret == false)
    {
        Serial.println("Soft ap creation failed");
        return;
    }

    httpd_server_start();

    Screen.print(1, ap_name);
    Screen.print(2, "Config WiFi on");
    Screen.print(3, "192.168.0.1");

    Serial.printf("Soft AP %s is running...\r\n", ap_name);
    Serial.printf("Connect and visit \"http://192.168.0.1/\" to config the Wi-Fi settings.\r\n");
}

extern void start_arduino(void);

static void EnterUserMode()
{
    Serial.print("You can 1. press Button A and reset to enter configuration mode.\r\n        2. press Button B and reset to enter AP mode.\r\n\r\n");

    start_arduino();

    for (;;)
    {
        // loop
        wait_ms(60000);
    }
}

int main(void)
{
    Initialization();

    if (IsConfigurationMode())
    {
        EnterConfigurationMode();
    }
    else if (IsAPMode())
    {
        EnterAPMode();
    }
    else
    {
        EnterUserMode();
    }

    return 0;
}
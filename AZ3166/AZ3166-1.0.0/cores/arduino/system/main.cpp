/**
 ******************************************************************************
 * The MIT License (MIT)
 * Copyright (C) 2017 Microsoft Corp. 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "Arduino.h"
#include "mico_system.h"
#include "console_cli.h"
#include "SystemWiFi.h"
#include "mbed_stats.h"
#include "SystemLock.h"
#include "EMW10xxInterface.h"

static bool Initialization(void)
{
    mbed_stats_heap_t heap_stats;
    mbed_stats_heap_get(&heap_stats);

    init_system_lock();

#if defined(USBCON)
    USBDevice.attach();
#endif
    
    Serial.print("\r\n************************************************");
    Serial.print("\r\n** MXChip - Microsoft Azure IoT Developer Kit **");
    Serial.print("\r\n************************************************\r\n");
    
    // Initialize the OLED screen
    Screen.init();

    return true;
}

static bool IsConfigurationMode()
{
    pinMode(USER_BUTTON_A, INPUT);
    int buttonState = digitalRead(USER_BUTTON_A);
    if(buttonState == LOW)
    {
        return true;
    }
    return false;
}

static bool IsAPMode()
{
    pinMode(USER_BUTTON_B, INPUT);
    int buttonState = digitalRead(USER_BUTTON_B);
    if(buttonState == LOW)
    {
        return true;
    }
    return false;
}

static int GetMACWithoutColon(char* buff)
{
    const char* mac = WiFiInterface()->get_mac_address();
    int j = 0;
    for(int i =0; i < strlen(mac); i++)
    {
        if (mac[i] != ':')
        {
            buff[j++] = mac[i];
        }
    }

    return j;
}

static void EnterConfigurationMode()
{
    pinMode(USER_BUTTON_A, INPUT);

    Screen.print("Azure IoT DevKit\r\n \r\nConfiguration\r\n");

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

    Screen.print("Azure IoT DevKit");
   
    if (!InitSystemWiFi())
    {
        Serial.println("Set wifi AP Mode failed");
        return;
    }

    char ap_name[24] = "AZ-";
    ap_name[3 + GetMACWithoutColon(ap_name + 3)] = 0;

    int ret = SystemWiFiAPStart(ap_name, "");
    if ( ret == false) 
    {
        Serial.println("Soft ap creation failed");
        return ;
    }

    httpd_server_start();
    
    Screen.print(1, ap_name);
    Screen.print(2, "Soft AP started");
    Screen.print(3, " > 192.168.0.1");

    Serial.printf("Soft AP %s is running...\r\n", ap_name);
    Serial.printf("Connect and visit \"http://192.168.0.1/\" to config the Wi-Fi settings.\r\n");
}

extern void start_arduino ( void );

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

int main( void )
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
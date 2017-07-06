// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "Arduino.h"
#include "SystemTime.h"
#include "SystemWiFi.h"
#include "NTPClient.h"

static const char* ntpHost = "0.pool.ntp.org";
static bool timeSynced = false;

static NTPResult NTPSyncUP(void)
{
    NTPClient ntp(WiFiInterface());
    return ntp.setTime(ntpHost);
}

void SyncTime(void)
{
    if (NTPSyncUP() == NTP_OK)
    {
        time_t t = time(NULL);
        if (!timeSynced)
        {
            Serial.printf("Time is now (UTC): %s\r\n", ctime(&t));
            timeSynced = true;
        }
    }
    else
    {
        Serial.printf("Unable to get the NTP host %s\r\n", ntpHost);
    }
}

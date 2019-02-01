// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "Arduino.h"
#include "SystemTime.h"
#include "SystemWiFi.h"
#include "NTPClient.h"

static const char* ntpHost[] = 
{
    "pool.ntp.org",
    "cn.pool.ntp.org",
    "europe.pool.ntp.org",
    "asia.pool.ntp.org",
    "oceania.pool.ntp.org"
};

static bool timeSynced = false;

static NTPResult NTPSyncUP(char* host)
{
    NTPClient ntp(WiFiInterface());
    return ntp.setTime(host);
}

void SyncTime(void)
{
    for (size_t i = 0; i < sizeof(ntpHost) / sizeof(ntpHost[0]); i++)
    {
        if (NTPSyncUP((char*)ntpHost[i]) == NTP_OK)
        {
            if (!timeSynced)
            {
                time_t t = time(NULL);
                Serial.printf("Time from %s, now is (UTC): %s\r\n", ntpHost[i], ctime(&t));
                timeSynced = true;
            }
            break;
        }
        else
        {
            Serial.printf("Unable to get the NTP host %s\r\n", ntpHost[i]);
        }
    }
}

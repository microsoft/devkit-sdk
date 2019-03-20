// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "Arduino.h"
#include "SystemTime.h"
#include "SystemWiFi.h"
#include "NTPClient.h"

static const char *defaultNTPs = "pool.ntp.org;cn.pool.ntp.org;europe.pool.ntp.org;asia.pool.ntp.org;oceania.pool.ntp.org";

static char **ntpHosts = NULL;
static int ntpHostCount = 0;

static bool timeSynced = false;

char **splitString(const char * tsList)
{
    int num = 1;
    for (char* c = (char*)tsList; *c != 0; c++)
    {
        if (*c == ';')
        {
            num++;
        }
    }

    char **list = (char**)calloc(num, sizeof(char*));
    if (list == NULL)
    {
        return NULL;
    }
    list[0] = strdup(tsList);
    int i = 1;
    for (char* c = list[0]; *c != 0; c++)
    {
        if (*c == ';')
        {
            *c = 0;
            list[i] = c + 1;
            i++;
        }
    }
        
    ntpHostCount = num;
    ntpHosts = list;
    return ntpHosts;
}

int SetTimeServer(const char * tsList)
{
    if (tsList == NULL || tsList[0] == 0)
    {
        return -1;
    }
    
    if (ntpHosts != NULL)
    {
        free(ntpHosts[0]);
        free(ntpHosts);
        ntpHosts = NULL;
        ntpHostCount = 0;
    }
    splitString(tsList);

    return 0;
}

static NTPResult NTPSyncUP(char* host)
{
    NTPClient ntp(WiFiInterface());
    return ntp.setTime(host);
}

void SyncTime(void)
{
    if (ntpHosts == NULL)
    {
        splitString(defaultNTPs);
    }
    
    timeSynced = false;
    for (int i = 0; i < ntpHostCount; i++)
    {
        if (NTPSyncUP(ntpHosts[i]) == NTP_OK)
        {
            timeSynced = true;
            break;
        }
    }
}

int IsTimeSynced(void)
{
    return (timeSynced ? 0 : -1);
}
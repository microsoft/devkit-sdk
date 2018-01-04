// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
#include "mbed.h"
#include "EMW10xxInterface.h"
#include "SystemVariables.h"
#include "SystemWiFi.h"

#ifdef __cplusplus
extern "C" {
#endif

static char boardID[24] = { "\0" };

int GetMACWithoutColon(char* buff)
{
    const char* mac = WiFiInterface()->get_mac_address();
    int j = 0;
    for(int i =0; i < strlen(mac); i++)
    {
        if (mac[i] != ':')
        {
            // Lower case
            buff[j++] = (mac[i] >= 'A' && mac[i] <= 'Z') ?  (mac[i] - 'A' + 'a') : mac[i];
        }
    }

    return j;
}

const char* GetBoardID(void)
{
    if (boardID[0] == 0)
    {
        boardID[0] = 'a';
        boardID[1] = 'z';
        boardID[2] = '-';
        boardID[3 + GetMACWithoutColon(boardID + 3)] = 0;
    }
    return boardID;
}

#ifdef __cplusplus
}
#endif
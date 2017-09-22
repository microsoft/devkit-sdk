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
            buff[j++] = mac[i];
        }
    }

    return j;
}

const char* GetBoardID(void)
{
    if (boardID[0] == 0)
    {
        boardID[0] = 'A';
        boardID[1] = 'Z';
        boardID[2] = '-';
        boardID[3 + GetMACWithoutColon(boardID + 3)] = 0;
    }
    return boardID;
}

#ifdef __cplusplus
}
#endif
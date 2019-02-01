// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
#include "mbed.h"
#include "EMW10xxInterface.h"
#include "SystemVariables.h"
#include "SystemWiFi.h"

#ifdef __cplusplus
extern "C" {
#endif

static char boardID[BOARD_ID_LENGTH + 1] = { "\0" };
static char boardAPName[BOARD_AP_LENGTH + 1] = { "\0" };

int GetMACWithoutColon(char* buff)
{
    const char* mac = WiFiInterface()->get_mac_address();
    int j = 0;
    for(size_t i =0; i < strlen(mac); i++)
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
        memcpy(boardID, boardIDHeader, strlen(boardIDHeader));
        boardID[strlen(boardIDHeader) + GetMACWithoutColon(boardID + strlen(boardIDHeader))] = 0;
    }
    return boardID;
}

const char* GetBoardAPName(void)
{
    if (boardAPName[0] == 0)
    {
        memcpy(boardAPName, boardAPHeader, strlen(boardAPHeader));
        boardAPName[strlen(boardAPHeader) + GetMACWithoutColon(boardAPName + strlen(boardAPHeader))] = 0;
    }
    return boardAPName;
}

#ifdef __cplusplus
}
#endif
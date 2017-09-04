// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
#include "Arduino.h"
#include "GetStartedUI.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Application title
static const unsigned char appIcon [] = 
{
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,
};

void DrawAppTitle(char* text)
{
    char sz[32];
    snprintf(sz, 32, "   %s", text);
    Screen.print(0, sz);
    Screen.draw(0, 0, 18, 2, (unsigned char*)appIcon);
}
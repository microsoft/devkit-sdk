// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
 
#include "Arduino.h"
#include "mico_system.h"
#include "oled.h"
#include "OledDisplay.h"

OLEDDisplay::OLEDDisplay()
{
}

OLEDDisplay::~OLEDDisplay()
{
}

void OLEDDisplay::init()
{
    OLED_Init();
    OLED_Clear();
}

void OLEDDisplay::clean()
{
    OLED_Clear();
}

int OLEDDisplay::print(const char *s, bool wrap)
{
    return print(0, s, wrap);
}

int OLEDDisplay::print(unsigned int line, const char *s, bool wrap)
{
    if (s == NULL)
    {
        return 0;
    }

    unsigned int ln = line;
    int start = 0;
    int offset = 0;
    
    while (true)
    {
        if (s[start + offset] == '\0')
        {
            ln = println(ln, &s[start], offset, wrap);
            break;
        }
        else if(s[start + offset] == '\r' || s[start + offset] == '\n')
        {
            if (offset > 0)
            {
                ln = println(ln, &s[start], offset, wrap);
                start += offset + 1;
                offset = 0;
            }
            else
            {
                // Skip
                start++;
            }
        }
        else
        {
            offset++;
        }
    }
    
    return ln;
}

void OLEDDisplay::draw(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[])
{
    OLED_DrawBMP(x0, y0, x1, y1, BMP);
}

int OLEDDisplay::println(unsigned int line, const char *s, int len, bool wrap)
{
    if (line > 3 || len == 0)
    {
        return line;
    }
    
    char oled_show_line[OLED_DISPLAY_MAX_CHAR_PER_ROW+1] = {'\0'};
    int lineNumber[4] = { OLED_DISPLAY_ROW_1, OLED_DISPLAY_ROW_2, OLED_DISPLAY_ROW_3, OLED_DISPLAY_ROW_4};
    
    int start = 0;
    int left = len;
    while (true)
    {
        if (line > 3)
        {
            break;
        }
        
        int i = 0;
        for (; i < min(left, OLED_DISPLAY_MAX_CHAR_PER_ROW); i++)
        {
            if (s[start + i] >= 0x20 && s[start + i] <= 0x7E)
            {
                oled_show_line[i] = s[start + i];
            }
            else
            {
                oled_show_line[i] = ' ';
            }
        }
        for (; i < OLED_DISPLAY_MAX_CHAR_PER_ROW; i++) {
            oled_show_line[i] = ' ';
        }
        oled_show_line[i] = 0;
        OLED_ShowString(OLED_DISPLAY_COLUMN_START, lineNumber[line ++], oled_show_line);
        
        if (wrap && left > OLED_DISPLAY_MAX_CHAR_PER_ROW)
        {
            start += OLED_DISPLAY_MAX_CHAR_PER_ROW;
            left -= OLED_DISPLAY_MAX_CHAR_PER_ROW;
        }
        else
        {
            break;
        }
    }
    
    return line;
}

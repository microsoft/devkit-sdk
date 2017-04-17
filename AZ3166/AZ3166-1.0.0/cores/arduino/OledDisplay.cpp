/* 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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

uint8_t OLEDDisplay::print(const char *s, bool wrap)
{
    return print(0, s, wrap);
}

uint8_t OLEDDisplay::print(uint8_t line, const char *s, bool wrap)
{
    if (s == NULL)
    {
        return 0;
    }
    
    uint8_t ln = line;
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

uint8_t OLEDDisplay::println(uint8_t line, const char *s, uint8_t len, bool wrap)
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

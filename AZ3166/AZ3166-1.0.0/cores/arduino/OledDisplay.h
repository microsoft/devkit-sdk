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

#ifndef __OLED_DISPLAY_H__
#define __OLED_DISPLAY_H__

#ifdef __cplusplus

class OLEDDisplay
{
public:
    OLEDDisplay();
    ~OLEDDisplay();
    
    void init();
    void clean();
    
    virtual int print(const char *s, bool wrap = false);
    virtual int print(unsigned int line, const char *s, bool wrap = false);

    /**
    * @brief show BMP image in OLED specified place
    *
    * @param x0: Position the X axis of the top left corner of the area image to display,
    *            valid value is [0, 127]
    * @param y0: Position the Y axis of the top left corner of the area image to display,
    *            valid value is [0, 7]
    * @param x1: Position the X axis of the bottom right corner of the area image to display,
    *            valid value is [1, 128]
    * @param y1: Position the Y axis of the bottom right corner of the area image to display,
    *            valid value is [1, 8]
    * @param BMP: BMP image pixel byte array. Every array element is an 8-bit binary data that
    *             draws 8-connected pixels in the same column
    * 
    * @return none
    */
    virtual int draw(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[]);
        
private:
    int println(unsigned int line, const char *s, int len, bool wrap);
};

#endif  // __cplusplus
#endif  // __OLED_DISPLAY_H__

// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

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
    virtual void draw(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[]);
        
private:
    int println(unsigned int line, const char *s, int len, bool wrap);
};

#endif  // __cplusplus
#endif  // __OLED_DISPLAY_H__

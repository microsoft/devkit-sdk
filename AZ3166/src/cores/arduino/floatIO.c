// Copyright (c) 2015 Volodymyr Shymanskyy. All rights reserved.
// Licensed under the MIT license.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "floatIO.h"
#define iSize 10                 // number of buffers, one for each float before wrapping around
 
/* float to string
 * f is the float to turn into a string
 * p is the precision (number of decimals)
 * return a string representation of the float.
 */
char *f2s(float f, int p){
    char * pBuff;                         // use to remember which part of the buffer to use for dtostrf
    static char sBuff[iSize][20];         // space for 20 characters including NULL terminator for each float
    static int iCount = 0;                // keep a tab of next place in sBuff to use
    pBuff = sBuff[iCount];                // use this buffer
    if(iCount >= iSize -1){               // check for wrap
        iCount = 0;                       // if wrapping start again and reset
    }
    else{
        iCount++;                         // advance the counter
    }
    return dtostrf(f, 0, p, pBuff);       // call the library function
}

/*
 * As there is a problem of sprintf %f in Arduino,
   follow https://github.com/blynkkk/blynk-library/issues/14 to implement dtostrf
 */
char * dtostrf(double number, signed char width, unsigned char prec, char *s) {
    if(isnan(number)) {
        strcpy(s, "nan");
        return s;
    }
    if(isinf(number)) {
        strcpy(s, "inf");
        return s;
    }

    if(number > 4294967040.0 || number < -4294967040.0) {
        strcpy(s, "ovf");
        return s;
    }
    char* out = s;
    // Handle negative numbers
    if(number < 0.0) {
        *out = '-';
        ++out;
        number = -number;
    }
    // Round correctly so that print(1.999, 2) prints as "2.00"
    double rounding = 0.5;
    for(unsigned char i = 0; i < prec; ++i) {
      rounding /= 10.0;
    }
    number += rounding;

    // Extract the integer part of the number and print it
    unsigned long int_part = (unsigned long) number;
    double remainder = number - (double) int_part;
    out += sprintf(out, "%lu", int_part);

    // Print the decimal point, but only if there are digits beyond
    if(prec > 0) {
        *out = '.';
        ++out;
    }

    while(prec-- > 0) {
        remainder *= 10.0;
        if((int)remainder == 0){
                *out = '0';
                 ++out;
        }
    }
    sprintf(out, "%d", (int) remainder);
    return s;
}
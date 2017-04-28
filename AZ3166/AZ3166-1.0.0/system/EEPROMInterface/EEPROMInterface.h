/**
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2017 Microsoft Corp.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ******************************************************************************
 */

#ifndef _EEPROM_INTERFACE_
#define _EEPROM_INTERFACE_

#include "mbed.h"

#define WIFI_SSID_ZONE_IDX      0x03
#define WIFI_PWD_ZONE_IDX       0X0A
#define AZ_IOT_HUB_ZONE_IDX     0X05

#define WIFI_SSID_MAX_LEN       32
#define WIFI_PWD_MAX_LEN        64
#define AZ_IOT_HUB_MAX_LEN      200
#define EEPROM_DEFAULT_LEN		200

/**
 * \brief Write/Read data to/from EEPROM of STSAFE_A100 through I2C interface.
 * 
 */
class EEPROMInterface
{
public:
	EEPROMInterface();
	~EEPROMInterface();
    
	int write(uint8_t* dataBuff, int buffSize, uint8_t dataZoneIndex);
	int read(uint8_t* dataBuff, int buffSize, uint16_t offset, uint8_t dataZoneIndex);
};

#endif /* _EEPROM_INTERFACE_ */

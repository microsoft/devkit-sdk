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

#include "EEPROMInterface.h"
#include "mbed.h"
#include "STSAFE_A.h"
#include "STSAFE_A_I2C.h"
#define STSAFE_A_POWER					PB_14
#define STRING_MAX_SIZE         200
//static char testbuffer[STRING_MAX_SIZE];

EEPROMInterface::EEPROMInterface() :
    STSAFE_A_Power(STSAFE_A_POWER),
    STSAFE_A_I2c(I2C_SDA, I2C_SCL),
    Peripheral(&STSAFE_A_I2c, 0x20)
{
  // Power on supply
  STSAFE_A_Power = 1;
	DELAY(50);

	// Set up I2C
	STSAFE_A_I2c.Frequency(400000);

	// Configure I2C communication buffer
	uint8_t	StatusCode = Peripheral.DataBufferConfiguration();
}

EEPROMInterface::~EEPROMInterface()
{

}

ResponseCode EEPROMInterface::write(uint8_t* dataBuff, int buffSize, uint8_t dataZoneIndex)
{
    ResponseCode responseCode;
    
	DataBuffer *bufferinfo = (DataBuffer*)malloc(sizeof(DataBuffer) + buffSize - 1);
    bufferinfo->Length = buffSize;
    memcpy(bufferinfo->Data, dataBuff, buffSize);
    
    responseCode = Peripheral.Update(0x00, 0x00, 0x00, ALWAYS, dataZoneIndex, 0x00, bufferinfo, NO_MAC);
    
    free(bufferinfo);
    
    return responseCode;
}

int EEPROMInterface::read(uint8_t* dataBuff, int buffSize, uint8_t dataZoneIndex)
{
    ResponseCode responseCode;

	ReadBuffer* sts_read = NULL;
	responseCode = Peripheral.Read(0x00, 0x00, ALWAYS, dataZoneIndex, 0x00, buffSize, &sts_read, NO_MAC);
	
	if (responseCode == 0)
	{
        memcpy(dataBuff, sts_read->Data, sts_read->Length);
        
        return sts_read->Length;
	}

  return -1;
}

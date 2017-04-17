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
#include "mbed.h"
#include "STSAFE_A.h"
#include "STSAFE_A_I2C.h"
#include "EEPROMInterface.h"

#define STSAFE_A_POWER_PIN PB_14

EEPROMInterface::EEPROMInterface()
{
    
}

EEPROMInterface::~EEPROMInterface()
{

}

int EEPROMInterface::write(uint8_t* dataBuff, int buffSize, uint8_t dataZoneIndex)
{
    if (dataBuff == NULL || buffSize <= 0)
    {
        return -1;
    }
    
    // Power on supply
    DigitalOut STSAFE_A_Power(STSAFE_A_POWER_PIN);
    STSAFE_A_Power = 1;
    DELAY(50);

    // Set up I2C
	cSTSAFE_A_I2c STSAFE_A_I2c(I2C_SDA, I2C_SCL);
	STSAFE_A_I2c.Frequency(400000);

    // Configure I2C communication buffer
	cSTSAFE_A Peripheral(&STSAFE_A_I2c, 0x20);
	Peripheral.DataBufferConfiguration();
    
    // Prepare the data
	DataBuffer *bufferinfo = (DataBuffer*)malloc(sizeof(DataBuffer) + buffSize - 1);
    bufferinfo->Length = buffSize;
    memcpy(bufferinfo->Data, dataBuff, buffSize);
    
    // Write
    ResponseCode result = Peripheral.Update(0x00, 0x00, 0x00, ALWAYS, dataZoneIndex, 0x00, bufferinfo, NO_MAC);
    free(bufferinfo);
    
    if (result != 0)
    {
        return -1;
    }
    return 0;
}

int EEPROMInterface::read(uint8_t* dataBuff, int buffSize, uint16_t offset, uint8_t dataZoneIndex)
{
    if (dataBuff == NULL)
    {
        return -1;
    }
    
    // Power on supply
    DigitalOut STSAFE_A_Power(STSAFE_A_POWER_PIN);
    STSAFE_A_Power = 1;
    DELAY(50);

    // Set up I2C
	cSTSAFE_A_I2c STSAFE_A_I2c(I2C_SDA, I2C_SCL);
	STSAFE_A_I2c.Frequency(400000);

    // Configure I2C communication buffer
	cSTSAFE_A Peripheral(&STSAFE_A_I2c, 0x20);
	Peripheral.DataBufferConfiguration();

    // Read
	ReadBuffer* sts_read = NULL;
	ResponseCode result = Peripheral.Read(0x00, 0x00, ALWAYS, dataZoneIndex, offset, buffSize, &sts_read, NO_MAC);
	if (result == 0)
	{
        memcpy(dataBuff, sts_read->Data, sts_read->Length);
        
        return sts_read->Length;
	}
    
    return -1;
}

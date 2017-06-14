/**
 ******************************************************************************
 * @file    HTS221Sensor.h
 * @author  CLab
 * @version V1.0.0
 * @date    5 August 2016
 * @brief   Abstract class of an HTS221 Humidity and Temperature sensor.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */


/* Prevent recursive inclusion -----------------------------------------------*/

#ifndef __HTS221Sensor_H__
#define __HTS221Sensor_H__


/* Includes ------------------------------------------------------------------*/

#include "ST_INTERFACES/DevI2C.h"
#include "utility/HTS221_driver.h"
#include "ST_INTERFACES/HumiditySensor.h"
#include "ST_INTERFACES/TempSensor.h"


/* Class Declaration ---------------------------------------------------------*/

/**
 * Abstract class of an HTS221 Humidity and Temperature sensor.
 */
class HTS221Sensor : public HumiditySensor, public TempSensor
{
  public:

    HTS221Sensor(DevI2C &i2c);
    HTS221Sensor(DevI2C &i2c, unsigned char address);
    virtual int init(void *init);
    virtual int readId(unsigned char *id);
    virtual int getHumidity(float *pfData);
    virtual int getTemperature(float *pfData);
    int enable(void);
    int disable(void);
    int reset(void);
    int getOdr(float *odr);
    int setOdr(float odr);

    /**
     * @brief Utility function to read data.
     * @param  pBuffer: pointer to data to be read.
     * @param  RegisterAddr: specifies internal address register to be read.
     * @param  NumByteToRead: number of bytes to be read.
     * @retval 0 if ok, an error code otherwise.
     */
    unsigned char readIO(unsigned char* pBuffer, unsigned char RegisterAddr, int NumByteToRead)
    {
        return (unsigned char) _dev_i2c.i2c_read(pBuffer, _address, RegisterAddr, (uint16_t)NumByteToRead);
    }
    
    /**
     * @brief Utility function to write data.
     * @param  pBuffer: pointer to data to be written.
     * @param  RegisterAddr: specifies internal address register to be written.
     * @param  NumByteToWrite: number of bytes to write.
     * @retval 0 if ok, an error code otherwise.
     */
    unsigned char writeIO(unsigned char* pBuffer, unsigned char RegisterAddr, int NumByteToWrite)
    {
        return (unsigned char) _dev_i2c.i2c_write(pBuffer, _address, RegisterAddr, (uint16_t)NumByteToWrite);
    }

  private:
    int readReg(unsigned char reg, unsigned char *data);
    int writeReg(unsigned char reg, unsigned char data);

    /* Helper classes. */
    DevI2C &_dev_i2c;
    
    /* Configuration */
    uint8_t _address;

};

#ifdef __cplusplus
 extern "C" {
#endif
unsigned char HTS221_io_write( void *handle, unsigned char WriteAddr, unsigned char *pBuffer, int nBytesToWrite );
unsigned char HTS221_io_read( void *handle, unsigned char ReadAddr, unsigned char *pBuffer, int nBytesToRead );
#ifdef __cplusplus
  }
#endif

#endif

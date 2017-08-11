/**
 ******************************************************************************
 * @file    LPS22HBSensor.cpp
 * @author  CLab
 * @version V1.0.0
 * @date    5 August 2016
 * @brief   Implementation of an LPS22HB Humidity and Temperature sensor.
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


/* Includes ------------------------------------------------------------------*/

#include "mbed.h"
#include "ST_INTERFACES/DevI2C.h"
#include "LPS22HBSensor.h"


/* Class Implementation ------------------------------------------------------*/

/** Constructor
 * @param i2c object of an helper class which handles the I2C peripheral
 * @param address the address of the component's instance
 */
LPS22HBSensor::LPS22HBSensor(DevI2C &i2c) : _dev_i2c(i2c)
{
  _address = LPS22HB_ADDRESS_LOW;
};


/** Constructor
 * @param i2c object of an helper class which handles the I2C peripheral
 * @param address the address of the component's instance
 */
LPS22HBSensor::LPS22HBSensor(DevI2C &i2c, unsigned char address) : _dev_i2c(i2c), _address(address)
{

};

/**
 * @brief     Initializing the component.
 * @param[in] init pointer to device specific initialization structure.
 * @retval    "0" in case of success, an error code otherwise.
 */
int LPS22HBSensor::init(void *init)
{
  lps22hb_sensor_init(this);
  return 0;
}

/**
 * @brief   Deinitialize lps22hb sensor device.
 * @retval    "0" in case of success, an error code otherwise.
 */
int LPS22HBSensor::deInit()
{
  lps22hb_sensor_deinit(this);
  return 0;
}

/**
 * @brief  Read ID address of LPS22HB
 * @param  id the pointer where the ID of the device is stored
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::readId(unsigned char *id)
{
  if(!id)
  { 
    return 1;
  }
  
  /* Read WHO AM I register */
  if ( LPS22HB_ReadID(this, id) != PRESSURE_OK)
  {
    return 1;
  }

  return 0;
}

/**
 * @brief  Read LPS22HB output register, and calculate the humidity
 * @param  pfData the pointer to data output
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::getPressure(float* pfData)
{
  /* Read data from LPS22HB. */
  if ( LPS22HB_GetPressure(this, pfData) != PRESSURE_OK )
  {
    return 1;
  }
  return 0;
}

/**
 * @brief  Read LPS22HB output register, and calculate the temperature
 * @param  pfData the pointer to data output
 * @retval 0 in case of success, an error code otherwise
 */
int LPS22HBSensor::getTemperature(float* pfData)
{
  /* Read data from LPS22HB. */
  if ( LPS22HB_GetTemperature(this, pfData) != PRESSURE_OK )
  {
    return 1;
  }
  return 0;
}

unsigned char LPS22HB_io_write( void *handle, unsigned char WriteAddr, unsigned char *pBuffer, int nBytesToWrite )
{
  return ((LPS22HBSensor *)handle)->writeIO(pBuffer, WriteAddr, nBytesToWrite);
}

unsigned char LPS22HB_io_read( void *handle, unsigned char ReadAddr, unsigned char *pBuffer, int nBytesToRead )
{
  return ((LPS22HBSensor *)handle)->readIO(pBuffer, ReadAddr, nBytesToRead);
}

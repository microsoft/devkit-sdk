/**
 ******************************************************************************
 * @file    lps25h_class.h
 * @author  AST / EST
 * @version V0.0.1
 * @date    14-April-2015
 * @brief   Header file for component LPS25H
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
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
 *       without specific prior written permission.
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

#ifndef __LPS25H_CLASS_H
#define __LPS25H_CLASS_H

/* Includes ------------------------------------------------------------------*/
#include "mbed.h"
#include "ST_INTERFACES/DevI2C.h"
#include "utility/lps25h.h"
#include "ST_INTERFACES/PressureSensor.h"
#include "ST_INTERFACES/TempSensor.h"

/* Classes -------------------------------------------------------------------*/
/** Class representing a LPS25H sensor component
 */
class LPS25H : public PressureSensor, public TempSensor {
 public:
	/** Constructor
	 * @param[in] i2c device I2C to be used for communication
	 */
        LPS25H(DevI2C &i2c) : PressureSensor(), TempSensor(), dev_i2c(i2c) {
		LPS25H_SlaveAddress = LPS25H_ADDRESS_HIGH;
	}
	
	/** Destructor
	 */
        virtual ~LPS25H() {}
	
	/*** Interface Methods ***/
	virtual int init(void *init_struct) {
		return LPS25H_Init((PRESSURE_InitTypeDef*)init_struct);
	}

	/**
	 * @brief       Enter sensor shutdown mode
	 * @return      0 in case of success, an error code otherwise
	 */
	virtual int PowerOff(void) {
		return LPS25H_PowerOff();
	}

	virtual int read_id(uint8_t *p_id) {
		return LPS25H_ReadID(p_id);
	}

	/**
	 * @brief       Reset sensor
	 * @return      0 in case of success, an error code otherwise
	 */
	virtual int Reset(void) {
		return LPS25H_RebootCmd();
	}

	virtual int get_pressure(float *pfData) {
		return LPS25H_GetPressure(pfData);
	}

	virtual int get_temperature(float *pfData) {
		return LPS25H_GetTemperature(pfData);
	}

	void SlaveAddrRemap(uint8_t SA0_Bit_Status) {
		LPS25H_SlaveAddrRemap(SA0_Bit_Status);
	}

protected:
	/*** Methods ***/
	PRESSURE_StatusTypeDef LPS25H_Init(PRESSURE_InitTypeDef *LPS25H_Init);
	PRESSURE_StatusTypeDef LPS25H_ReadID(uint8_t *p_id);
	PRESSURE_StatusTypeDef LPS25H_RebootCmd(void);
	PRESSURE_StatusTypeDef LPS25H_GetPressure(float* pfData);
	PRESSURE_StatusTypeDef LPS25H_GetTemperature(float* pfData);
	PRESSURE_StatusTypeDef LPS25H_PowerOff(void);
	void LPS25H_SlaveAddrRemap(uint8_t SA0_Bit_Status);
	
	PRESSURE_StatusTypeDef LPS25H_PowerOn(void);
	PRESSURE_StatusTypeDef LPS25H_I2C_ReadRawPressure(int32_t *raw_press);
	PRESSURE_StatusTypeDef LPS25H_I2C_ReadRawTemperature(int16_t *raw_data);

	/**
	 * @brief  Configures LPS25H interrupt lines for NUCLEO boards
	 */
	void LPS25H_IO_ITConfig(void)
	{
		/* To be implemented */
	}

	/**
	 * @brief  Configures LPS25H I2C interface
	 * @return PRESSURE_OK in case of success, an error code otherwise
	 */
	PRESSURE_StatusTypeDef LPS25H_IO_Init(void)
	{
		return PRESSURE_OK; /* done in constructor */
	}

	/**
	 * @brief      Utility function to read data from LPS25H
	 * @param[out] pBuffer pointer to the byte-array to read data in to
	 * @param[in]  RegisterAddr specifies internal address register to read from.
	 * @param[in]  NumByteToRead number of bytes to be read.
	 * @retval     PRESSURE_OK if ok, 
	 * @retval     PRESSURE_ERROR if an I2C error has occured
	 */
	PRESSURE_StatusTypeDef LPS25H_IO_Read(uint8_t* pBuffer, 
					      uint8_t RegisterAddr, uint16_t NumByteToRead)
	{
		int ret = dev_i2c.i2c_read(pBuffer,
					   LPS25H_SlaveAddress,
					   RegisterAddr,
					   NumByteToRead);
		if(ret != 0) {
			return PRESSURE_ERROR;
		}
		return PRESSURE_OK;
	}
	
	/**
	 * @brief      Utility function to write data to LPS25H
	 * @param[in]  pBuffer pointer to the byte-array data to send
	 * @param[in]  RegisterAddr specifies internal address register to read from.
	 * @param[in]  NumByteToWrite number of bytes to write.
	 * @retval     PRESSURE_OK if ok, 
	 * @retval     PRESSURE_ERROR if an I2C error has occured
	 */
	PRESSURE_StatusTypeDef LPS25H_IO_Write(uint8_t* pBuffer, 
					       uint8_t RegisterAddr, uint16_t NumByteToWrite)
	{
		int ret = dev_i2c.i2c_write(pBuffer,
					    LPS25H_SlaveAddress,
					    RegisterAddr,
					    NumByteToWrite);
		if(ret != 0) {
			return PRESSURE_ERROR;
		}
		return PRESSURE_OK;
	}
	
	/*** Instance Variables ***/
	/* IO Device */
	DevI2C &dev_i2c;

	uint8_t LPS25H_SlaveAddress;
};

#endif // __LPS25H_CLASS_H

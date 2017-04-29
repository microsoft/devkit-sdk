/**
 ******************************************************************************
 * @file    lps25h_class.cpp
 * @author  AST / EST
 * @version V0.0.1
 * @date    14-April-2015
 * @brief   Implementation file for the LPS25H driver class
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
#include "lps25h_class.h"
#include "utility/lps25h.h"

/* Methods -------------------------------------------------------------------*/
/* betzw - based on:
           X-CUBE-MEMS1/trunk/Drivers/BSP/Components/lps25h/lps25h.c: revision #400,
           X-CUBE-MEMS1/trunk: revision #416
*/

/**
 * @brief  Set LPS25H Initialization
 * @param  LPS25H_Init the configuration setting for the LPS25H
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS25H::LPS25H_Init(PRESSURE_InitTypeDef *LPS25H_Init)
{
  uint8_t tmp1 = 0x00;
  
  /* Configure the low level interface ---------------------------------------*/
  if(LPS25H_IO_Init() != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  if(LPS25H_PowerOn() != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  if(LPS25H_IO_Read(&tmp1, LPS25H_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  /* Output Data Rate selection */
  tmp1 &= ~(LPS25H_ODR_MASK);
  tmp1 |= LPS25H_Init->OutputDataRate;
  
  /* Interrupt circuit selection */
  tmp1 &= ~(LPS25H_DIFF_EN_MASK);
  tmp1 |= LPS25H_Init->DiffEnable;
  
  /* Block Data Update selection */
  tmp1 &= ~(LPS25H_BDU_MASK);
  tmp1 |= LPS25H_Init->BlockDataUpdate;
  
  /* Serial Interface Mode selection */
  tmp1 &= ~(LPS25H_SPI_SIM_MASK);
  tmp1 |= LPS25H_Init->SPIMode;
  
  if(LPS25H_IO_Write(&tmp1, LPS25H_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  if(LPS25H_IO_Read(&tmp1, LPS25H_RES_CONF_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  /* Serial Interface Mode selection */
  tmp1 &= ~(LPS25H_P_RES_MASK);
  tmp1 |= LPS25H_Init->PressureResolution;
  
  /* Serial Interface Mode selection */
  tmp1 &= ~(LPS25H_T_RES_MASK);
  tmp1 |= LPS25H_Init->TemperatureResolution;
  
  if(LPS25H_IO_Write(&tmp1, LPS25H_RES_CONF_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  LPS25H_IO_ITConfig();
  
  return PRESSURE_OK;
}

/**
 * @brief  Read ID address of LPS25H
 * @param  ht_id the pointer where the ID of the device is stored
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS25H::LPS25H_ReadID(uint8_t *p_id)
{
  if(!p_id)
  {
    return PRESSURE_ERROR;
  }
  
  return LPS25H_IO_Read(p_id, LPS25H_WHO_AM_I_ADDR, 1);
}

/**
 * @brief  Reboot memory content of LPS25H
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS25H::LPS25H_RebootCmd(void)
{
  uint8_t tmpreg;
  
  /* Read CTRL_REG5 register */
  if(LPS25H_IO_Read(&tmpreg, LPS25H_CTRL_REG2_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  /* Enable or Disable the reboot memory */
  tmpreg |= LPS25H_RESET_MEMORY;
  
  /* Write value to MEMS CTRL_REG5 regsister */
  if(LPS25H_IO_Write(&tmpreg, LPS25H_CTRL_REG2_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  return PRESSURE_OK;
}


/**
 * @brief  Read LPS25H output register, and calculate the raw pressure
 * @param  raw_press the pressure raw value
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS25H::LPS25H_I2C_ReadRawPressure(int32_t *raw_press)
{
  uint8_t buffer[3], i;
  uint32_t tempVal = 0;
  
  /* Read the register content */
  
  if(LPS25H_IO_Read(buffer, (LPS25H_PRESS_POUT_XL_ADDR | LPS25H_I2C_MULTIPLEBYTE_CMD),
                    3) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  /* Build the raw data */
  for (i = 0 ; i < 3 ; i++)
    tempVal |= (((uint32_t) buffer[i]) << (8 * i));
    
  /* convert the 2's complement 24 bit to 2's complement 32 bit */
  if (tempVal & 0x00800000)
    tempVal |= 0xFF000000;
    
  /* return the built value */
  *raw_press = ((int32_t) tempVal);
  
  return PRESSURE_OK;
}

/**
 * @brief  Read LPS25H output register, and calculate the pressure in mbar
 * @param  pfData the pressure value in mbar
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS25H::LPS25H_GetPressure(float* pfData)
{
  int32_t raw_press = 0;
  
  if(LPS25H_I2C_ReadRawPressure(&raw_press) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  *pfData = (float)raw_press / 4096.0f;
  
  return PRESSURE_OK;
}

/**
 * @brief  Read LPS25H output register, and calculate the raw temperature
 * @param  raw_data the temperature raw value
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS25H::LPS25H_I2C_ReadRawTemperature(int16_t *raw_data)
{
  uint8_t buffer[2];
  uint16_t tempVal = 0;
  
  /* Read the register content */
  if(LPS25H_IO_Read(buffer, (LPS25H_TEMP_OUT_L_ADDR | LPS25H_I2C_MULTIPLEBYTE_CMD),
                    2) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  /* Build the raw value */
  tempVal = (((uint16_t)buffer[1]) << 8) + (uint16_t)buffer[0];
  
  /* Return it */
  *raw_data = ((int16_t)tempVal);
  
  return PRESSURE_OK;
}

/**
 * @brief  Read LPS25H output register, and calculate the temperature
 * @param  pfData the temperature value
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS25H::LPS25H_GetTemperature(float *pfData)
{
  int16_t raw_data;
  
  if(LPS25H_I2C_ReadRawTemperature(&raw_data) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  *pfData = (float)((((float)raw_data / 480.0f) + 42.5f));
  
  return PRESSURE_OK;
}
/**
 * @brief  Exit the shutdown mode for LPS25H
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS25H::LPS25H_PowerOn(void)
{
  uint8_t tmpreg;
  
  /* Read the register content */
  if(LPS25H_IO_Read(&tmpreg, LPS25H_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  /* Set the power down bit */
  tmpreg |= LPS25H_MODE_ACTIVE;
  
  /* Write register */
  if(LPS25H_IO_Write(&tmpreg, LPS25H_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  return PRESSURE_OK;
}


/**
 * @brief  Enter the shutdown mode for LPS25H
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS25H::LPS25H_PowerOff(void)
{
  uint8_t tmpreg;
  
  /* Read the register content */
  if(LPS25H_IO_Read(&tmpreg, LPS25H_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  /* Reset the power down bit */
  tmpreg &= ~(LPS25H_MODE_ACTIVE);
  
  /* Write register */
  if(LPS25H_IO_Write(&tmpreg, LPS25H_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  return PRESSURE_OK;
}

/**
 * @brief  Set the slave address according to SA0 bit
 * @param  SA0_Bit_Status LPS25H_SA0_LOW or LPS25H_SA0_HIGH
 * @retval None
 */
void LPS25H::LPS25H_SlaveAddrRemap(uint8_t SA0_Bit_Status)
{
  LPS25H_SlaveAddress = (SA0_Bit_Status == LPS25H_SA0_LOW ? LPS25H_ADDRESS_LOW : LPS25H_ADDRESS_HIGH);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

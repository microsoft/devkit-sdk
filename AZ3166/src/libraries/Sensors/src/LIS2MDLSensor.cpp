/**
 ******************************************************************************
 * @file    lis2mdlSensor.cpp
 * @author  AST / EST
 * @version V0.0.1
 * @date    14-April-2015
 * @brief   Implementation file for the LIS2MDL driver class
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
#include "LIS2MDLSensor.h"

/**
 * @brief  Set LIS2MDL Initialization
 * @retval MAGNETO_OK in case of success, an error code otherwise
 */
MAGNETO_StatusTypeDef LIS2MDLSensor::LIS2MDLInit()
{
  uint8_t tmp1 = 0x00;
  MAGNETO_StatusTypeDef ret;

  tmp1 = 0x8C;
  ret = LIS2MDL_IO_Write(&tmp1, CFG_REG_A, 1);
  if (ret != MAGNETO_OK) return ret;
  tmp1 = 0x02;
  ret = LIS2MDL_IO_Write(&tmp1, CFG_REG_B, 1);
  if (ret != MAGNETO_OK) return ret;
  tmp1 = 0x10;
  ret = LIS2MDL_IO_Write(&tmp1, CFG_REG_C, 1);
  if (ret != MAGNETO_OK) return ret;
  
  return MAGNETO_OK;
  
  /******************************/
}


/**
 * @brief  Read ID of LIS2MDL Magnetic sensor
 * @param  m_id the pointer where the ID of the device is stored
 * @retval MAGNETO_OK in case of success, an error code otherwise
 */
MAGNETO_StatusTypeDef LIS2MDLSensor::LIS2MDL_Read_M_ID(uint8_t *m_id)
{
  if(!m_id)
  {
    return MAGNETO_ERROR;
  }
  
  return LIS2MDL_IO_Read(m_id, WHO_AM_I_ADDR, 1);
}


/**
 * @brief  Read raw data from LIS2MDL Magnetic sensor output register
 * @param  pData the pointer where the magnetometer raw data are stored
 * @retval MAGNETO_OK in case of success, an error code otherwise
 */
MAGNETO_StatusTypeDef LIS2MDLSensor::LIS2MDL_M_GetAxesRaw(int16_t *pData)
{
  uint8_t tempReg[2] = {0, 0};
  MAGNETO_StatusTypeDef ret;

  ret = LIS2MDL_IO_Read(&tempReg[0], OUTX_L_REG, 1);
  if (ret != MAGNETO_OK) return ret;
  ret = LIS2MDL_IO_Read(&tempReg[1], OUTX_H_REG, 1);
  if (ret != MAGNETO_OK) return ret;
  pData[0] = ((((int16_t)tempReg[1]) << 8) + (int16_t)tempReg[0]);
  
  ret = LIS2MDL_IO_Read(&tempReg[0], OUTY_L_REG, 1);
  if (ret != MAGNETO_OK) return ret;
  ret = LIS2MDL_IO_Read(&tempReg[1], OUTY_H_REG, 1);
  if (ret != MAGNETO_OK) return ret;
  pData[1] = ((((int16_t)tempReg[1]) << 8) + (int16_t)tempReg[0]);
  
  ret = LIS2MDL_IO_Read(&tempReg[0], OUTZ_L_REG, 1);
  if (ret != MAGNETO_OK) return ret;
  ret = LIS2MDL_IO_Read(&tempReg[1], OUTZ_H_REG, 1);
  if (ret != MAGNETO_OK) return ret;
  pData[2] = ((((int16_t)tempReg[1]) << 8) + (int16_t)tempReg[0]);\
  
  return MAGNETO_OK;
}


/**
 * @brief Read data from LIS2MDL Magnetic sensor and calculate Magnetic in mgauss
 * @param pData the pointer where the magnetometer data are stored
 * @retval MAGNETO_OK in case of success, an error code otherwise
 */
MAGNETO_StatusTypeDef LIS2MDLSensor::LIS2MDL_M_GetAxes(int32_t *pData)
{
  int16_t pDataRaw[3];
  
  if(LIS2MDL_M_GetAxesRaw(pDataRaw) != MAGNETO_OK)
  {
    return MAGNETO_ERROR;
  }
  
  pData[0] = (int32_t)(pDataRaw[0]);
  pData[1] = (int32_t)(pDataRaw[1]);
  pData[2] = (int32_t)(pDataRaw[2]);
  
  return MAGNETO_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

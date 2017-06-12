/* MiCO Team
 * Copyright (c) 2017 MXCHIP Information Tech. Co.,Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Includes ------------------------------------------------------------------*/
#include "LPS22HB_driver.h"
#include "mico.h"

#define lps25hb_log(M, ...) custom_log("LPS22HB", M, ##__VA_ARGS__)
#define lps25hb_log_trace() custom_log_trace("LPS22HB")

/* ------------------------------------------------------- */ 
/* Here you should declare the variable that implements    */
/* the internal struct of extended features of LPS22HB.    */
/* Then you must update the NULL pointer in the variable   */
/* of the extended features below.                         */
/* See the example of LSM6DS3 in lsm6ds3.c                 */
/* ------------------------------------------------------- */

extern unsigned char LPS22HB_io_write( void *handle, unsigned char WriteAddr, unsigned char *pBuffer, int nBytesToWrite );
extern unsigned char LPS22HB_io_read( void *handle, unsigned char ReadAddr, unsigned char *pBuffer, int nBytesToRead );

PRESSURE_DrvExtTypeDef LPS22HBDrv_ext = {
    PRESSURE_LPS22HB_DIL24_COMPONENT, /* unique ID for LPS22HB in the PRESSURE driver class */
    NULL /* pointer to internal struct of extended features of LPS22HB */
}; 

/* I2C device */
mico_i2c_device_t lps25hb_i2c_device = {
  LPS22HB_I2C_PORT, 0x5C, I2C_ADDRESS_WIDTH_7BIT, I2C_STANDARD_SPEED_MODE
};

PRESSURE_StatusTypeDef LPS22HB_IO_Init(void)
{
  // I2C init
  MicoI2cFinalize(&lps25hb_i2c_device);   // in case error
  MicoI2cInitialize(&lps25hb_i2c_device);

  if( false == MicoI2cProbeDevice(&lps25hb_i2c_device, 50) ){
    lps25hb_log("LPS22HB_ERROR: no i2c device found!");
    return PRESSURE_ERROR;
  }
  return PRESSURE_OK;
}

/**
 * @brief  Set LPS22HB Initialization
 * @param  LPS22HB_Init the configuration setting for the LPS22HB
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS22HB_Init(void *handle, PRESSURE_InitTypeDef *LPS22HB_Init)
{  
    uint8_t tmp1 = 0x00;

    /* Configure the low level interface ---------------------------------------*/
    if(LPS22HB_IO_Init() != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    if(LPS22HB_PowerOn(handle) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    if(LPS22HB_io_read(handle, LPS22HB_CTRL_REG1_ADDR, &tmp1, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    /* Output Data Rate selection */
    tmp1 &= ~(LPS22HB_ODR_MASK);
    tmp1 |= LPS22HB_Init->OutputDataRate;

    /* Interrupt circuit selection */
    tmp1 &= ~(LPS22HB_DIFF_EN_MASK);
    tmp1 |= LPS22HB_Init->DiffEnable;

    /* Block Data Update selection */
    tmp1 &= ~(LPS22HB_BDU_MASK);
    tmp1 |= LPS22HB_Init->BlockDataUpdate;

    /* Serial Interface Mode selection */
    tmp1 &= ~(LPS22HB_SPI_SIM_MASK);
    tmp1 |= LPS22HB_Init->SPIMode;

    if(LPS22HB_io_write(handle, LPS22HB_CTRL_REG1_ADDR, &tmp1, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }      

    if(LPS22HB_io_read(handle, LPS22HB_RES_CONF_ADDR, &tmp1, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    /* Serial Interface Mode selection */
    tmp1 &= ~(LPS22HB_P_RES_MASK);
    tmp1 |= LPS22HB_Init->PressureResolution;

    /* Serial Interface Mode selection */
    tmp1 &= ~(LPS22HB_T_RES_MASK);
    tmp1 |= LPS22HB_Init->TemperatureResolution;

    if(LPS22HB_io_write(handle, LPS22HB_RES_CONF_ADDR, &tmp1, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }
    
    return PRESSURE_OK;
}

/**
 * @brief  Read ID address of LPS22HB
 * @param  ht_id the pointer where the ID of the device is stored
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS22HB_ReadID(void *handle, uint8_t *p_id)
{
    if(!p_id)
    { 
      return PRESSURE_ERROR;
    }
 
    return LPS22HB_io_read(handle, LPS22HB_WHO_AM_I_ADDR, p_id, 1);
}

/**
 * @brief  Reboot memory content of LPS22HB
 * @param  None
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS22HB_RebootCmd(void *handle)
{
    uint8_t tmpreg;

    /* Read CTRL_REG5 register */
    if(LPS22HB_io_read(handle, LPS22HB_CTRL_REG2_ADDR, &tmpreg, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    /* Enable or Disable the reboot memory */
    tmpreg |= LPS22HB_RESET_MEMORY;

    /* Write value to MEMS CTRL_REG5 regsister */
    if(LPS22HB_io_write(handle, LPS22HB_CTRL_REG2_ADDR, &tmpreg, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }
    
    return PRESSURE_OK;
}


/**
 * @brief  Read LPS22HB output register, and calculate the raw pressure
 * @param  raw_press the pressure raw value
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS22HB_I2C_ReadRawPressure(void *handle, uint32_t *raw_press)
{
    uint8_t buffer[3], i;
    uint32_t tempVal=0;
    uint8_t tmp = 0x00;
    
    if(LPS22HB_io_read(handle, LPS22HB_CTRL_REG1_ADDR, &tmp, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    /* Output Data Rate selection */
    tmp &= (LPS22HB_ODR_MASK);
    
    if(tmp == 0x00)
    {
      if(LPS22HB_io_read(handle, LPS22HB_CTRL_REG2_ADDR, &tmp, 1) != PRESSURE_OK)
      {
        return PRESSURE_ERROR;
      }

      /* Serial Interface Mode selection */
      tmp &= ~(LPS22HB_ONE_SHOT_MASK);
      tmp |= LPS22HB_ONE_SHOT_START;

      if(LPS22HB_io_write(handle, LPS22HB_CTRL_REG2_ADDR, &tmp, 1) != PRESSURE_OK)
      {
        return PRESSURE_ERROR;
      }
    
      do{
      
        if(LPS22HB_io_read(handle, LPS22HB_STATUS_REG_ADDR, &tmp, 1) != PRESSURE_OK)
        {
          return PRESSURE_ERROR;
        }
       
      }while(!(tmp&&0x01));
    }    
    
    /* Read the register content */

    if(LPS22HB_io_read(handle, (LPS22HB_PRESS_POUT_XL_ADDR | LPS22HB_I2C_MULTIPLEBYTE_CMD), buffer, 3) != PRESSURE_OK)
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
    *raw_press = ((uint32_t) tempVal);
    
    return PRESSURE_OK;
}

/**
 * @brief  Read LPS22HB output register, and calculate the pressure in mbar
 * @param  pfData the pressure value in mbar
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS22HB_GetPressure(void *handle, float* pfData)
{
    uint32_t raw_press = 0;
    uint8_t tmp = 0x00;
    
    if(LPS22HB_io_read(handle, LPS22HB_CTRL_REG1_ADDR, &tmp, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    /* Output Data Rate selection */
    tmp &= (LPS22HB_ODR_MASK);
    
    if(tmp == 0x00)
    {
      if(LPS22HB_io_read(handle, LPS22HB_CTRL_REG2_ADDR, &tmp, 1) != PRESSURE_OK)
      {
        return PRESSURE_ERROR;
      }

      /* Serial Interface Mode selection */
      tmp &= ~(LPS22HB_ONE_SHOT_MASK);
      tmp |= LPS22HB_ONE_SHOT_START;

      if(LPS22HB_io_write(handle, LPS22HB_CTRL_REG2_ADDR, &tmp, 1) != PRESSURE_OK)
      {
        return PRESSURE_ERROR;
      }
    
      do{
      
        if(LPS22HB_io_read(handle, LPS22HB_STATUS_REG_ADDR, &tmp, 1) != PRESSURE_OK)
        {
          return PRESSURE_ERROR;
        }
       
      }while(!(tmp&&0x01));
    }    

    if(LPS22HB_I2C_ReadRawPressure(handle, &raw_press) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    *pfData = (float)raw_press /4096.0f;
    
    return PRESSURE_OK;
}

/**
 * @brief  Read LPS22HB output register, and calculate the raw temperature
 * @param  raw_data the temperature raw value
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS22HB_I2C_ReadRawTemperature(void *handle, int16_t *raw_data)
{
    uint8_t buffer[2];
    uint16_t tempVal=0;

    /* Read the register content */
    if(LPS22HB_io_read(handle, LPS22HB_TEMP_OUT_L_ADDR, &buffer[0], 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    if(LPS22HB_io_read(handle, LPS22HB_TEMP_OUT_H_ADDR, &buffer[1], 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }
    /* Build the raw value */
    tempVal = (((uint16_t)buffer[1]) << 8)+(uint16_t)buffer[0];

    /* Return it */
    *raw_data = ((int16_t)tempVal);
    
    return PRESSURE_OK;
}

/**
 * @brief  Read LPS22HB output register, and calculate the temperature
 * @param  pfData the temperature value
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS22HB_GetTemperature(void *handle, float *pfData)
{
    int16_t raw_data;
    uint8_t tmp = 0x00;
    
    if(LPS22HB_io_read(handle, LPS22HB_CTRL_REG1_ADDR, &tmp, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    /* Output Data Rate selection */
    tmp &= (LPS22HB_ODR_MASK);
    
    if(tmp == 0x00)
    {
      if(LPS22HB_io_read(handle, LPS22HB_CTRL_REG2_ADDR, &tmp, 1) != PRESSURE_OK)
      {
        return PRESSURE_ERROR;
      }

      /* Serial Interface Mode selection */
      tmp &= ~(LPS22HB_ONE_SHOT_MASK);
      tmp |= LPS22HB_ONE_SHOT_START;

      if(LPS22HB_io_write(handle, LPS22HB_CTRL_REG2_ADDR, &tmp, 1) != PRESSURE_OK)
      {
        return PRESSURE_ERROR;
      }
    
      do{
      
        if(LPS22HB_io_read(handle, LPS22HB_STATUS_REG_ADDR, &tmp, 1) != PRESSURE_OK)
        {
          return PRESSURE_ERROR;
        }
       
      }while(!(tmp&&0x01));
    }

    if(LPS22HB_I2C_ReadRawTemperature(handle, &raw_data) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }
    *pfData = (float)raw_data/100.0f;
    
    return PRESSURE_OK;
}
/**
 * @brief  Exit the shutdown mode for LPS22HB
 * @param  None
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS22HB_PowerOn(void *handle)
{
    uint8_t tmpreg;

    /* Read the register content */
    if(LPS22HB_io_read(handle, LPS22HB_CTRL_REG1_ADDR, &tmpreg, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    /* Set the power down bit */
    tmpreg |= LPS22HB_MODE_ACTIVE;

    /* Write register */
    if(LPS22HB_io_write(handle, LPS22HB_CTRL_REG1_ADDR, &tmpreg, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }
    
    return PRESSURE_OK;
}


/**
 * @brief  Enter the shutdown mode for LPS22HB
 * @param  None
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS22HB_PowerOff(void *handle)
{
    uint8_t tmpreg;

    /* Read the register content */
    if(LPS22HB_io_read(handle, LPS22HB_CTRL_REG1_ADDR, &tmpreg, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    /* Reset the power down bit */
    tmpreg &= ~(LPS22HB_MODE_ACTIVE);

    /* Write register */
    if(LPS22HB_io_write(handle, LPS22HB_CTRL_REG1_ADDR, &tmpreg, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }
    
    return PRESSURE_OK;
}

PRESSURE_StatusTypeDef lps22hb_sensor_init(void *handle)
{
  PRESSURE_InitTypeDef lps25hb;
  lps25hb.OutputDataRate = LPS22HB_ODR_ONE_SHOT;
  lps25hb.PressureResolution = LPS22HB_P_RES_AVG_32;
  lps25hb.TemperatureResolution = LPS22HB_T_RES_AVG_32;
  lps25hb.DiffEnable = LPS22HB_DIFF_ENABLE;
  lps25hb.BlockDataUpdate = LPS22HB_BDU_READ;
  lps25hb.SPIMode = LPS22HB_SPI_SIM_4W;
     
  if(LPS22HB_Init(handle, &lps25hb) != PRESSURE_OK){
    return -1;
  }
  return 0;
}

PRESSURE_StatusTypeDef lps22hb_sensor_deinit(void *handle)
{
  if(LPS22HB_PowerOff(handle) != PRESSURE_OK){
    return -1;
  }
  if(MicoI2cFinalize(&lps25hb_i2c_device) != PRESSURE_OK){
    return -1;
  }
  return 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/     

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
#include "lps22hb.h"
#include "mico.h"

#define lps25hb_log(M, ...) custom_log("LPS22HB", M, ##__VA_ARGS__)
#define lps25hb_log_trace() custom_log_trace("LPS22HB")



static PRESSURE_StatusTypeDef LPS25HB_Init(PRESSURE_InitTypeDef *LPS25HB_Init);
static PRESSURE_StatusTypeDef LPS25HB_ReadID(uint8_t *p_id);
static PRESSURE_StatusTypeDef LPS25HB_RebootCmd(void);
static PRESSURE_StatusTypeDef LPS25HB_GetPressure(float* pfData);
static PRESSURE_StatusTypeDef LPS25HB_GetTemperature(float* pfData);
static PRESSURE_StatusTypeDef LPS25HB_PowerOff(void);
static void LPS25HB_SlaveAddrRemap(uint8_t SA0_Bit_Status);


PRESSURE_DrvTypeDef LPS25HBDrv =
{
 LPS25HB_Init,
 LPS25HB_PowerOff,
 LPS25HB_ReadID,
 LPS25HB_RebootCmd,
 0,
 0,
 0,
 0,
 0,
 LPS25HB_GetPressure,
 LPS25HB_GetTemperature,
 LPS25HB_SlaveAddrRemap,
 NULL
};
 
/* ------------------------------------------------------- */ 
/* Here you should declare the variable that implements    */
/* the internal struct of extended features of LPS25HB.    */
/* Then you must update the NULL pointer in the variable   */
/* of the extended features below.                         */
/* See the example of LSM6DS3 in lsm6ds3.c                 */
/* ------------------------------------------------------- */
 
PRESSURE_DrvExtTypeDef LPS25HBDrv_ext = {
    PRESSURE_LPS25HB_DIL24_COMPONENT, /* unique ID for LPS25HB in the PRESSURE driver class */
    NULL /* pointer to internal struct of extended features of LPS25HB */
}; 

uint8_t LPS25HB_SlaveAddress = LPS25HB_ADDRESS_LOW;

/* I2C device */
mico_i2c_device_t lps25hb_i2c_device = {
  LPS25HB_I2C_PORT, 0x5C, I2C_ADDRESS_WIDTH_7BIT, I2C_STANDARD_SPEED_MODE
};

PRESSURE_StatusTypeDef LPS25HB_IO_Init(void)
{
  // I2C init
  MicoI2cFinalize(&lps25hb_i2c_device);   // in case error
  MicoI2cInitialize(&lps25hb_i2c_device);

  if( false == MicoI2cProbeDevice(&lps25hb_i2c_device, 50) ){
    lps25hb_log("LPS25HB_ERROR: no i2c device found!");
    return PRESSURE_ERROR;
  }
  return PRESSURE_OK;
}


/*	\Brief: The function is used as I2C bus write
*	\Return : Status of the I2C write
*	\param dev_addr : The device address of the sensor
*	\param reg_addr : Address of the first register, will data is going to be written
*	\param reg_data : It is a value hold in the array,
*		will be used for write the value into the register
*	\param cnt : The no of byte of data to be write
*/
PRESSURE_StatusTypeDef LPS25HB_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint16_t NumByteToWrite)
{
  mico_i2c_message_t lps25hb_i2c_msg = {NULL, NULL, 0, 0, 0, false};
  int iError = 0;
  uint8_t array[8];
  uint8_t stringpos;
  array[0] = RegisterAddr;
  for (stringpos = 0; stringpos < NumByteToWrite; stringpos++) {
    array[stringpos + 1] = *(pBuffer + stringpos);
  }
  
  iError = MicoI2cBuildTxMessage(&lps25hb_i2c_msg, array, NumByteToWrite + 1, 3);
  iError = MicoI2cTransfer(&lps25hb_i2c_device, &lps25hb_i2c_msg, 1);
  if(0 != iError){
    iError = PRESSURE_ERROR;
  }
  
  return (PRESSURE_StatusTypeDef)iError;
}

/*	\Brief: The function is used as I2C bus read
*	\Return : Status of the I2C read
*	\param dev_addr : The device address of the sensor
*	\param reg_addr : Address of the first register, will data is going to be read
*	\param reg_data : This data read from the sensor, which is hold in an array
*	\param cnt : The no of byte of data to be read
*/
PRESSURE_StatusTypeDef LPS25HB_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint16_t NumByteToRead)
{
  mico_i2c_message_t lps25hb_i2c_msg = {NULL, NULL, 0, 0, 0, false};
  int iError = 0;
  uint8_t array[8] = {0};
  array[0] = RegisterAddr;
  
  iError = MicoI2cBuildCombinedMessage(&lps25hb_i2c_msg, array, pBuffer, 1, NumByteToRead, 3);
  if(0 != iError){
    return PRESSURE_ERROR; 
  }
  iError = MicoI2cTransfer(&lps25hb_i2c_device, &lps25hb_i2c_msg, 1);
  if(0 != iError){
    return PRESSURE_ERROR;
  }
  return (PRESSURE_StatusTypeDef)iError;
}

static PRESSURE_StatusTypeDef LPS25HB_PowerOn(void);
static PRESSURE_StatusTypeDef LPS25HB_I2C_ReadRawPressure(uint32_t *raw_press);
static PRESSURE_StatusTypeDef LPS25HB_I2C_ReadRawTemperature(int16_t *raw_data);



/**
 * @brief  Set LPS25HB Initialization
 * @param  LPS25HB_Init the configuration setting for the LPS25HB
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_Init(PRESSURE_InitTypeDef *LPS25HB_Init)
{  
    uint8_t tmp1 = 0x00;

    /* Configure the low level interface ---------------------------------------*/
    if(LPS25HB_IO_Init() != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    if(LPS25HB_PowerOn() != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    if(LPS25HB_IO_Read(&tmp1, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    /* Output Data Rate selection */
    tmp1 &= ~(LPS25HB_ODR_MASK);
    tmp1 |= LPS25HB_Init->OutputDataRate;

    /* Interrupt circuit selection */
    tmp1 &= ~(LPS25HB_DIFF_EN_MASK);
    tmp1 |= LPS25HB_Init->DiffEnable;

    /* Block Data Update selection */
    tmp1 &= ~(LPS25HB_BDU_MASK);
    tmp1 |= LPS25HB_Init->BlockDataUpdate;

    /* Serial Interface Mode selection */
    tmp1 &= ~(LPS25HB_SPI_SIM_MASK);
    tmp1 |= LPS25HB_Init->SPIMode;

    if(LPS25HB_IO_Write(&tmp1, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }      

    if(LPS25HB_IO_Read(&tmp1, LPS25HB_SlaveAddress, LPS25HB_RES_CONF_ADDR, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    /* Serial Interface Mode selection */
    tmp1 &= ~(LPS25HB_P_RES_MASK);
    tmp1 |= LPS25HB_Init->PressureResolution;

    /* Serial Interface Mode selection */
    tmp1 &= ~(LPS25HB_T_RES_MASK);
    tmp1 |= LPS25HB_Init->TemperatureResolution;

    if(LPS25HB_IO_Write(&tmp1, LPS25HB_SlaveAddress, LPS25HB_RES_CONF_ADDR, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }
    
    return PRESSURE_OK;
}

/**
 * @brief  Read ID address of LPS25HB
 * @param  ht_id the pointer where the ID of the device is stored
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_ReadID(uint8_t *p_id)
{
    if(!p_id)
    { 
      return PRESSURE_ERROR;
    }
 
    return LPS25HB_IO_Read(p_id, LPS25HB_SlaveAddress, LPS25HB_WHO_AM_I_ADDR, 1);
}

/**
 * @brief  Reboot memory content of LPS25HB
 * @param  None
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_RebootCmd(void)
{
    uint8_t tmpreg;

    /* Read CTRL_REG5 register */
    if(LPS25HB_IO_Read(&tmpreg, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG2_ADDR, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    /* Enable or Disable the reboot memory */
    tmpreg |= LPS25HB_RESET_MEMORY;

    /* Write value to MEMS CTRL_REG5 regsister */
    if(LPS25HB_IO_Write(&tmpreg, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG2_ADDR, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }
    
    return PRESSURE_OK;
}


/**
 * @brief  Read LPS25HB output register, and calculate the raw pressure
 * @param  raw_press the pressure raw value
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_I2C_ReadRawPressure(uint32_t *raw_press)
{
    uint8_t buffer[3], i;
    uint32_t tempVal=0;
    uint8_t tmp = 0x00;
    
    if(LPS25HB_IO_Read(&tmp, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    /* Output Data Rate selection */
    tmp &= (LPS25HB_ODR_MASK);
    
    if(tmp == 0x00)
    {
      if(LPS25HB_IO_Read(&tmp, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG2_ADDR, 1) != PRESSURE_OK)
      {
        return PRESSURE_ERROR;
      }

      /* Serial Interface Mode selection */
      tmp &= ~(LPS25HB_ONE_SHOT_MASK);
      tmp |= LPS25HB_ONE_SHOT_START;

      if(LPS25HB_IO_Write(&tmp, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG2_ADDR, 1) != PRESSURE_OK)
      {
        return PRESSURE_ERROR;
      }
    
      do{
      
        if(LPS25HB_IO_Read(&tmp, LPS25HB_SlaveAddress, LPS25HB_STATUS_REG_ADDR, 1) != PRESSURE_OK)
        {
          return PRESSURE_ERROR;
        }
       
      }while(!(tmp&&0x01));
    }    
    
    /* Read the register content */

    if(LPS25HB_IO_Read(buffer, LPS25HB_SlaveAddress, (LPS25HB_PRESS_POUT_XL_ADDR | LPS25HB_I2C_MULTIPLEBYTE_CMD), 3) != PRESSURE_OK)
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
 * @brief  Read LPS25HB output register, and calculate the pressure in mbar
 * @param  pfData the pressure value in mbar
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_GetPressure(float* pfData)
{
    uint32_t raw_press = 0;
    uint8_t tmp = 0x00;
    
    if(LPS25HB_IO_Read(&tmp, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    /* Output Data Rate selection */
    tmp &= (LPS25HB_ODR_MASK);
    
    if(tmp == 0x00)
    {
      if(LPS25HB_IO_Read(&tmp, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG2_ADDR, 1) != PRESSURE_OK)
      {
        return PRESSURE_ERROR;
      }

      /* Serial Interface Mode selection */
      tmp &= ~(LPS25HB_ONE_SHOT_MASK);
      tmp |= LPS25HB_ONE_SHOT_START;

      if(LPS25HB_IO_Write(&tmp, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG2_ADDR, 1) != PRESSURE_OK)
      {
        return PRESSURE_ERROR;
      }
    
      do{
      
        if(LPS25HB_IO_Read(&tmp, LPS25HB_SlaveAddress, LPS25HB_STATUS_REG_ADDR, 1) != PRESSURE_OK)
        {
          return PRESSURE_ERROR;
        }
       
      }while(!(tmp&&0x01));
    }    

    if(LPS25HB_I2C_ReadRawPressure(&raw_press) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    *pfData = (float)raw_press /4096.0f;
    
    return PRESSURE_OK;
}

/**
 * @brief  Read LPS25HB output register, and calculate the raw temperature
 * @param  raw_data the temperature raw value
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_I2C_ReadRawTemperature(int16_t *raw_data)
{
    uint8_t buffer[2];
    uint16_t tempVal=0;

    /* Read the register content */
    if(LPS25HB_IO_Read(&buffer[0], LPS25HB_SlaveAddress, LPS25HB_TEMP_OUT_L_ADDR, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    if(LPS25HB_IO_Read(&buffer[1], LPS25HB_SlaveAddress, LPS25HB_TEMP_OUT_H_ADDR, 1) != PRESSURE_OK)
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
 * @brief  Read LPS25HB output register, and calculate the temperature
 * @param  pfData the temperature value
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_GetTemperature(float *pfData)
{
    int16_t raw_data;

    if(LPS25HB_I2C_ReadRawTemperature(&raw_data) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    *pfData = (float)((((float)raw_data/480.0f) + 42.5f));
    
    return PRESSURE_OK;
}
/**
 * @brief  Exit the shutdown mode for LPS25HB
 * @param  None
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_PowerOn(void)
{
    uint8_t tmpreg;

    /* Read the register content */
    if(LPS25HB_IO_Read(&tmpreg, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    /* Set the power down bit */
    tmpreg |= LPS25HB_MODE_ACTIVE;

    /* Write register */
    if(LPS25HB_IO_Write(&tmpreg, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }
    
    return PRESSURE_OK;
}


/**
 * @brief  Enter the shutdown mode for LPS25HB
 * @param  None
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_PowerOff(void)
{
    uint8_t tmpreg;

    /* Read the register content */
    if(LPS25HB_IO_Read(&tmpreg, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }

    /* Reset the power down bit */
    tmpreg &= ~(LPS25HB_MODE_ACTIVE);

    /* Write register */
    if(LPS25HB_IO_Write(&tmpreg, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
    {
      return PRESSURE_ERROR;
    }
    
    return PRESSURE_OK;
}

/**
 * @brief  Set the slave address according to SA0 bit
 * @param  SA0_Bit_Status LPS25HB_SA0_LOW or LPS25HB_SA0_HIGH
 * @retval None
 */
static void LPS25HB_SlaveAddrRemap(uint8_t SA0_Bit_Status)
{
    LPS25HB_SlaveAddress = (SA0_Bit_Status==LPS25HB_SA0_LOW?LPS25HB_ADDRESS_LOW:LPS25HB_ADDRESS_HIGH);
}



OSStatus lps25hb_sensor_init(void)
{
  PRESSURE_InitTypeDef lps25hb;
  lps25hb.OutputDataRate = LPS25HB_ODR_ONE_SHOT;
  lps25hb.PressureResolution = LPS25HB_P_RES_AVG_32;
  lps25hb.TemperatureResolution = LPS25HB_T_RES_AVG_32;
  lps25hb.DiffEnable = LPS25HB_DIFF_ENABLE;
  lps25hb.BlockDataUpdate = LPS25HB_BDU_READ;
  lps25hb.SPIMode = LPS25HB_SPI_SIM_4W;
     
  if(LPS25HB_Init(&lps25hb) != PRESSURE_OK){
    return -1;
  }
  return 0;
}

OSStatus lps25hb_Read_Data(float *temperature,float *pressure)
{
  if(LPS25HB_GetTemperature(temperature) != PRESSURE_OK){
    return -1;
  }
  if(LPS25HB_GetPressure(pressure) != PRESSURE_OK){
    return -1;
  }
  return 0;
}

OSStatus lps25hb_sensor_deinit(void)
{
  if(LPS25HB_PowerOff() != PRESSURE_OK){
    return -1;
  }
  if(MicoI2cFinalize(&lps25hb_i2c_device) != PRESSURE_OK){
    return -1;
  }
  return 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/     

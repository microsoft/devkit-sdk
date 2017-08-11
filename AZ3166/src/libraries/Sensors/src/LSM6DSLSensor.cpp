/**
 ******************************************************************************
 * @file    LSM6DSLSensor.cpp
 * @author  CLab
 * @version V1.0.0
 * @date    5 August 2016
 * @brief   Implementation of an LSM6DSL Inertial Measurement Unit (IMU) 6 axes
 *          sensor.
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
#include "LSM6DSLSensor.h"
#include "utility/LSM6DSL_acc_gyro_driver.h"


/* Class Implementation ------------------------------------------------------*/

/** Constructor
 * @param i2c object of an helper class which handles the I2C peripheral
 * @param address the address of the component's instance
 */
LSM6DSLSensor::LSM6DSLSensor(DevI2C &i2c, PinName int1_pin, PinName int2_pin) : _dev_i2c(i2c), _int1_irq(int1_pin), _int2_irq(int2_pin)
{
  _address = LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW; 
};

/** Constructor
 * @param i2c object of an helper class which handles the I2C peripheral
 * @param address the address of the component's instance
 */
LSM6DSLSensor::LSM6DSLSensor(DevI2C &i2c, PinName int1_pin, PinName int2_pin, uint8_t address) : _dev_i2c(i2c), _int1_irq(int1_pin), _int2_irq(int2_pin), _address(address)
{

};

/**
 * @brief     Initializing the component.
 * @param[in] init pointer to device specific initialization structure.
 * @retval    "0" in case of success, an error code otherwise.
 */
int LSM6DSLSensor::init(void *init)
{
  /* Enable register address automatically incremented during a multiple byte
     access with a serial interface. */
  if ( LSM6DSL_ACC_GYRO_W_IF_Addr_Incr( (void *)this, LSM6DSL_ACC_GYRO_IF_INC_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Enable BDU */
  if ( LSM6DSL_ACC_GYRO_W_BDU( (void *)this, LSM6DSL_ACC_GYRO_BDU_BLOCK_UPDATE ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* FIFO mode selection */
  if ( LSM6DSL_ACC_GYRO_W_FIFO_MODE( (void *)this, LSM6DSL_ACC_GYRO_FIFO_MODE_BYPASS ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Output data rate selection - power down. */
  if ( LSM6DSL_ACC_GYRO_W_ODR_XL( (void *)this, LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Full scale selection. */
  if ( setXFullScale( 2.0f ) == 1 )
  {
    return 1;
  }

  /* Output data rate selection - power down */
  if ( LSM6DSL_ACC_GYRO_W_ODR_G( (void *)this, LSM6DSL_ACC_GYRO_ODR_G_POWER_DOWN ) == MEMS_ERROR )
  {
    return 1;
  }

  /* Full scale selection. */
  if ( setGFullScale( 2000.0f ) == 1 )
  {
    return 1;
  }

  _x_last_odr = 104.0f;

  _x_is_enabled = 0;
  
  _g_last_odr = 104.0f;

  _g_is_enabled = 0;

  return 0;
}

/**
 * @brief  Enable LSM6DSL Accelerator
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::enableAccelerator(void)
{ 
  /* Check if the component is already enabled */
  if ( _x_is_enabled == 1 )
  {
    return 0;
  }
  
  /* Output data rate selection. */
  if ( setXOdrWhenEnabled( _x_last_odr ) == 1 )
  {
    return 1;
  }
  
  _x_is_enabled = 1;
  
  return 0;
}

/**
 * @brief  Enable LSM6DSL Gyroscope
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::enableGyroscope(void)
{ 
  /* Check if the component is already enabled */
  if ( _g_is_enabled == 1 )
  {
    return 0;
  }
  
  /* Output data rate selection. */
  if ( setGOdrWhenEnabled( _g_last_odr ) == 1 )
  {
    return 1;
  }
  
  _g_is_enabled = 1;
  
  return 0;
}

/**
 * @brief  Disable LSM6DSL Accelerator
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::disableAccelerator(void)
{ 
  /* Check if the component is already disabled */
  if ( _x_is_enabled == 0 )
  {
    return 0;
  }
  
  /* Store actual output data rate. */
  if ( getXOdr( &_x_last_odr ) == 1 )
  {
    return 1;
  }
  
  /* Output data rate selection - power down. */
  if ( LSM6DSL_ACC_GYRO_W_ODR_XL( (void *)this, LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN ) == MEMS_ERROR )
  {
    return 1;
  }
  
  _x_is_enabled = 0;
  
  return 0;
}

/**
 * @brief  Disable LSM6DSL Gyroscope
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::disableGyroscope(void)
{ 
  /* Check if the component is already disabled */
  if ( _g_is_enabled == 0 )
  {
    return 0;
  }
  
  /* Store actual output data rate. */
  if ( getGOdr( &_g_last_odr ) == 1 )
  {
    return 1;
  }
  
  /* Output data rate selection - power down */
  if ( LSM6DSL_ACC_GYRO_W_ODR_G( (void *)this, LSM6DSL_ACC_GYRO_ODR_G_POWER_DOWN ) == MEMS_ERROR )
  {
    return 1;
  }
  
  _g_is_enabled = 0;
  
  return 0;
}

/**
 * @brief  Read ID of LSM6DSL Accelerometer and Gyroscope
 * @param  id the pointer where the ID of the device is stored
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::readId(unsigned char *id)
{
  if(!id)
  { 
    return 1;
  }

  /* Read WHO AM I register */
  if ( LSM6DSL_ACC_GYRO_R_WHO_AM_I( (void *)this, (u8_t *)id) == MEMS_ERROR )
  {
    return 1;
  }

  return 0;
}

/**
 * @brief  Read data from LSM6DSL Accelerometer
 * @param  pData the pointer where the accelerometer data are stored
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::getXAxes(int *pData)
{
  int16_t dataRaw[3];
  float sensitivity = 0;
  
  /* Read raw data from LSM6DSL output register. */
  if ( getXAxesRaw( dataRaw ) == 1 )
  {
    return 1;
  }
  
  /* Get LSM6DSL actual sensitivity. */
  if ( getXSensitivity( &sensitivity ) == 1 )
  {
    return 1;
  }
  
  /* Calculate the data. */
  pData[0] = ( int )( dataRaw[0] * sensitivity );
  pData[1] = ( int )( dataRaw[1] * sensitivity );
  pData[2] = ( int )( dataRaw[2] * sensitivity );
  
  return 0;
}

/**
 * @brief  Read data from LSM6DSL Gyroscope
 * @param  pData the pointer where the gyroscope data are stored
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::getGAxes(int *pData)
{
  int16_t dataRaw[3];
  float sensitivity = 0;
  
  /* Read raw data from LSM6DSL output register. */
  if ( getGAxesRaw( dataRaw ) == 1 )
  {
    return 1;
  }
  
  /* Get LSM6DSL actual sensitivity. */
  if ( getGSensitivity( &sensitivity ) == 1 )
  {
    return 1;
  }
  
  /* Calculate the data. */
  pData[0] = ( int32_t )( dataRaw[0] * sensitivity );
  pData[1] = ( int32_t )( dataRaw[1] * sensitivity );
  pData[2] = ( int32_t )( dataRaw[2] * sensitivity );
  
  return 0;
}

/**
 * @brief  Read Accelerometer Sensitivity
 * @param  pfData the pointer where the accelerometer sensitivity is stored
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::getXSensitivity(float *pfData)
{
  LSM6DSL_ACC_GYRO_FS_XL_t fullScale;
  
  /* Read actual full scale selection from sensor. */
  if ( LSM6DSL_ACC_GYRO_R_FS_XL( (void *)this, &fullScale ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Store the sensitivity based on actual full scale. */
  switch( fullScale )
  {
    case LSM6DSL_ACC_GYRO_FS_XL_2g:
      *pfData = ( float )LSM6DSL_ACC_SENSITIVITY_FOR_FS_2G;
      break;
    case LSM6DSL_ACC_GYRO_FS_XL_4g:
      *pfData = ( float )LSM6DSL_ACC_SENSITIVITY_FOR_FS_4G;
      break;
    case LSM6DSL_ACC_GYRO_FS_XL_8g:
      *pfData = ( float )LSM6DSL_ACC_SENSITIVITY_FOR_FS_8G;
      break;
    case LSM6DSL_ACC_GYRO_FS_XL_16g:
      *pfData = ( float )LSM6DSL_ACC_SENSITIVITY_FOR_FS_16G;
      break;
    default:
      *pfData = -1.0f;
      return 1;
  }
  
  return 0;
}

/**
 * @brief  Read Gyroscope Sensitivity
 * @param  pfData the pointer where the gyroscope sensitivity is stored
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::getGSensitivity(float *pfData)
{
  LSM6DSL_ACC_GYRO_FS_125_t fullScale125;
  LSM6DSL_ACC_GYRO_FS_G_t   fullScale;
  
  /* Read full scale 125 selection from sensor. */
  if ( LSM6DSL_ACC_GYRO_R_FS_125( (void *)this, &fullScale125 ) == MEMS_ERROR )
  {
    return 1;
  }
  
  if ( fullScale125 == LSM6DSL_ACC_GYRO_FS_125_ENABLED )
  {
    *pfData = ( float )LSM6DSL_GYRO_SENSITIVITY_FOR_FS_125DPS;
  }
  
  else
  {
  
    /* Read actual full scale selection from sensor. */
    if ( LSM6DSL_ACC_GYRO_R_FS_G( (void *)this, &fullScale ) == MEMS_ERROR )
    {
      return 1;
    }
    
    /* Store the sensitivity based on actual full scale. */
    switch( fullScale )
    {
      case LSM6DSL_ACC_GYRO_FS_G_245dps:
        *pfData = ( float )LSM6DSL_GYRO_SENSITIVITY_FOR_FS_245DPS;
        break;
      case LSM6DSL_ACC_GYRO_FS_G_500dps:
        *pfData = ( float )LSM6DSL_GYRO_SENSITIVITY_FOR_FS_500DPS;
        break;
      case LSM6DSL_ACC_GYRO_FS_G_1000dps:
        *pfData = ( float )LSM6DSL_GYRO_SENSITIVITY_FOR_FS_1000DPS;
        break;
      case LSM6DSL_ACC_GYRO_FS_G_2000dps:
        *pfData = ( float )LSM6DSL_GYRO_SENSITIVITY_FOR_FS_2000DPS;
        break;
      default:
        *pfData = -1.0f;
        return 1;
    }
  }
  
  return 0;
}

/**
 * @brief  Read raw data from LSM6DSL Accelerometer
 * @param  pData the pointer where the accelerometer raw data are stored
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::getXAxesRaw(int16_t *pData)
{
  uint8_t regValue[6] = {0, 0, 0, 0, 0, 0};
  
  /* Read output registers from LSM6DSL_ACC_GYRO_OUTX_L_XL to LSM6DSL_ACC_GYRO_OUTZ_H_XL. */
  if ( LSM6DSL_ACC_GYRO_GetRawAccData( (void *)this, regValue ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Format the data. */
  pData[0] = ( ( ( ( int16_t )regValue[1] ) << 8 ) + ( int16_t )regValue[0] );
  pData[1] = ( ( ( ( int16_t )regValue[3] ) << 8 ) + ( int16_t )regValue[2] );
  pData[2] = ( ( ( ( int16_t )regValue[5] ) << 8 ) + ( int16_t )regValue[4] );
  
  return 0;
}

/**
 * @brief  Read raw data from LSM6DSL Gyroscope
 * @param  pData the pointer where the gyroscope raw data are stored
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::getGAxesRaw(int16_t *pData)
{
  uint8_t regValue[6] = {0, 0, 0, 0, 0, 0};
  
  /* Read output registers from LSM6DSL_ACC_GYRO_OUTX_L_G to LSM6DSL_ACC_GYRO_OUTZ_H_G. */
  if ( LSM6DSL_ACC_GYRO_GetRawGyroData( (void *)this, regValue ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Format the data. */
  pData[0] = ( ( ( ( int16_t )regValue[1] ) << 8 ) + ( int16_t )regValue[0] );
  pData[1] = ( ( ( ( int16_t )regValue[3] ) << 8 ) + ( int16_t )regValue[2] );
  pData[2] = ( ( ( ( int16_t )regValue[5] ) << 8 ) + ( int16_t )regValue[4] );
  
  return 0;
}

/**
 * @brief  Read LSM6DSL Accelerometer output data rate
 * @param  odr the pointer to the output data rate
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::getXOdr(float* odr)
{
  LSM6DSL_ACC_GYRO_ODR_XL_t odr_low_level;
  
  if ( LSM6DSL_ACC_GYRO_R_ODR_XL( (void *)this, &odr_low_level ) == MEMS_ERROR )
  {
    return 1;
  }
  
  switch( odr_low_level )
  {
    case LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN:
      *odr = 0.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_13Hz:
      *odr = 13.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_26Hz:
      *odr = 26.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_52Hz:
      *odr = 52.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_104Hz:
      *odr = 104.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_208Hz:
      *odr = 208.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_416Hz:
      *odr = 416.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_833Hz:
      *odr = 833.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_1660Hz:
      *odr = 1660.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_3330Hz:
      *odr = 3330.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_XL_6660Hz:
      *odr = 6660.0f;
      break;
    default:
      *odr = -1.0f;
      return 1;
  }
  
  return 0;
}

/**
 * @brief  Read LSM6DSL Gyroscope output data rate
 * @param  odr the pointer to the output data rate
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::getGOdr(float* odr)
{
  LSM6DSL_ACC_GYRO_ODR_G_t odr_low_level;
  
  if ( LSM6DSL_ACC_GYRO_R_ODR_G( (void *)this, &odr_low_level ) == MEMS_ERROR )
  {
    return 1;
  }
  
  switch( odr_low_level )
  {
    case LSM6DSL_ACC_GYRO_ODR_G_POWER_DOWN:
      *odr = 0.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_13Hz:
      *odr = 13.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_26Hz:
      *odr = 26.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_52Hz:
      *odr = 52.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_104Hz:
      *odr = 104.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_208Hz:
      *odr = 208.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_416Hz:
      *odr = 416.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_833Hz:
      *odr = 833.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_1660Hz:
      *odr = 1660.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_3330Hz:
      *odr = 3330.0f;
      break;
    case LSM6DSL_ACC_GYRO_ODR_G_6660Hz:
      *odr = 6660.0f;
      break;
    default:
      *odr = -1.0f;
      return 1;
  }
  
  return 0;
}

/**
 * @brief  Set LSM6DSL Accelerometer output data rate
 * @param  odr the output data rate to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::setXOdr(float odr)
{
  if(_x_is_enabled == 1)
  {
    if(setXOdrWhenEnabled(odr) == 1)
    {
      return 1;
    }
  }
  else
  {
    if(setXOdrWhenDisabled(odr) == 1)
    {
      return 1;
    }
  }
  
  return 0;
}

/**
 * @brief  Set LSM6DSL Accelerometer output data rate when enabled
 * @param  odr the output data rate to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::setXOdrWhenEnabled(float odr)
{
  LSM6DSL_ACC_GYRO_ODR_XL_t new_odr;
  
  new_odr = ( odr <=   13.0f ) ? LSM6DSL_ACC_GYRO_ODR_XL_13Hz
          : ( odr <=   26.0f ) ? LSM6DSL_ACC_GYRO_ODR_XL_26Hz
          : ( odr <=   52.0f ) ? LSM6DSL_ACC_GYRO_ODR_XL_52Hz
          : ( odr <=  104.0f ) ? LSM6DSL_ACC_GYRO_ODR_XL_104Hz
          : ( odr <=  208.0f ) ? LSM6DSL_ACC_GYRO_ODR_XL_208Hz
          : ( odr <=  416.0f ) ? LSM6DSL_ACC_GYRO_ODR_XL_416Hz
          : ( odr <=  833.0f ) ? LSM6DSL_ACC_GYRO_ODR_XL_833Hz
          : ( odr <= 1660.0f ) ? LSM6DSL_ACC_GYRO_ODR_XL_1660Hz
          : ( odr <= 3330.0f ) ? LSM6DSL_ACC_GYRO_ODR_XL_3330Hz
          :                      LSM6DSL_ACC_GYRO_ODR_XL_6660Hz;
            
  if ( LSM6DSL_ACC_GYRO_W_ODR_XL( (void *)this, new_odr ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief  Set LSM6DSL Accelerometer output data rate when disabled
 * @param  odr the output data rate to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::setXOdrWhenDisabled(float odr)
{ 
  _x_last_odr = ( odr <=   13.0f ) ? 13.0f
             : ( odr <=   26.0f ) ? 26.0f
             : ( odr <=   52.0f ) ? 52.0f
             : ( odr <=  104.0f ) ? 104.0f
             : ( odr <=  208.0f ) ? 208.0f
             : ( odr <=  416.0f ) ? 416.0f
             : ( odr <=  833.0f ) ? 833.0f
             : ( odr <= 1660.0f ) ? 1660.0f
             : ( odr <= 3330.0f ) ? 3330.0f
             :                      6660.0f;
                                 
  return 0;
}

/**
 * @brief  Set LSM6DSL Gyroscope output data rate
 * @param  odr the output data rate to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::setGOdr(float odr)
{
  if(_g_is_enabled == 1)
  {
    if(setGOdrWhenEnabled(odr) == 1)
    {
      return 1;
    }
  }
  else
  {
    if(setGOdrWhenDisabled(odr) == 1)
    {
      return 1;
    }
  }
  
  return 0;
}

/**
 * @brief  Set LSM6DSL Gyroscope output data rate when enabled
 * @param  odr the output data rate to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::setGOdrWhenEnabled(float odr)
{
  LSM6DSL_ACC_GYRO_ODR_G_t new_odr;
  
  new_odr = ( odr <=  13.0f )  ? LSM6DSL_ACC_GYRO_ODR_G_13Hz
          : ( odr <=  26.0f )  ? LSM6DSL_ACC_GYRO_ODR_G_26Hz
          : ( odr <=  52.0f )  ? LSM6DSL_ACC_GYRO_ODR_G_52Hz
          : ( odr <= 104.0f )  ? LSM6DSL_ACC_GYRO_ODR_G_104Hz
          : ( odr <= 208.0f )  ? LSM6DSL_ACC_GYRO_ODR_G_208Hz
          : ( odr <= 416.0f )  ? LSM6DSL_ACC_GYRO_ODR_G_416Hz
          : ( odr <= 833.0f )  ? LSM6DSL_ACC_GYRO_ODR_G_833Hz
          : ( odr <= 1660.0f ) ? LSM6DSL_ACC_GYRO_ODR_G_1660Hz
          : ( odr <= 3330.0f ) ? LSM6DSL_ACC_GYRO_ODR_G_3330Hz
          :                      LSM6DSL_ACC_GYRO_ODR_G_6660Hz;
            
  if ( LSM6DSL_ACC_GYRO_W_ODR_G( (void *)this, new_odr ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief  Set LSM6DSL Gyroscope output data rate when disabled
 * @param  odr the output data rate to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::setGOdrWhenDisabled(float odr)
{
  _g_last_odr = ( odr <=  13.0f )  ? 13.0f
             : ( odr <=  26.0f )  ? 26.0f
             : ( odr <=  52.0f )  ? 52.0f
             : ( odr <= 104.0f )  ? 104.0f
             : ( odr <= 208.0f )  ? 208.0f
             : ( odr <= 416.0f )  ? 416.0f
             : ( odr <= 833.0f )  ? 833.0f
             : ( odr <= 1660.0f ) ? 1660.0f
             : ( odr <= 3330.0f ) ? 3330.0f
             :                      6660.0f;
                                 
  return 0;
}

/**
 * @brief  Read LSM6DSL Accelerometer full scale
 * @param  fullScale the pointer to the full scale
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::getXFullScale(float* fullScale)
{
  LSM6DSL_ACC_GYRO_FS_XL_t fs_low_level;
  
  if ( LSM6DSL_ACC_GYRO_R_FS_XL( (void *)this, &fs_low_level ) == MEMS_ERROR )
  {
    return 1;
  }
  
  switch( fs_low_level )
  {
    case LSM6DSL_ACC_GYRO_FS_XL_2g:
      *fullScale = 2.0f;
      break;
    case LSM6DSL_ACC_GYRO_FS_XL_4g:
      *fullScale = 4.0f;
      break;
    case LSM6DSL_ACC_GYRO_FS_XL_8g:
      *fullScale = 8.0f;
      break;
    case LSM6DSL_ACC_GYRO_FS_XL_16g:
      *fullScale = 16.0f;
      break;
    default:
      *fullScale = -1.0f;
      return 1;
  }
  
  return 0;
}

/**
 * @brief  Read LSM6DSL Gyroscope full scale
 * @param  fullScale the pointer to the full scale
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::getGFullScale(float* fullScale)
{
  LSM6DSL_ACC_GYRO_FS_G_t fs_low_level;
  LSM6DSL_ACC_GYRO_FS_125_t fs_125;
  
  if ( LSM6DSL_ACC_GYRO_R_FS_125( (void *)this, &fs_125 ) == MEMS_ERROR )
  {
    return 1;
  }
  if ( LSM6DSL_ACC_GYRO_R_FS_G( (void *)this, &fs_low_level ) == MEMS_ERROR )
  {
    return 1;
  }
  
  if ( fs_125 == LSM6DSL_ACC_GYRO_FS_125_ENABLED )
  {
    *fullScale = 125.0f;
  }
  
  else
  {
    switch( fs_low_level )
    {
      case LSM6DSL_ACC_GYRO_FS_G_245dps:
        *fullScale = 245.0f;
        break;
      case LSM6DSL_ACC_GYRO_FS_G_500dps:
        *fullScale = 500.0f;
        break;
      case LSM6DSL_ACC_GYRO_FS_G_1000dps:
        *fullScale = 1000.0f;
        break;
      case LSM6DSL_ACC_GYRO_FS_G_2000dps:
        *fullScale = 2000.0f;
        break;
      default:
        *fullScale = -1.0f;
        return 1;
    }
  }
  
  return 0;
}

/**
 * @brief  Set LSM6DSL Accelerometer full scale
 * @param  fullScale the full scale to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::setXFullScale(float fullScale)
{
  LSM6DSL_ACC_GYRO_FS_XL_t new_fs;
  
  new_fs = ( fullScale <= 2.0f ) ? LSM6DSL_ACC_GYRO_FS_XL_2g
         : ( fullScale <= 4.0f ) ? LSM6DSL_ACC_GYRO_FS_XL_4g
         : ( fullScale <= 8.0f ) ? LSM6DSL_ACC_GYRO_FS_XL_8g
         :                         LSM6DSL_ACC_GYRO_FS_XL_16g;
           
  if ( LSM6DSL_ACC_GYRO_W_FS_XL( (void *)this, new_fs ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief  Set LSM6DSL Gyroscope full scale
 * @param  fullScale the full scale to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::setGFullScale(float fullScale)
{
  LSM6DSL_ACC_GYRO_FS_G_t new_fs;
  
  if ( fullScale <= 125.0f )
  {
    if ( LSM6DSL_ACC_GYRO_W_FS_125( (void *)this, LSM6DSL_ACC_GYRO_FS_125_ENABLED ) == MEMS_ERROR )
    {
      return 1;
    }
  }
  else
  {
    new_fs = ( fullScale <=  245.0f ) ? LSM6DSL_ACC_GYRO_FS_G_245dps
           : ( fullScale <=  500.0f ) ? LSM6DSL_ACC_GYRO_FS_G_500dps
           : ( fullScale <= 1000.0f ) ? LSM6DSL_ACC_GYRO_FS_G_1000dps
           :                            LSM6DSL_ACC_GYRO_FS_G_2000dps;
             
    if ( LSM6DSL_ACC_GYRO_W_FS_125( (void *)this, LSM6DSL_ACC_GYRO_FS_125_DISABLED ) == MEMS_ERROR )
    {
      return 1;
    }
    if ( LSM6DSL_ACC_GYRO_W_FS_G( (void *)this, new_fs ) == MEMS_ERROR )
    {
      return 1;
    }
  }
  
  return 0;
}

/**
 * @brief  Enable free fall detection
 * @param pin the interrupt pin to be used
 * @note  This function sets the LSM6DSL accelerometer ODR to 416Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval 0 in case of success, an error code otherwise
*/
int LSM6DSLSensor::enableFreeFallDetection(LSM6DSL_Interrupt_Pin_t pin)
{
  /* Output Data Rate selection */
  if(setXOdr(416.0f) == 1)
  {
    return 1;
  }
  
  /* Full scale selection */
  if ( LSM6DSL_ACC_GYRO_W_FS_XL( (void *)this, LSM6DSL_ACC_GYRO_FS_XL_2g ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* FF_DUR setting */
  if ( LSM6DSL_ACC_GYRO_W_FF_Duration( (void *)this, 0x06 ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* WAKE_DUR setting */
  if ( LSM6DSL_ACC_GYRO_W_WAKE_DUR( (void *)this, 0x00 ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* TIMER_HR setting */
  if ( LSM6DSL_ACC_GYRO_W_TIMER_HR( (void *)this, LSM6DSL_ACC_GYRO_TIMER_HR_6_4ms ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* SLEEP_DUR setting */
  if ( LSM6DSL_ACC_GYRO_W_SLEEP_DUR( (void *)this, 0x00 ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* FF_THS setting */
  if ( LSM6DSL_ACC_GYRO_W_FF_THS( (void *)this, LSM6DSL_ACC_GYRO_FF_THS_312mg ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Enable basic Interrupts */
  if ( LSM6DSL_ACC_GYRO_W_BASIC_INT( (void *)this, LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Enable free fall event on either INT1 or INT2 pin */
  switch (pin)
  {
  case LSM6DSL_INT1_PIN:
    if ( LSM6DSL_ACC_GYRO_W_FFEvOnInt1( (void *)this, LSM6DSL_ACC_GYRO_INT1_FF_ENABLED ) == MEMS_ERROR )
    {
      return 1;
    }
    break;

  case LSM6DSL_INT2_PIN:
    if ( LSM6DSL_ACC_GYRO_W_FFEvOnInt2( (void *)this, LSM6DSL_ACC_GYRO_INT2_FF_ENABLED ) == MEMS_ERROR )
    {
      return 1;
    }
    break;

  default:
    return 1;
  }
  
  return 0;
}

/**
 * @brief  Disable free fall detection
 * @param  None
 * @retval 0 in case of success, an error code otherwise
*/
int LSM6DSLSensor::disableFreeFallDetection(void)
{
  /* Disable free fall event on INT1 pin */
  if ( LSM6DSL_ACC_GYRO_W_FFEvOnInt1( (void *)this, LSM6DSL_ACC_GYRO_INT1_FF_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable free fall event on INT2 pin */
  if ( LSM6DSL_ACC_GYRO_W_FFEvOnInt2( (void *)this, LSM6DSL_ACC_GYRO_INT2_FF_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable basic Interrupts */
  if ( LSM6DSL_ACC_GYRO_W_BASIC_INT( (void *)this, LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* FF_DUR setting */
  if ( LSM6DSL_ACC_GYRO_W_FF_Duration( (void *)this, 0x00 ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* FF_THS setting */
  if ( LSM6DSL_ACC_GYRO_W_FF_THS( (void *)this, LSM6DSL_ACC_GYRO_FF_THS_156mg ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief Set the free fall detection threshold for LSM6DSL accelerometer sensor
 * @param thr the threshold to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::setFreeFallThreshold(int thr)
{

  if ( LSM6DSL_ACC_GYRO_W_FF_THS( (void *)this, (LSM6DSL_ACC_GYRO_FF_THS_t)thr ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief Enable the pedometer feature for LSM6DSL accelerometer sensor
 * @note  This function sets the LSM6DSL accelerometer ODR to 26Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::enablePedometer(void)
{
  /* Output Data Rate selection */
  if( setXOdr(26.0f) == 1 )
  {
    return 1;
  }
  
  /* Full scale selection. */
  if( setXFullScale(2.0f) == 1 )
  {
    return 1;
  }
  
  /* Set pedometer threshold. */
  if ( setPedometerThreshold(LSM6DSL_PEDOMETER_THRESHOLD_MID_HIGH) == 1 )
  {
    return 1;
  }
  
  /* Enable embedded functionalities. */
  if ( LSM6DSL_ACC_GYRO_W_FUNC_EN( (void *)this, LSM6DSL_ACC_GYRO_FUNC_EN_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Enable pedometer algorithm. */
  if ( LSM6DSL_ACC_GYRO_W_PEDO( (void *)this, LSM6DSL_ACC_GYRO_PEDO_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Enable pedometer on INT1. */
  if ( LSM6DSL_ACC_GYRO_W_STEP_DET_on_INT1( (void *)this, LSM6DSL_ACC_GYRO_INT1_PEDO_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief Disable the pedometer feature for LSM6DSL accelerometer sensor
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::disablePedometer(void)
{
  /* Disable pedometer on INT1. */
  if ( LSM6DSL_ACC_GYRO_W_STEP_DET_on_INT1( (void *)this, LSM6DSL_ACC_GYRO_INT1_PEDO_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable pedometer algorithm. */
  if ( LSM6DSL_ACC_GYRO_W_PEDO( (void *)this, LSM6DSL_ACC_GYRO_PEDO_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable embedded functionalities. */
  if ( LSM6DSL_ACC_GYRO_W_FUNC_EN( (void *)this, LSM6DSL_ACC_GYRO_FUNC_EN_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Reset pedometer threshold. */
  if ( setPedometerThreshold(0x0) == 1 )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief Get the step counter for LSM6DSL accelerometer sensor
 * @param step_count the pointer to the step counter
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::getStepCounter(int *step_count)
{
  if ( LSM6DSL_ACC_GYRO_Get_GetStepCounter( (void *)this, ( uint8_t* )step_count ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief Reset of the step counter for LSM6DSL accelerometer sensor
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::resetStepCounter(void)
{
  if ( LSM6DSL_ACC_GYRO_W_PedoStepReset( (void *)this, LSM6DSL_ACC_GYRO_PEDO_RST_STEP_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  wait_ms(10);
  
  if ( LSM6DSL_ACC_GYRO_W_PedoStepReset( (void *)this, LSM6DSL_ACC_GYRO_PEDO_RST_STEP_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief Set the pedometer threshold for LSM6DSL accelerometer sensor
 * @param thr the threshold to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::setPedometerThreshold(unsigned char thr)
{
  if ( LSM6DSL_ACC_GYRO_W_PedoThreshold( (void *)this, thr ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief Enable the tilt detection for LSM6DSL accelerometer sensor
 * @param pin the interrupt pin to be used
 * @note  This function sets the LSM6DSL accelerometer ODR to 26Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::enableTiltDetection(LSM6DSL_Interrupt_Pin_t pin)
{
  /* Output Data Rate selection */
  if( setXOdr(26.0f) == 1 )
  {
    return 1;
  }
  
  /* Full scale selection. */
  if( setXFullScale(2.0f) == 1 )
  {
    return 1;
  }
  
  /* Enable embedded functionalities */
  if ( LSM6DSL_ACC_GYRO_W_FUNC_EN( (void *)this, LSM6DSL_ACC_GYRO_FUNC_EN_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Enable tilt calculation. */
  if ( LSM6DSL_ACC_GYRO_W_TILT( (void *)this, LSM6DSL_ACC_GYRO_TILT_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }

  /* Enable tilt detection on either INT1 or INT2 pin */
  switch (pin)
  {
  case LSM6DSL_INT1_PIN:
    if ( LSM6DSL_ACC_GYRO_W_TiltEvOnInt1( (void *)this, LSM6DSL_ACC_GYRO_INT1_TILT_ENABLED ) == MEMS_ERROR )
    {
      return 1;
    }
    break;

  case LSM6DSL_INT2_PIN:
    if ( LSM6DSL_ACC_GYRO_W_TiltEvOnInt2( (void *)this, LSM6DSL_ACC_GYRO_INT2_TILT_ENABLED ) == MEMS_ERROR )
    {
      return 1;
    }
    break;

  default:
    return 1;
  }

  return 0;
}

/**
 * @brief Disable the tilt detection for LSM6DSL accelerometer sensor
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::disableTiltDetection(void)
{
  /* Disable tilt event on INT1. */
  if ( LSM6DSL_ACC_GYRO_W_TiltEvOnInt1( (void *)this, LSM6DSL_ACC_GYRO_INT1_TILT_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }

  /* Disable tilt event on INT2. */
  if ( LSM6DSL_ACC_GYRO_W_TiltEvOnInt2( (void *)this, LSM6DSL_ACC_GYRO_INT2_TILT_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable tilt calculation. */
  if ( LSM6DSL_ACC_GYRO_W_TILT( (void *)this, LSM6DSL_ACC_GYRO_TILT_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable embedded functionalities */
  if ( LSM6DSL_ACC_GYRO_W_FUNC_EN( (void *)this, LSM6DSL_ACC_GYRO_FUNC_EN_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief Enable the wake up detection for LSM6DSL accelerometer sensor
 * @param pin the interrupt pin to be used
 * @note  This function sets the LSM6DSL accelerometer ODR to 416Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::enableWakeUpDetection(LSM6DSL_Interrupt_Pin_t pin)
{
  /* Output Data Rate selection */
  if( setXOdr(416.0f) == 1 )
  {
    return 1;
  }
  
  /* Full scale selection. */
  if( setXFullScale(2.0f) == 1 )
  {
    return 1;
  }
  
  /* WAKE_DUR setting */
  if ( LSM6DSL_ACC_GYRO_W_WAKE_DUR( (void *)this, 0x00 ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Set wake up threshold. */
  if ( LSM6DSL_ACC_GYRO_W_WK_THS( (void *)this, 0x02 ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Enable basic Interrupts */
  if ( LSM6DSL_ACC_GYRO_W_BASIC_INT( (void *)this, LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }

  /* Enable wake up detection on either INT1 or INT2 pin */
  switch (pin)
  {
  case LSM6DSL_INT1_PIN:
    if ( LSM6DSL_ACC_GYRO_W_WUEvOnInt1( (void *)this, LSM6DSL_ACC_GYRO_INT1_WU_ENABLED ) == MEMS_ERROR )
    {
      return 1;
    }
    break;

  case LSM6DSL_INT2_PIN:
    if ( LSM6DSL_ACC_GYRO_W_WUEvOnInt2( (void *)this, LSM6DSL_ACC_GYRO_INT2_WU_ENABLED ) == MEMS_ERROR )
    {
      return 1;
    }
    break;

  default:
    return 1;
  }
  
  return 0;
}

/**
 * @brief Disable the wake up detection for LSM6DSL accelerometer sensor
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::disableWakeUpDetection(void)
{
  /* Disable wake up event on INT1 */
  if ( LSM6DSL_ACC_GYRO_W_WUEvOnInt1( (void *)this, LSM6DSL_ACC_GYRO_INT1_WU_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }

  /* Disable wake up event on INT2 */
  if ( LSM6DSL_ACC_GYRO_W_WUEvOnInt2( (void *)this, LSM6DSL_ACC_GYRO_INT2_WU_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable basic Interrupts */
  if ( LSM6DSL_ACC_GYRO_W_BASIC_INT( (void *)this, LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* WU_DUR setting */
  if ( LSM6DSL_ACC_GYRO_W_WAKE_DUR( (void *)this, 0x00 ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* WU_THS setting */
  if ( LSM6DSL_ACC_GYRO_W_WK_THS( (void *)this, 0x00 ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief Set the wake up threshold for LSM6DSL accelerometer sensor
 * @param thr the threshold to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::setWakeUpThreshold(unsigned char thr)
{
  if ( LSM6DSL_ACC_GYRO_W_WK_THS( (void *)this, (uint8_t)thr ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief Enable the single tap detection for LSM6DSL accelerometer sensor
 * @param pin the interrupt pin to be used
 * @note  This function sets the LSM6DSL accelerometer ODR to 416Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::enableSingleTapDetection(LSM6DSL_Interrupt_Pin_t pin)
{
  /* Output Data Rate selection */
  if( setXOdr(416.0f) == 1 )
  {
    return 1;
  }
  
  /* Full scale selection. */
  if( setXFullScale(2.0f) == 1 )
  {
    return 1;
  }

  /* Enable X direction in tap recognition. */
  if ( LSM6DSL_ACC_GYRO_W_TAP_X_EN( (void *)this, LSM6DSL_ACC_GYRO_TAP_X_EN_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Enable Y direction in tap recognition. */
  if ( LSM6DSL_ACC_GYRO_W_TAP_Y_EN( (void *)this, LSM6DSL_ACC_GYRO_TAP_Y_EN_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Enable Z direction in tap recognition. */
  if ( LSM6DSL_ACC_GYRO_W_TAP_Z_EN( (void *)this, LSM6DSL_ACC_GYRO_TAP_Z_EN_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Set tap threshold. */
  if ( setTapThreshold( LSM6DSL_TAP_THRESHOLD_MID_LOW ) == 1 )
  {
    return 1;
  }
  
  /* Set tap shock time window. */
  if ( setTapShockTime( LSM6DSL_TAP_SHOCK_TIME_MID_HIGH ) == 1 )
  {
    return 1;
  }
  
  /* Set tap quiet time window. */
  if ( setTapQuietTime( LSM6DSL_TAP_QUIET_TIME_MID_LOW ) == 1 )
  {
    return 1;
  }
  
  /* _NOTE_: Tap duration time window - don't care for single tap. */
  
  /* _NOTE_: Single/Double Tap event - don't care of this flag for single tap. */
  
  /* Enable basic Interrupts */
  if ( LSM6DSL_ACC_GYRO_W_BASIC_INT( (void *)this, LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Enable single tap on either INT1 or INT2 pin */
  switch (pin)
  {
  case LSM6DSL_INT1_PIN:
    if ( LSM6DSL_ACC_GYRO_W_SingleTapOnInt1( (void *)this, LSM6DSL_ACC_GYRO_INT1_SINGLE_TAP_ENABLED ) == MEMS_ERROR )
    {
      return 1;
    }
    break;

  case LSM6DSL_INT2_PIN:
    if ( LSM6DSL_ACC_GYRO_W_SingleTapOnInt2( (void *)this, LSM6DSL_ACC_GYRO_INT2_SINGLE_TAP_ENABLED ) == MEMS_ERROR )
    {
      return 1;
    }
    break;

  default:
    return 1;
  }
  
  return 0;
}

/**
 * @brief Disable the single tap detection for LSM6DSL accelerometer sensor
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::disableSingleTapDetection(void)
{
  /* Disable single tap interrupt on INT1 pin. */
  if ( LSM6DSL_ACC_GYRO_W_SingleTapOnInt1( (void *)this, LSM6DSL_ACC_GYRO_INT1_SINGLE_TAP_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable single tap interrupt on INT2 pin. */
  if ( LSM6DSL_ACC_GYRO_W_SingleTapOnInt2( (void *)this, LSM6DSL_ACC_GYRO_INT2_SINGLE_TAP_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable basic Interrupts */
  if ( LSM6DSL_ACC_GYRO_W_BASIC_INT( (void *)this, LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Reset tap threshold. */
  if ( setTapThreshold( 0x0 ) == 1 )
  {
    return 1;
  }
  
  /* Reset tap shock time window. */
  if ( setTapShockTime( 0x0 ) == 1 )
  {
    return 1;
  }
  
  /* Reset tap quiet time window. */
  if ( setTapQuietTime( 0x0 ) == 1 )
  {
    return 1;
  }
  
  /* _NOTE_: Tap duration time window - don't care for single tap. */
  
  /* _NOTE_: Single/Double Tap event - don't care of this flag for single tap. */
  
  /* Disable Z direction in tap recognition. */
  if ( LSM6DSL_ACC_GYRO_W_TAP_Z_EN( (void *)this, LSM6DSL_ACC_GYRO_TAP_Z_EN_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable Y direction in tap recognition. */
  if ( LSM6DSL_ACC_GYRO_W_TAP_Y_EN( (void *)this, LSM6DSL_ACC_GYRO_TAP_Y_EN_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable X direction in tap recognition. */
  if ( LSM6DSL_ACC_GYRO_W_TAP_X_EN( (void *)this, LSM6DSL_ACC_GYRO_TAP_X_EN_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief Enable the double tap detection for LSM6DSL accelerometer sensor
 * @param pin the interrupt pin to be used
 * @note  This function sets the LSM6DSL accelerometer ODR to 416Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::enableDoubleTapDetection(LSM6DSL_Interrupt_Pin_t pin)
{
  /* Output Data Rate selection */
  if( setXOdr(416.0f) == 1 )
  {
    return 1;
  }
  
  /* Full scale selection. */
  if( setXFullScale(2.0f) == 1 )
  {
    return 1;
  }

  /* Enable X direction in tap recognition. */
  if ( LSM6DSL_ACC_GYRO_W_TAP_X_EN( (void *)this, LSM6DSL_ACC_GYRO_TAP_X_EN_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Enable Y direction in tap recognition. */
  if ( LSM6DSL_ACC_GYRO_W_TAP_Y_EN( (void *)this, LSM6DSL_ACC_GYRO_TAP_Y_EN_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Enable Z direction in tap recognition. */
  if ( LSM6DSL_ACC_GYRO_W_TAP_Z_EN( (void *)this, LSM6DSL_ACC_GYRO_TAP_Z_EN_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Set tap threshold. */
  if ( setTapThreshold( LSM6DSL_TAP_THRESHOLD_MID_LOW ) == 1 )
  {
    return 1;
  }
  
  /* Set tap shock time window. */
  if ( setTapShockTime( LSM6DSL_TAP_SHOCK_TIME_HIGH ) == 1 )
  {
    return 1;
  }
  
  /* Set tap quiet time window. */
  if ( setTapQuietTime( LSM6DSL_TAP_QUIET_TIME_HIGH ) == 1 )
  {
    return 1;
  }
  
  /* Set tap duration time window. */
  if ( setTapDurationTime( LSM6DSL_TAP_DURATION_TIME_MID ) == 1 )
  {
    return 1;
  }
  
  /* Single and double tap enabled. */
  if ( LSM6DSL_ACC_GYRO_W_SINGLE_DOUBLE_TAP_EV( (void *)this, LSM6DSL_ACC_GYRO_SINGLE_DOUBLE_TAP_DOUBLE_TAP ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Enable basic Interrupts */
  if ( LSM6DSL_ACC_GYRO_W_BASIC_INT( (void *)this, LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Enable double tap on either INT1 or INT2 pin */
  switch (pin)
  {
  case LSM6DSL_INT1_PIN:
    if ( LSM6DSL_ACC_GYRO_W_TapEvOnInt1( (void *)this, LSM6DSL_ACC_GYRO_INT1_TAP_ENABLED ) == MEMS_ERROR )
    {
      return 1;
    }
    break;

  case LSM6DSL_INT2_PIN:
    if ( LSM6DSL_ACC_GYRO_W_TapEvOnInt2( (void *)this, LSM6DSL_ACC_GYRO_INT2_TAP_ENABLED ) == MEMS_ERROR )
    {
      return 1;
    }
    break;

  default:
    return 1;
  }
  
  return 0;
}

/**
 * @brief Disable the double tap detection for LSM6DSL accelerometer sensor
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::disableDoubleTapDetection(void)
{
  /* Disable double tap interrupt on INT1 pin. */
  if ( LSM6DSL_ACC_GYRO_W_TapEvOnInt1( (void *)this, LSM6DSL_ACC_GYRO_INT1_TAP_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable double tap interrupt on INT2 pin. */
  if ( LSM6DSL_ACC_GYRO_W_TapEvOnInt2( (void *)this, LSM6DSL_ACC_GYRO_INT2_TAP_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable basic Interrupts */
  if ( LSM6DSL_ACC_GYRO_W_BASIC_INT( (void *)this, LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Reset tap threshold. */
  if ( setTapThreshold( 0x0 ) == 1 )
  {
    return 1;
  }
  
  /* Reset tap shock time window. */
  if ( setTapShockTime( 0x0 ) == 1 )
  {
    return 1;
  }
  
  /* Reset tap quiet time window. */
  if ( setTapQuietTime( 0x0 ) == 1 )
  {
    return 1;
  }
  
  /* Reset tap duration time window. */
  if ( setTapDurationTime( 0x0 ) == 1 )
  {
    return 1;
  }
  
  /* Only single tap enabled. */
  if ( LSM6DSL_ACC_GYRO_W_SINGLE_DOUBLE_TAP_EV( (void *)this, LSM6DSL_ACC_GYRO_SINGLE_DOUBLE_TAP_SINGLE_TAP ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable Z direction in tap recognition. */
  if ( LSM6DSL_ACC_GYRO_W_TAP_Z_EN( (void *)this, LSM6DSL_ACC_GYRO_TAP_Z_EN_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable Y direction in tap recognition. */
  if ( LSM6DSL_ACC_GYRO_W_TAP_Y_EN( (void *)this, LSM6DSL_ACC_GYRO_TAP_Y_EN_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable X direction in tap recognition. */
  if ( LSM6DSL_ACC_GYRO_W_TAP_X_EN( (void *)this, LSM6DSL_ACC_GYRO_TAP_X_EN_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief Set the tap threshold for LSM6DSL accelerometer sensor
 * @param thr the threshold to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::setTapThreshold(unsigned char thr)
{
  if ( LSM6DSL_ACC_GYRO_W_TAP_THS( (void *)this, (u8_t)thr ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief Set the tap shock time window for LSM6DSL accelerometer sensor
 * @param time the shock time window to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::setTapShockTime(uint8_t time)
{
  if ( LSM6DSL_ACC_GYRO_W_SHOCK_Duration( (void *)this, time ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief Set the tap quiet time window for LSM6DSL accelerometer sensor
 * @param time the quiet time window to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::setTapQuietTime(uint8_t time)
{
  if ( LSM6DSL_ACC_GYRO_W_QUIET_Duration( (void *)this, time ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief Set the tap duration of the time window for LSM6DSL accelerometer sensor
 * @param time the duration of the time window to be set
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::setTapDurationTime(uint8_t time)
{
  if ( LSM6DSL_ACC_GYRO_W_DUR( (void *)this, time ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief Enable the 6D orientation detection for LSM6DSL accelerometer sensor
 * @param pin the interrupt pin to be used
 * @note  This function sets the LSM6DSL accelerometer ODR to 416Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::enable6dOrientation(LSM6DSL_Interrupt_Pin_t pin)
{
  /* Output Data Rate selection */
  if( setXOdr(416.0f) == 1 )
  {
    return 1;
  }
  
  /* Full scale selection. */
  if( setXFullScale(2.0f) == 1 )
  {
    return 1;
  }

  /* Set 6D threshold. */
  if ( LSM6DSL_ACC_GYRO_W_SIXD_THS( (void *)this, LSM6DSL_ACC_GYRO_SIXD_THS_60_degree ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Enable basic Interrupts */
  if ( LSM6DSL_ACC_GYRO_W_BASIC_INT( (void *)this, LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Enable 6D orientation on either INT1 or INT2 pin */
  switch (pin)
  {
  case LSM6DSL_INT1_PIN:
    if ( LSM6DSL_ACC_GYRO_W_6DEvOnInt1( (void *)this, LSM6DSL_ACC_GYRO_INT1_6D_ENABLED ) == MEMS_ERROR )
    {
      return 1;
    }
    break;

  case LSM6DSL_INT2_PIN:
    if ( LSM6DSL_ACC_GYRO_W_6DEvOnInt2( (void *)this, LSM6DSL_ACC_GYRO_INT2_6D_ENABLED ) == MEMS_ERROR )
    {
      return 1;
    }
    break;

  default:
    return 1;
  }
  
  return 0;
}

/**
 * @brief Disable the 6D orientation detection for LSM6DSL accelerometer sensor
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::disable6dOrientation(void)
{
  /* Disable 6D orientation interrupt on INT1 pin. */
  if ( LSM6DSL_ACC_GYRO_W_6DEvOnInt1( (void *)this, LSM6DSL_ACC_GYRO_INT1_6D_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable 6D orientation interrupt on INT2 pin. */
  if ( LSM6DSL_ACC_GYRO_W_6DEvOnInt2( (void *)this, LSM6DSL_ACC_GYRO_INT2_6D_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Disable basic Interrupts */
  if ( LSM6DSL_ACC_GYRO_W_BASIC_INT( (void *)this, LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED ) == MEMS_ERROR )
  {
    return 1;
  }
  
  /* Reset 6D threshold. */
  if ( LSM6DSL_ACC_GYRO_W_SIXD_THS( (void *)this, LSM6DSL_ACC_GYRO_SIXD_THS_80_degree ) == MEMS_ERROR )
  {
    return 1;
  }
  
  return 0;
}

/**
 * @brief Get the 6D orientation XL axis for LSM6DSL accelerometer sensor
 * @param xl the pointer to the 6D orientation XL axis
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::get6dOrientationXL(uint8_t *xl)
{
  LSM6DSL_ACC_GYRO_DSD_XL_t xl_raw;
  
  if ( LSM6DSL_ACC_GYRO_R_DSD_XL( (void *)this, &xl_raw ) == MEMS_ERROR )
  {
    return 1;
  }
  
  switch( xl_raw )
  {
    case LSM6DSL_ACC_GYRO_DSD_XL_DETECTED:
      *xl = 1;
      break;
    case LSM6DSL_ACC_GYRO_DSD_XL_NOT_DETECTED:
      *xl = 0;
      break;
    default:
      return 1;
  }
  
  return 0;
}

/**
 * @brief Get the 6D orientation XH axis for LSM6DSL accelerometer sensor
 * @param xh the pointer to the 6D orientation XH axis
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::get6dOrientationXH(uint8_t *xh)
{
  LSM6DSL_ACC_GYRO_DSD_XH_t xh_raw;
  
  if ( LSM6DSL_ACC_GYRO_R_DSD_XH( (void *)this, &xh_raw ) == MEMS_ERROR )
  {
    return 1;
  }
  
  switch( xh_raw )
  {
    case LSM6DSL_ACC_GYRO_DSD_XH_DETECTED:
      *xh = 1;
      break;
    case LSM6DSL_ACC_GYRO_DSD_XH_NOT_DETECTED:
      *xh = 0;
      break;
    default:
      return 1;
  }
  
  return 0;
}

/**
 * @brief Get the 6D orientation YL axis for LSM6DSL accelerometer sensor
 * @param yl the pointer to the 6D orientation YL axis
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::get6dOrientationYL(uint8_t *yl)
{
  LSM6DSL_ACC_GYRO_DSD_YL_t yl_raw;
  
  if ( LSM6DSL_ACC_GYRO_R_DSD_YL( (void *)this, &yl_raw ) == MEMS_ERROR )
  {
    return 1;
  }
  
  switch( yl_raw )
  {
    case LSM6DSL_ACC_GYRO_DSD_YL_DETECTED:
      *yl = 1;
      break;
    case LSM6DSL_ACC_GYRO_DSD_YL_NOT_DETECTED:
      *yl = 0;
      break;
    default:
      return 1;
  }
  
  return 0;
}

/**
 * @brief Get the 6D orientation YH axis for LSM6DSL accelerometer sensor
 * @param yh the pointer to the 6D orientation YH axis
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::get6dOrientationYH(uint8_t *yh)
{
  LSM6DSL_ACC_GYRO_DSD_YH_t yh_raw;
  
  if ( LSM6DSL_ACC_GYRO_R_DSD_YH( (void *)this, &yh_raw ) == MEMS_ERROR )
  {
    return 1;
  }
  
  switch( yh_raw )
  {
    case LSM6DSL_ACC_GYRO_DSD_YH_DETECTED:
      *yh = 1;
      break;
    case LSM6DSL_ACC_GYRO_DSD_YH_NOT_DETECTED:
      *yh = 0;
      break;
    default:
      return 1;
  }
  
  return 0;
}

/**
 * @brief Get the 6D orientation ZL axis for LSM6DSL accelerometer sensor
 * @param zl the pointer to the 6D orientation ZL axis
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::get6dOrientationZL(uint8_t *zl)
{
  LSM6DSL_ACC_GYRO_DSD_ZL_t zl_raw;
  
  if ( LSM6DSL_ACC_GYRO_R_DSD_ZL( (void *)this, &zl_raw ) == MEMS_ERROR )
  {
    return 1;
  }
  
  switch( zl_raw )
  {
    case LSM6DSL_ACC_GYRO_DSD_ZL_DETECTED:
      *zl = 1;
      break;
    case LSM6DSL_ACC_GYRO_DSD_ZL_NOT_DETECTED:
      *zl = 0;
      break;
    default:
      return 1;
  }
  
  return 0;
}

/**
 * @brief Get the 6D orientation ZH axis for LSM6DSL accelerometer sensor
 * @param zh the pointer to the 6D orientation ZH axis
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::get6dOrientationZH(uint8_t *zh)
{
  LSM6DSL_ACC_GYRO_DSD_ZH_t zh_raw;
  
  if ( LSM6DSL_ACC_GYRO_R_DSD_ZH( (void *)this, &zh_raw ) == MEMS_ERROR )
  {
    return 1;
  }
  
  switch( zh_raw )
  {
    case LSM6DSL_ACC_GYRO_DSD_ZH_DETECTED:
      *zh = 1;
      break;
    case LSM6DSL_ACC_GYRO_DSD_ZH_NOT_DETECTED:
      *zh = 0;
      break;
    default:
      return 1;
  }
  
  return 0;
}

/**
 * @brief Get the status of all hardware events for LSM6DSL accelerometer sensor
 * @param status the pointer to the status of all hardware events
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::getEventStatus(LSM6DSL_Event_Status_t *status)
{
  uint8_t Wake_Up_Src = 0, Tap_Src = 0, D6D_Src = 0, Func_Src = 0, Md1_Cfg = 0, Md2_Cfg = 0, Int1_Ctrl = 0;

  memset((void *)status, 0x0, sizeof(LSM6DSL_Event_Status_t));

  if(readReg(LSM6DSL_ACC_GYRO_WAKE_UP_SRC, &Wake_Up_Src) != 0)
  {
    return 1;
  }

  if(readReg(LSM6DSL_ACC_GYRO_TAP_SRC, &Tap_Src) != 0)
  {
    return 1;
  }

  if(readReg(LSM6DSL_ACC_GYRO_D6D_SRC, &D6D_Src) != 0)
  {
    return 1;
  }

  if(readReg(LSM6DSL_ACC_GYRO_FUNC_SRC, &Func_Src) != 0)
  {
    return 1;
  }

  if(readReg(LSM6DSL_ACC_GYRO_MD1_CFG, &Md1_Cfg ) != 0 )
  {
    return 1;
  }

  if(readReg(LSM6DSL_ACC_GYRO_MD2_CFG, &Md2_Cfg ) != 0)
  {
    return 1;
  }

  if(readReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &Int1_Ctrl ) != 0)
  {
    return 1;
  }

  if((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_FF_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_FF_MASK))
  {
    if((Wake_Up_Src & LSM6DSL_ACC_GYRO_FF_EV_STATUS_MASK))
    {
      status->FreeFallStatus = 1;  
    }
  }

  if((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_WU_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_WU_MASK))
  {
    if((Wake_Up_Src & LSM6DSL_ACC_GYRO_WU_EV_STATUS_MASK))
    {
      status->WakeUpStatus = 1;  
    }
  }

  if((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_SINGLE_TAP_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_SINGLE_TAP_MASK))
  {
    if((Tap_Src & LSM6DSL_ACC_GYRO_SINGLE_TAP_EV_STATUS_MASK))
    {
      status->TapStatus = 1;  
    }
  }

  if((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_TAP_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_TAP_MASK))
  {
    if((Tap_Src & LSM6DSL_ACC_GYRO_DOUBLE_TAP_EV_STATUS_MASK))
    {
      status->DoubleTapStatus = 1;  
    }
  }

  if((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_6D_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_6D_MASK))
  {
    if((D6D_Src & LSM6DSL_ACC_GYRO_D6D_EV_STATUS_MASK))
    {
      status->D6DOrientationStatus = 1;  
    }
  }

  if((Int1_Ctrl & LSM6DSL_ACC_GYRO_INT1_PEDO_MASK))
  {
    if((Func_Src & LSM6DSL_ACC_GYRO_PEDO_EV_STATUS_MASK))
    {
      status->StepStatus = 1;  
    }
  }

  if((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_TILT_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_TILT_MASK))
  {
    if((Func_Src & LSM6DSL_ACC_GYRO_TILT_EV_STATUS_MASK))
    {
      status->TiltStatus = 1;  
    }
  }

  return 0;
}

/**
 * @brief Read the data from register
 * @param reg register address
 * @param data register data
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::readReg( uint8_t reg, uint8_t *data )
{

  if ( LSM6DSL_ACC_GYRO_read_reg( (void *)this, reg, data, 1 ) == MEMS_ERROR )
  {
    return 1;
  }

  return 0;
}

/**
 * @brief Write the data to register
 * @param reg register address
 * @param data register data
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSLSensor::writeReg( uint8_t reg, uint8_t data )
{

  if ( LSM6DSL_ACC_GYRO_write_reg( (void *)this, reg, &data, 1 ) == MEMS_ERROR )
  {
    return 1;
  }

  return 0;
}


unsigned char LSM6DSL_io_write( void *handle, unsigned char WriteAddr, unsigned char *pBuffer, int nBytesToWrite )
{
  return ((LSM6DSLSensor *)handle)->writeIO(pBuffer, WriteAddr, nBytesToWrite);
}

unsigned char LSM6DSL_io_read( void *handle, unsigned char ReadAddr, unsigned char *pBuffer, int nBytesToRead )
{
  return ((LSM6DSLSensor *)handle)->readIO(pBuffer, ReadAddr, nBytesToRead);
}

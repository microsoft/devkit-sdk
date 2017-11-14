/**
 ******************************************************************************
 * @file    lis2mdlSensor.h
 * @author  AST / EST
 * @version V0.0.1
 * @date    14-April-2015
 * @brief   Header file for component LIS2MDL
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

#ifndef __LIS2MDL_SENSOR_H
#define __LIS2MDL_SENSOR_H

/* Includes ------------------------------------------------------------------*/
#include "mbed.h"
#include "ST_INTERFACES/DevI2C.h"
#include "utility/lis2mdl_driver.h"
#include "ST_INTERFACES/MagneticSensor.h"

/* Classes -------------------------------------------------------------------*/
/** Class representing a LIS2MDL sensor component
 */
class LIS2MDLSensor : public MagneticSensor {
 public:
    /** Constructor
     * @param[in] i2c device I2C to be used for communication
     */
        LIS2MDLSensor(DevI2C &i2c) : MagneticSensor(), dev_i2c(i2c) {
    }
    
    /** Destructor
     */
        virtual ~LIS2MDLSensor() {}
    
    /*** Interface Methods ***/
    virtual int init(void *) {
        return LIS2MDLInit();
    }

    virtual int readId(unsigned char *m_id) {
        return LIS2MDL_Read_M_ID((uint8_t*)m_id);
    }

    virtual int getMAxes(int *pData) {
        return LIS2MDL_M_GetAxes((int32_t*)pData);
    }

 protected:
    /*** Methods ***/
    MAGNETO_StatusTypeDef LIS2MDLInit();
    MAGNETO_StatusTypeDef LIS2MDL_Read_M_ID(uint8_t *m_id);
    MAGNETO_StatusTypeDef LIS2MDL_M_GetAxes(int32_t *pData);
    MAGNETO_StatusTypeDef LIS2MDL_M_GetAxesRaw(int16_t *pData);

    /**
     * @brief      Utility function to read data from LIS2MDL
     * @param[out] pBuffer pointer to the byte-array to read data in to
     * @param[in]  RegisterAddr specifies internal address register to read from.
     * @param[in]  NumByteToRead number of bytes to be read.
     * @retval     MAGNETO_OK if ok, 
     * @retval     MAGNETO_ERROR if an I2C error has occurred
     */
    MAGNETO_StatusTypeDef LIS2MDL_IO_Read(uint8_t* pBuffer, 
                          uint8_t RegisterAddr, uint16_t NumByteToRead)
    {
        int ret = dev_i2c.i2c_read(pBuffer,
                       LIS2MDL_M_MEMS_ADDRESS,
                       RegisterAddr,
                       NumByteToRead);
        if(ret != 0) {
            return MAGNETO_ERROR;
        }
        return MAGNETO_OK;
    }
    
    /**
     * @brief      Utility function to write data to LIS2MDL
     * @param[in]  pBuffer pointer to the byte-array data to send
     * @param[in]  RegisterAddr specifies internal address register to read from.
     * @param[in]  NumByteToWrite number of bytes to write.
     * @retval     MAGNETO_OK if ok, 
     * @retval     MAGNETO_ERROR if an I2C error has occurred
     */
    MAGNETO_StatusTypeDef LIS2MDL_IO_Write(uint8_t* pBuffer, 
                           uint8_t RegisterAddr, uint16_t NumByteToWrite)
    {
        int ret = dev_i2c.i2c_write(pBuffer,
                        LIS2MDL_M_MEMS_ADDRESS,
                        RegisterAddr,
                        NumByteToWrite);
        if(ret != 0) {
            return MAGNETO_ERROR;
        }
        return MAGNETO_OK;
    }

    virtual int get_m_axes_raw(int16_t *pData) {
        return LIS2MDL_M_GetAxesRaw(pData);
    }

    /*** Instance Variables ***/
    /* IO Device */
    DevI2C &dev_i2c;
};

#endif // __LIS2MDL_SENSOR_H

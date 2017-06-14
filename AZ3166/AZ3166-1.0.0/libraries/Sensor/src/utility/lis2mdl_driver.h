/**
 ******************************************************************************
 * @file    lis2mdl.h
 * @author  MEMS Application Team
 * @version V1.3.0
 * @date    28-May-2015
 * @brief   This file contains definitions for the lis2mdl.c
 *          firmware driver.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LIS2MDL_H
#define __LIS2MDL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#ifndef NULL
#define NULL      (void *) 0
#endif

/**
* @brief  MAGNETO status enumerator definition
*/
typedef enum
{
  MAGNETO_OK = 0,
  MAGNETO_ERROR = 1,
  MAGNETO_TIMEOUT = 2,
  MAGNETO_NOT_IMPLEMENTED = 3
} MAGNETO_StatusTypeDef;


/******************************************************************************/
/***************** START MAGNETIC SENSOR REGISTER MAPPING  ********************/
/******************************************************************************/

#define WHO_AM_I_ADDR                             0x4F
#define INT_CTRL_REG                              0x63
#define CFG_REG_A                                 0x60
#define CFG_REG_B                                 0x61
#define CFG_REG_C                                 0x62
#define STATUS_REG                                0x67
#define OUTX_L_REG                                0x68
#define OUTX_H_REG                                0x69
#define OUTY_L_REG                                0x6A
#define OUTY_H_REG                                0x6B
#define OUTZ_L_REG                                0x6C
#define OUTZ_H_REG                                0x6D
/**
 * @brief Device Address
 */

#define LIS2MDL_M_MEMS_ADDRESS                              0x3C    // SAD[1] = 1

/* Magneto sensor IO functions */
extern MAGNETO_StatusTypeDef LIS2MDL_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToWrite);
extern MAGNETO_StatusTypeDef LIS2MDL_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToRead);

#ifdef __cplusplus
}
#endif

#endif /* __LIS2MDL_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

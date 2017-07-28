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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LPS22HB_SENSOR_H
#define __LPS22HB_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

#include "mico_common.h"

  
/** @addtogroup MICO_Drivers_interface
  * @{
  */

/** @addtogroup MiCO_Sensor_Driver
  * @{
  */

/** @defgroup MiCO_lps22hb_Driver MiCO lps22hb Driver
  * @brief Provide driver interface for lps22hb sensor
  * @{
  */

#ifndef LPS22HB_I2C_PORT
#define LPS22HB_I2C_PORT      MICO_I2C_NONE
#endif
  

#ifndef NULL
  #define NULL      (void *) 0
#endif
  
  
/******************************************************************************/
/*************************** START REGISTER MAPPING  **************************/
/******************************************************************************/
/**
 * @brief  PRESSURE init structure definition
 */
typedef struct
{
    uint8_t OutputDataRate;
    uint8_t PressureResolution;
    uint8_t TemperatureResolution;
    uint8_t DiffEnable;
    uint8_t BlockDataUpdate;
    uint8_t SPIMode;
} PRESSURE_InitTypeDef;

/**
 * @brief  PRESSURE status enumerator definition
 */
typedef enum {
    PRESSURE_OK = 0,
    PRESSURE_ERROR = 1,
    PRESSURE_TIMEOUT = 2,
    PRESSURE_NOT_IMPLEMENTED = 3
} PRESSURE_StatusTypeDef;

/**
 * @brief  PRESSURE component id enumerator definition
 */
typedef enum {
    PRESSURE_NONE_COMPONENT = 0,
    PRESSURE_LPS22HB_COMPONENT = 1,
    PRESSURE_LPS22HB_DIL24_COMPONENT = 2
} PRESSURE_ComponentTypeDef;

/**
 * @brief  PRESSURE driver extended structure definition
 */
typedef struct {
    PRESSURE_ComponentTypeDef id; /* This id must be unique for each component belonging to this class that wants to extend common class */
    void *pData; /* This pointer is specific for each component */
}PRESSURE_DrvExtTypeDef;

/**
 * @brief  PRESSURE driver structure definition
 */
typedef struct
{
    PRESSURE_StatusTypeDef       (*Init)(PRESSURE_InitTypeDef *);
    PRESSURE_StatusTypeDef       (*PowerOff)(void);
    PRESSURE_StatusTypeDef       (*ReadID)(uint8_t *);
    PRESSURE_StatusTypeDef       (*Reset)(void);
    void                         (*ConfigIT)(uint16_t);
    void                         (*EnableIT)(uint8_t);
    void                         (*DisableIT)(uint8_t);
    uint8_t                      (*ITStatus)(uint16_t, uint16_t);
    void                         (*ClearIT)(uint16_t, uint16_t);
    PRESSURE_StatusTypeDef       (*GetPressure)(float *);
    PRESSURE_StatusTypeDef       (*GetTemperature)(float *);
    void                         (*SlaveAddrRemap)(uint8_t);
    PRESSURE_DrvExtTypeDef       *extData;
}PRESSURE_DrvTypeDef;

/**
 * @brief Reference pressure (LSB data)
 * \code
 * Read/write
 * Default value: 0x00
 * 7:0 REF7-ODR0: Lower part of the reference pressure that
 *      is sum to the sensor output pressure.
 * \endcode
 */
#define LPS22HB_REF_P_XL_ADDR         0x15

/**
 * @brief Reference pressure (middle part)
 * \code
 * Read/write
 * Default value: 0x00
 * 7:0 REF15-ODR8: Middle part of the reference pressure that
 *      is sum to the sensor output pressure.
 * \endcode
 */
#define LPS22HB_REF_P_L_ADDR          0x16

/**
 * @brief Reference pressure (MSB part)
 * \code
 * Read/write
 * Default value: 0x00
 * 7:0 REF15-ODR8: Higher part of the reference pressure that
 *      is sum to the sensor output pressure.
 * \endcode
 */
#define LPS22HB_REF_P_H_ADDR          0x17

/**
 * @brief Device identifier register.
 * \code
 * Read
 * Default value: 0xBD
 * 7:0 This read-only register contains the device identifier that,
 for LPS22HB, is set to 0xCA.
 * \endcode
 */
#define LPS22HB_WHO_AM_I_ADDR                             0x0F

/**
 * @brief Pressure and temperature resolution mode register.
 * \code
 * Read
 * Default value: 0x05
 * [7:4] Reserved
 * [3:2] AVGP1-0: select the pressure internal average.
 * [1:0] AVGT1-0: select the temperature internal average.
 * \endcode
 */
#define LPS22HB_RES_CONF_ADDR                             0x1A

/**
 * @brief Pressure sensor control register 1
 * \code
 * Read/write
 * Default value: 0x00
 * 7 PD: power down control. 0 - disable; 1 - enable
 * 6:4 ODR2, ODR1, ODR0: output data rate selection.
 *     ODR2  | ODR1  | ODR0  | Pressure output data-rate(Hz)  | Temperature output data-rate(Hz)
 *   ----------------------------------------------------------------------------------
 *      0    |  0    |  0    |         one shot               |         one shot
 *      0    |  0    |  1    |            1                   |            1
 *      0    |  1    |  0    |            7                   |            7
 *      0    |  1    |  1    |            12.5                |            12.5
 *      1    |  0    |  0    |            25                  |            25
 *      1    |  0    |  1    |         Reserved               |         Reserved
 *      1    |  1    |  0    |         Reserved               |         Reserved
 *      1    |  1    |  1    |         Reserved               |         Reserved
 *
 * 3 DIFF_EN: Interrupt circuit. 0 - disable; 1 - enable
 * 2 BDU: block data update. 0 - disable; 1 - enable
 * 1 DELTA_EN: delta pressure. 0 - disable; 1 - enable
 * 1 RESET_AZ: reset AutoZero. 0 - disable; 1 - enable  ///////ALE REVIEW
 * 0 SIM: SPI Serial Interface Mode selection. 0 - SPI 4-wire; 1 - SPI 3-wire ///////ALE REVIEW
 * \endcode
 */
#define LPS22HB_CTRL_REG1_ADDR                    0x10

/**
 * @brief Pressure sensor control register 2
 * \code
 * Read/write
 * Default value: 0x00
 * 7 BOOT:  Reboot memory content. 0: normal mode; 1: reboot memory content
 * 6 FIFO_EN: FIFO. 0: disable; 1:  enable
 * 5 WTM_EN:  FIFO Watermark level use. 0: disable; 1: enable
 * 4:3 Reserved. keep these bits at 0
 * 2 SWRESET: Software reset. 0: normal mode; 1: SW reset.
 * 1 AUTO_ZERO: Autozero enable. 0: normal mode; 1: autozero enable.
 * 0 ONE_SHOT: One shot enable. 0: waiting for start of conversion; 1: start for a new dataset
 * \endcode
 */
#define LPS22HB_CTRL_REG2_ADDR                      0x11

/**
 * @brief Pressure sensor control register 3
 * \code
 * Read/write
 * Default value: 0x00
 * 7 INT_H_L: Interrupt. 0:active high; 1: active low.
 * 6 PP_OD: Push-Pull/OpenDrain selection on interrupt pads. 0: Push-pull; 1: open drain.
 * 5 Reserved
 * 4:3 INT2_S2, INT2_S1: INT2 output signal selection control bits. // TO DO
 * 1:0 INT1_S2, INT1_S1: data signal on INT1 pad control bits.
 *    INT1(2)_S2  | INT1(2)_S1  | INT1(2) pin
 *   ------------------------------------------------------
 *        0       |      0      |     Data signal
 *        0       |      1      |     Pressure high (P_high)
 *        1       |      0      |     Pressure low (P_low)
 *        1       |      1      |     P_low OR P_high


 * \endcode
 */
#define LPS22HB_CTRL_REG3_ADDR                    0x12

/**
 * @brief Pressure sensor control register 4
 * \code
 * Read/write
 * Default value: 0x00
 * 7 P2_EMPTY: Empty Signal on INT2 pin.
 * 6 P2_WTM: Watermark Signal on INT2 pin.
 * 5 P2_Overrun:Overrun Signal on INT2 pin.
 * 4 P2_DRDY: Data Ready Signal on INT2 pin.
 * 3 P1_EMPTY: Empty Signal on INT1 pin.
 * 2 P1_WTM: Watermark Signal on INT1 pin.
 * 1 P1_Overrun:Overrun Signal on INT1 pin.
 * 0 P1_DRDY: Data Ready Signal on INT1 pin.
 * \endcode
 */
//#define LPS22HB_CTRL_REG4_ADDR                    0x13

/**
 * @brief Interrupt configuration Register
 * \code
 * Read/write
 * Default value: 0x00.
 * 7:3 Reserved.
 * 2 LIR: Latch Interrupt request into INT_SOURCE register. 0 - disable; 1 - enable
 * 1 PL_E: Enable interrupt generation on differential pressure low event. 0 - disable; 1 - enable
 * 0 PH_E: Enable interrupt generation on differential pressure high event. 0 - disable; 1 - enable
 * \endcode
 */
//#define LPS22HB_INT_CFG_REG_ADDR                  0x24

/**
 * @brief Interrupt source Register
 * \code
 * Read
 * Default value: 0x00.
 * 7:3 0.
 * 2 IA: Interrupt Active.0: no interrupt has been generated; 1: one or more interrupt events have been generated.
 * 1 PL: Differential pressure Low. 0: no interrupt has been generated; 1: Low differential pressure event has occurred.
 * 0 PH: Differential pressure High. 0: no interrupt has been generated; 1: High differential pressure event has occurred.
 * \endcode
 */
#define LPS22HB_INT_SOURCE_REG_ADDR               0x25

/**
 * @brief Threshold pressure (LSB)
 * \code
 * Read
 * Default value: 0x00.
 * 7:0 THS7-THS0: Low part of threshold value for pressure interrupt
 * generation. The complete threshold value is given by THS_P_H & THS_P_L and is
 * expressed as unsigned number. P_ths(mbar)=(THS_P_H & THS_P_L)[dec]/16.
 * \endcode
 */
#define LPS22HB_THS_P_LOW_REG_ADDR                0x16

/**
 * @brief Threshold pressure (MSB)
 * \code
 * Read
 * Default value: 0x00.
 * 7:0 THS15-THS8: High part of threshold value for pressure interrupt
 * generation. The complete threshold value is given by THS_P_H & THS_P_L and is
 * expressed as unsigned number. P_ths(mbar)=(THS_P_H & THS_P_L)[dec]/16.
 * \endcode
 */
#define LPS22HB_THS_P_HIGH_REG_ADDR              0x17

/**
 * @brief  Status Register
 * \code
 * Read
 * Default value: 0x00
 * 7:6 0
 * 5 P_OR: Pressure data overrun. 0: no overrun has occurred; 1: new data for pressure has overwritten the previous one.
 * 4 T_OR: Temperature data overrun. 0: no overrun has occurred; 1: a new data for temperature has overwritten the previous one.
 * 3:2 0
 * 1 P_DA: Pressure data available. 0: new data for pressure is not yet available; 1: new data for pressure is available.
 * 0 T_DA: Temperature data available. 0: new data for temperature is not yet available; 1: new data for temperature is available.
 * \endcode
 */
#define LPS22HB_STATUS_REG_ADDR                 0x27

/**
 * @brief  Pressure data (LSB).
 * \code
 * Read
 * Default value: 0x00.
 * POUT7 - POUT0: Pressure data LSB (2's complement).
 * Pressure output data: Pout(mbar)=(PRESS_OUT_H & PRESS_OUT_L &
 * PRESS_OUT_XL)[dec]/4096.
 * \endcode
 */
#define LPS22HB_PRESS_POUT_XL_ADDR              0x28

/**
 * @brief  Pressure data (Middle part).
 * \code
 * Read
 * Default value: 0x80.
 * POUT15 - POUT8: Pressure data middle part (2's complement).
 * Pressure output data: Pout(mbar)=(PRESS_OUT_H & PRESS_OUT_L &
 * PRESS_OUT_XL)[dec]/4096.
 * \endcode
 */
#define LPS22HB_PRESS_OUT_L_ADDR                0x29

/**
 * @brief  Pressure data (MSB).
 * \code
 * Read
 * Default value: 0x2F.
 * POUT23 - POUT16: Pressure data MSB (2's complement).
 * Pressure output data: Pout(mbar)=(PRESS_OUT_H & PRESS_OUT_L &
 * PRESS_OUT_XL)[dec]/4096.
 * \endcode
 */
#define LPS22HB_PRESS_OUT_H_ADDR                0x2A

/**
 * @brief  Temperature data (LSB).
 * \code
 * Read
 * Default value: 0x00.
 * TOUT7 - TOUT0: temperature data LSB.
 * T(degC) = 42.5 + (Temp_OUTH & TEMP_OUT_L)[dec]/480.
 * \endcode
 */
#define LPS22HB_TEMP_OUT_L_ADDR                 0x2B

/**
 * @brief  Temperature data (MSB).
 * \code
 * Read
 * Default value: 0x00.
 * TOUT15 - TOUT8: temperature data MSB.
 * T(degC) = 42.5 + (Temp_OUTH & TEMP_OUT_L)[dec]/480.
 * \endcode
 */
#define LPS22HB_TEMP_OUT_H_ADDR                 0x2C

/**
 * @brief FIFO control register
 * \code
 * Read/write
 * Default value: 0x00
 * 7:5 F_MODE2, F_MODE1, F_MODE0: FIFO mode selection.
 *     FM2   | FM1   | FM0   |    FIFO MODE
 *   ---------------------------------------------------
 *      0    |  0    |  0    |      BYPASS MODE
 *      0    |  0    |  1    | FIFO MODE. Stops collecting data when full
 *      0    |  1    |  0    | STREAM MODE: Keep the newest measurements in the FIFO
 *      0    |  1    |  1    | STREAM MODE until trigger deasserted, then change to FIFO MODE
 *      1    |  0    |  0    | BYPASS MODE until trigger deasserted, then STREAM MODE
 *      1    |  0    |  1    |       Reserved
 *      1    |  1    |  0    | FIFO_MEAN MODE: Fifo is used to generate a running average filtered pressure
 *      1    |  1    |  1    | BYPASS mode until trigger deasserted, then FIFO MODE
 *
 * 4:0 FIFO Mean Mode Sample size
 *     WTM_POINT4 | WTM_POINT4 | WTM_POINT4 |  WTM_POINT4 | WTM_POINT4 | Sample Size
 *   ----------------------------------------------------------------------------------
 *      0         |    0       |    0       |      0      |     1      |       2
 *      0         |    0       |    0       |      1      |     1      |       4
 *      0         |    0       |    1       |      1      |     1      |       8
 *      0         |    1       |    1       |      1      |     1      |       16
 *      1         |    1       |    1       |      1      |     1      |       32
 * other values operation not guaranteed
 * \endcode
 */
#define LPS22HB_CTRL_FIFO_ADDR                    0x14

/**
 * @brief FIFO Status register
 * \code
 * Read/write
 * Default value: 0x00
 * 7 WTM_FIFO: Watermark status. 0:FIFO filling is lower than watermark level; 1: FIFO is equal or higher than watermark level.
 * 6 FULL_FIFO: Overrun bit status. 0 - FIFO not full; 1 -FIFO is full.
 * 5 EMPTY_FIFO: Empty FIFO bit. 0 - FIFO not empty; 1 -FIFO is empty.
 * 4:0 DIFF_POINT4...0: FIFOsStored data level.
 * \endcode
 */
#define LPS22HB_STATUS_FIFO_ADDR                    0x26

/**
 * @brief Pressure offset register
 * \code
 * Read/write
 * Default value: 0x00
 * 7:0 RPDS15...8:Pressure Offset for 1 point calibration after soldering.
 * \endcode
 */
#define LPS22HB_RPDS_TRIM_L_ADDR                    0x18

/**
 * @brief Pressure offset register
 * \code
 * Read/write
 * Default value: 0x00
 * 7:0 RPDS23...16:Pressure Offset for 1 point calibration after soldering.
 * \endcode
 */
#define LPS22HB_RPDS_TRIM_H_ADDR                    0x19

/******************************************************************************/
/**************************** END REGISTER MAPPING  ***************************/
/******************************************************************************/

/**
 * @brief Multiple Byte. Mask for enabling multiple byte read/write command.
 */   
#define LPS22HB_I2C_MULTIPLEBYTE_CMD                      ((uint8_t)0x80)  
  
/**
 * @brief Device Address
 */
#define LPS22HB_ADDRESS_LOW           0xB8
#define LPS22HB_ADDRESS_HIGH          0xBA


/**
 * @brief Device Identifier. Default value of the WHO_AM_I register.
 */
#define I_AM_LPS22HB                 ((uint8_t)0xBD)


#define LPS22HB_MODE_POWERDOWN            ((uint8_t)0x00)
#define LPS22HB_MODE_ACTIVE               ((uint8_t)0x80)

#define LPS22HB_MODE_MASK                 ((uint8_t)0x80)

#define LPS22HB_ODR_ONE_SHOT             ((uint8_t)0x00) /*!< Output Data Rate: P - one shot, T - one shot */
#define LPS22HB_ODR_1Hz                  ((uint8_t)0x10) /*!< Output Data Rate: P - 1Hz, T - 1Hz */
#define LPS22HB_ODR_7Hz                  ((uint8_t)0x20) /*!< Output Data Rate: P - 7Hz, T - 7Hz */
#define LPS22HB_ODR_12_5Hz               ((uint8_t)0x30) /*!< Output Data Rate: P - 12.5Hz, T - 12.5Hz */
#define LPS22HB_ODR_25Hz                 ((uint8_t)0x40) /*!< Output Data Rate: P - 25Hz, T - 25Hz */

#define LPS22HB_ODR_MASK                 ((uint8_t)0x70)


#define LPS22HB_DIFF_DISABLE             ((uint8_t)0x00) /*!< interrupt circuit enabled */
#define LPS22HB_DIFF_ENABLE              ((uint8_t)0x08) /*!< interrupt generation disabled */

#define LPS22HB_DIFF_EN_MASK             ((uint8_t)0x08)


#define LPS22HB_BDU_CONT              ((uint8_t)0x00) /*!< continuous update */
#define LPS22HB_BDU_READ              ((uint8_t)0x04) /*!< output registers not updated until MSB and LSB reading */

#define LPS22HB_BDU_MASK          ((uint8_t)0x04)

#define LPS22HB_SPI_SIM_4W            ((uint8_t)0x00) /*!< 4-wire interface */
#define LPS22HB_SPI_SIM_3W            ((uint8_t)0x01) /*!< 3-wire interface */

#define LPS22HB_SPI_SIM_MASK          ((uint8_t)0x01)

#define LPS22HB_NORMAL_MODE           ((uint8_t)0x00)
#define LPS22HB_RESET_MEMORY          ((uint8_t)0x80)

#define LPS22HB_RESET_MEMORY_MASK     ((uint8_t)0x80)

#define LPS22HB_P_RES_AVG_8              ((uint8_t)0x00)
#define LPS22HB_P_RES_AVG_32             ((uint8_t)0x01)
#define LPS22HB_P_RES_AVG_128            ((uint8_t)0x02)
#define LPS22HB_P_RES_AVG_512            ((uint8_t)0x03)

#define LPS22HB_P_RES_MASK               ((uint8_t)0x03)

#define LPS22HB_ONE_SHOT_START               ((uint8_t)0x01)

#define LPS22HB_ONE_SHOT_MASK                ((uint8_t)0x01)


#define LPS22HB_T_RES_AVG_8              ((uint8_t)0x00)
#define LPS22HB_T_RES_AVG_16             ((uint8_t)0x04)
#define LPS22HB_T_RES_AVG_32             ((uint8_t)0x08)
#define LPS22HB_T_RES_AVG_64             ((uint8_t)0x0C)

#define LPS22HB_T_RES_MASK               ((uint8_t)0x0C)

#define LPS22HB_SA0_LOW                  ((uint8_t)0x00)
#define LPS22HB_SA0_HIGH                 ((uint8_t)0x01)


/* ------------------------------------------------------- */ 
/* Here you should declare the internal struct of          */
/* extended features of LPS22HB. See the example of        */
/* LSM6DS3 in lsm6ds3.h                                    */
/* ------------------------------------------------------- */


/* Pressure sensor driver structure */
extern PRESSURE_DrvTypeDef LPS22HBDrv;
extern PRESSURE_DrvExtTypeDef LPS22HBDrv_ext;


/**
 * @brief Initialize lps22hb sensor device
 *
 * @return   kNoErr        : on success.
 * @return   kGeneralErr   : if an error occurred
 */
PRESSURE_StatusTypeDef lps22hb_sensor_init(void *handle);

/**
 * @brief   Deinitialize lps22hb sensor device.
 * 
 * @return   kNoErr        : on success.
 * @return   kGeneralErr   : if an error occurred
 */
PRESSURE_StatusTypeDef lps22hb_sensor_deinit(void *handle);



PRESSURE_StatusTypeDef LPS22HB_Init(void *handle, PRESSURE_InitTypeDef *LPS22HB_Init);
PRESSURE_StatusTypeDef LPS22HB_ReadID(void *handle, uint8_t *p_id);
PRESSURE_StatusTypeDef LPS22HB_RebootCmd(void *handle);
PRESSURE_StatusTypeDef LPS22HB_GetPressure(void *handle, float* pfData);
PRESSURE_StatusTypeDef LPS22HB_GetTemperature(void *handle, float* pfData);
PRESSURE_StatusTypeDef LPS22HB_PowerOff(void *handle);
void LPS22HB_SlaveAddrRemap(void *handle, uint8_t SA0_Bit_Status);
PRESSURE_StatusTypeDef LPS22HB_PowerOn(void *handle);
PRESSURE_StatusTypeDef LPS22HB_I2C_ReadRawPressure(void *handle, uint32_t *raw_press);
PRESSURE_StatusTypeDef LPS22HB_I2C_ReadRawTemperature(void *handle, int16_t *raw_data);
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __LPS22HB_SENSOR_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

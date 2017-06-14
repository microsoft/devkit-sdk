/**
 ******************************************************************************
 * @file    LSM6DSLSensor.h
 * @author  CLab
 * @version V1.0.0
 * @date    5 August 2016
 * @brief   Abstract Class of an LSM6DSL Inertial Measurement Unit (IMU) 6 axes
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


/* Prevent recursive inclusion -----------------------------------------------*/

#ifndef __LSM6DSLSensor_H__
#define __LSM6DSLSensor_H__


/* Includes ------------------------------------------------------------------*/

#include "ST_INTERFACES/DevI2C.h"
#include "utility/LSM6DSL_acc_gyro_driver.h"
#include "ST_INTERFACES/MotionSensor.h"
#include "ST_INTERFACES/GyroSensor.h"

/* Defines -------------------------------------------------------------------*/

#define LSM6DSL_ACC_SENSITIVITY_FOR_FS_2G   0.061  /**< Sensitivity value for 2 g full scale [mg/LSB] */
#define LSM6DSL_ACC_SENSITIVITY_FOR_FS_4G   0.122  /**< Sensitivity value for 4 g full scale [mg/LSB] */
#define LSM6DSL_ACC_SENSITIVITY_FOR_FS_8G   0.244  /**< Sensitivity value for 8 g full scale [mg/LSB] */
#define LSM6DSL_ACC_SENSITIVITY_FOR_FS_16G  0.488  /**< Sensitivity value for 16 g full scale [mg/LSB] */

#define LSM6DSL_GYRO_SENSITIVITY_FOR_FS_125DPS   04.375  /**< Sensitivity value for 125 dps full scale [mdps/LSB] */
#define LSM6DSL_GYRO_SENSITIVITY_FOR_FS_245DPS   08.750  /**< Sensitivity value for 245 dps full scale [mdps/LSB] */
#define LSM6DSL_GYRO_SENSITIVITY_FOR_FS_500DPS   17.500  /**< Sensitivity value for 500 dps full scale [mdps/LSB] */
#define LSM6DSL_GYRO_SENSITIVITY_FOR_FS_1000DPS  35.000  /**< Sensitivity value for 1000 dps full scale [mdps/LSB] */
#define LSM6DSL_GYRO_SENSITIVITY_FOR_FS_2000DPS  70.000  /**< Sensitivity value for 2000 dps full scale [mdps/LSB] */

#define LSM6DSL_PEDOMETER_THRESHOLD_LOW       0x00  /**< Lowest  value of pedometer threshold */
#define LSM6DSL_PEDOMETER_THRESHOLD_MID_LOW   0x07
#define LSM6DSL_PEDOMETER_THRESHOLD_MID       0x0F
#define LSM6DSL_PEDOMETER_THRESHOLD_MID_HIGH  0x17
#define LSM6DSL_PEDOMETER_THRESHOLD_HIGH      0x1F  /**< Highest value of pedometer threshold */

#define LSM6DSL_WAKE_UP_THRESHOLD_LOW       0x01  /**< Lowest  value of wake up threshold */
#define LSM6DSL_WAKE_UP_THRESHOLD_MID_LOW   0x0F
#define LSM6DSL_WAKE_UP_THRESHOLD_MID       0x1F
#define LSM6DSL_WAKE_UP_THRESHOLD_MID_HIGH  0x2F
#define LSM6DSL_WAKE_UP_THRESHOLD_HIGH      0x3F  /**< Highest value of wake up threshold */

#define LSM6DSL_TAP_THRESHOLD_LOW       0x01  /**< Lowest  value of wake up threshold */
#define LSM6DSL_TAP_THRESHOLD_MID_LOW   0x08
#define LSM6DSL_TAP_THRESHOLD_MID       0x10
#define LSM6DSL_TAP_THRESHOLD_MID_HIGH  0x18
#define LSM6DSL_TAP_THRESHOLD_HIGH      0x1F  /**< Highest value of wake up threshold */

#define LSM6DSL_TAP_SHOCK_TIME_LOW       0x00  /**< Lowest  value of wake up threshold */
#define LSM6DSL_TAP_SHOCK_TIME_MID_LOW   0x01
#define LSM6DSL_TAP_SHOCK_TIME_MID_HIGH  0x02
#define LSM6DSL_TAP_SHOCK_TIME_HIGH      0x03  /**< Highest value of wake up threshold */

#define LSM6DSL_TAP_QUIET_TIME_LOW       0x00  /**< Lowest  value of wake up threshold */
#define LSM6DSL_TAP_QUIET_TIME_MID_LOW   0x01
#define LSM6DSL_TAP_QUIET_TIME_MID_HIGH  0x02
#define LSM6DSL_TAP_QUIET_TIME_HIGH      0x03  /**< Highest value of wake up threshold */

#define LSM6DSL_TAP_DURATION_TIME_LOW       0x00  /**< Lowest  value of wake up threshold */
#define LSM6DSL_TAP_DURATION_TIME_MID_LOW   0x04
#define LSM6DSL_TAP_DURATION_TIME_MID       0x08
#define LSM6DSL_TAP_DURATION_TIME_MID_HIGH  0x0C
#define LSM6DSL_TAP_DURATION_TIME_HIGH      0x0F  /**< Highest value of wake up threshold */

/* Typedefs ------------------------------------------------------------------*/

typedef enum
{
  LSM6DSL_INT1_PIN,
  LSM6DSL_INT2_PIN
} LSM6DSL_Interrupt_Pin_t;

typedef struct
{
  unsigned int FreeFallStatus : 1;
  unsigned int TapStatus : 1;
  unsigned int DoubleTapStatus : 1;
  unsigned int WakeUpStatus : 1;
  unsigned int StepStatus : 1;
  unsigned int TiltStatus : 1;
  unsigned int D6DOrientationStatus : 1;
} LSM6DSL_Event_Status_t;

/* Class Declaration ---------------------------------------------------------*/

/**
 * Abstract class of an LSM6DSL Inertial Measurement Unit (IMU) 6 axes
 * sensor.
 */
class LSM6DSLSensor : public MotionSensor, public GyroSensor
{
  public:
    LSM6DSLSensor(DevI2C &i2c, PinName INT1_pin, PinName INT2_pin);
    LSM6DSLSensor(DevI2C &i2c, PinName INT1_pin, PinName INT2_pin, uint8_t address);

    virtual int init(void *init);
    virtual int readId(unsigned char *id);
    virtual int getXAxes(int *pData);
    virtual int getGAxes(int *pData);
    virtual int getXSensitivity(float *pfData);
    virtual int getGSensitivity(float *pfData);
    virtual int getXOdr(float *odr);
    virtual int getGOdr(float *odr);
    virtual int setXOdr(float odr);
    virtual int setGOdr(float odr);
    virtual int getXFullScale(float *fullScale);
    virtual int getGFullScale(float *fullScale);
    virtual int setXFullScale(float fullScale);
    virtual int setGFullScale(float fullScale);

    int enableAccelerator(void);
    int enableGyroscope(void);
    int disableAccelerator(void);
    int disableGyroscope(void);
    int enableFreeFallDetection(LSM6DSL_Interrupt_Pin_t pin = LSM6DSL_INT1_PIN);
    int disableFreeFallDetection(void);
    int setFreeFallThreshold(int thr);
    int enablePedometer(void);
    int disablePedometer(void);
    int getStepCounter(int *step_count);
    int resetStepCounter(void);
    int setPedometerThreshold(unsigned char thr);
    int enableTiltDetection(LSM6DSL_Interrupt_Pin_t pin = LSM6DSL_INT1_PIN);
    int disableTiltDetection(void);
    int enableWakeUpDetection(LSM6DSL_Interrupt_Pin_t pin = LSM6DSL_INT2_PIN);
    int disableWakeUpDetection(void);
    int setWakeUpThreshold(unsigned char thr);
    int enableSingleTapDetection(LSM6DSL_Interrupt_Pin_t pin = LSM6DSL_INT1_PIN);
    int disableSingleTapDetection(void);
    int enableDoubleTapDetection(LSM6DSL_Interrupt_Pin_t pin = LSM6DSL_INT1_PIN);
    int disableDoubleTapDetection(void);
    int setTapThreshold(unsigned char thr);
    int setTapShockTime(unsigned char time);
    int setTapQuietTime(unsigned char time);
    int setTapDurationTime(unsigned char time);
    int enable6dOrientation(LSM6DSL_Interrupt_Pin_t pin = LSM6DSL_INT1_PIN);
    int disable6dOrientation(void);
    int get6dOrientationXL(unsigned char *xl);
    int get6dOrientationXH(unsigned char *xh);
    int get6dOrientationYL(unsigned char *yl);
    int get6dOrientationYH(unsigned char *yh);
    int get6dOrientationZL(unsigned char *zl);
    int get6dOrientationZH(unsigned char *zh);
    int getEventStatus(LSM6DSL_Event_Status_t *status);

    /**
     * @brief  Attaching an interrupt handler to the INT1 interrupt.
     * @param  fptr An interrupt handler.
     * @retval None.
     */
    void attachInt1Irq(void (*fptr)(void))
    {
        _int1_irq.rise(fptr);
    }

    /**
     * @brief  Enabling the INT1 interrupt handling.
     * @param  None.
     * @retval None.
     */
    void enableInt1Irq(void)
    {
        _int1_irq.enable_irq();
    }
    
    /**
     * @brief  Disabling the INT1 interrupt handling.
     * @param  None.
     * @retval None.
     */
    void disableInt1Irq(void)
    {
        _int1_irq.disable_irq();
    }
    
    /**
     * @brief  Attaching an interrupt handler to the INT2 interrupt.
     * @param  fptr An interrupt handler.
     * @retval None.
     */
    void attachInt2Irq(void (*fptr)(void))
    {
        _int2_irq.rise(fptr);
    }

    /**
     * @brief  Enabling the INT2 interrupt handling.
     * @param  None.
     * @retval None.
     */
    void enableInt2Irq(void)
    {
        _int2_irq.enable_irq();
    }
    
    /**
     * @brief  Disabling the INT2 interrupt handling.
     * @param  None.
     * @retval None.
     */
    void disableInt2Irq(void)
    {
        _int2_irq.disable_irq();
    }

    /**
     * @brief Utility function to read data.
     * @param  pBuffer: pointer to data to be read.
     * @param  RegisterAddr: specifies internal address register to be read.
     * @param  NumByteToRead: number of bytes to be read.
     * @retval 0 if ok, an error code otherwise.
     */
    unsigned char readIO(unsigned char* pBuffer, unsigned char RegisterAddr, int NumByteToRead)
    {
        return (unsigned char) _dev_i2c.i2c_read(pBuffer, _address, RegisterAddr, (uint16_t)NumByteToRead);
    }
    
    /**
     * @brief Utility function to write data.
     * @param  pBuffer: pointer to data to be written.
     * @param  RegisterAddr: specifies internal address register to be written.
     * @param  NumByteToWrite: number of bytes to write.
     * @retval 0 if ok, an error code otherwise.
     */
    unsigned char writeIO(unsigned char* pBuffer, unsigned char RegisterAddr, int NumByteToWrite)
    {
        return (unsigned char) _dev_i2c.i2c_write(pBuffer, _address, RegisterAddr, (uint16_t)NumByteToWrite);
    }

  private:
    int setXOdrWhenEnabled(float odr);
    int setGOdrWhenEnabled(float odr);
    int setXOdrWhenDisabled(float odr);
    int setGOdrWhenDisabled(float odr);
    int readReg(uint8_t reg, uint8_t *data);
    int writeReg(uint8_t reg, uint8_t data);

    virtual int getXAxesRaw(int16_t *pData);
    virtual int getGAxesRaw(int16_t *pData);

    /* Helper classes. */
    DevI2C &_dev_i2c;

    InterruptIn _int1_irq;
    InterruptIn _int2_irq;

    /* Configuration */
    uint8_t _address;
    
    uint8_t _x_is_enabled;
    float _x_last_odr;
    uint8_t _g_is_enabled;
    float _g_last_odr;
};

#ifdef __cplusplus
 extern "C" {
#endif
unsigned char LSM6DSL_io_write( void *handle, unsigned char WriteAddr, unsigned char *pBuffer, int nBytesToWrite );
unsigned char LSM6DSL_io_read( void *handle, unsigned char ReadAddr, unsigned char *pBuffer, int nBytesToRead );
#ifdef __cplusplus
  }
#endif

#endif

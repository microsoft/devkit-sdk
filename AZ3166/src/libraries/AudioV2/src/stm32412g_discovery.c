/**
 ******************************************************************************
 * @file    stm32412g_discovery.c
 * @author  MCD Application Team
 * @version V2.0.0
 * @date    27-January-2017
 * @brief   This file provides a set of firmware functions to manage LEDs,
 *          push-buttons and COM ports available on STM32412G-DISCOVERY board
 *          (MB1209) from STMicroelectronics.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
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
#include "stm32412g_discovery.h"

/** @addtogroup BSP
 * @{
 */

/** @addtogroup STM32412G_DISCOVERY
 * @{
 */

/** @defgroup STM32412G_DISCOVERY_LOW_LEVEL STM32412G-DISCOVERY LOW LEVEL
 * @{
 */

/** @defgroup STM32412G_DISCOVERY_LOW_LEVEL_Private_TypesDefinitions STM32412G Discovery Low Level Private Typedef
 * @{
 */
typedef struct
{
    __IO uint16_t REG;
    __IO uint16_t RAM;
} LCD_CONTROLLER_TypeDef;
/**
 * @}
 */

/** @defgroup STM32412G_DISCOVERY_LOW_LEVEL_Private_Defines STM32412G Discovery Low Level Private Def
 * @{
 */
/**
 * @brief STM32412G DISCOVERY BSP Driver version number V2.0.0
 */
#define __STM32412G_DISCOVERY_BSP_VERSION_MAIN   (0x02) /*!< [31:24] main version */
#define __STM32412G_DISCOVERY_BSP_VERSION_SUB1   (0x00) /*!< [23:16] sub1 version */
#define __STM32412G_DISCOVERY_BSP_VERSION_SUB2   (0x00) /*!< [15:8]  sub2 version */
#define __STM32412G_DISCOVERY_BSP_VERSION_RC     (0x00) /*!< [7:0]  release candidate */
#define __STM32412G_DISCOVERY_BSP_VERSION        ((__STM32412G_DISCOVERY_BSP_VERSION_MAIN << 24)\
                                                 |(__STM32412G_DISCOVERY_BSP_VERSION_SUB1 << 16)\
                                                 |(__STM32412G_DISCOVERY_BSP_VERSION_SUB2 << 8 )\
                                                 |(__STM32412G_DISCOVERY_BSP_VERSION_RC))

/* We use BANK1 as we use FMC_NE1 signal */
#define FMC_BANK1_BASE  ((uint32_t)(0x60000000 | 0x00000000))  
#define FMC_BANK3_BASE  ((uint32_t)(0x60000000 | 0x08000000))  
#define FMC_BANK1       ((LCD_CONTROLLER_TypeDef *) FMC_BANK1_BASE)

/**
 * @}
 */

/** @defgroup STM32412G_DISCOVERY_LOW_LEVEL_Private_Variables STM32412G Discovery Low Level Variables 
 * @{
 */

const uint32_t GPIO_PIN[LEDn] = { LED1_PIN,
LED2_PIN,
LED3_PIN,
LED4_PIN };

GPIO_TypeDef* BUTTON_PORT[BUTTONn] = { WAKEUP_BUTTON_GPIO_PORT };

const uint16_t BUTTON_PIN[BUTTONn] = { WAKEUP_BUTTON_PIN };

const uint16_t BUTTON_IRQn[BUTTONn] = { WAKEUP_BUTTON_EXTI_IRQn };

GPIO_TypeDef* JOY_PORT[JOYn] = { SEL_JOY_GPIO_PORT,
DOWN_JOY_GPIO_PORT,
LEFT_JOY_GPIO_PORT,
RIGHT_JOY_GPIO_PORT,
UP_JOY_GPIO_PORT };

const uint16_t JOY_PIN[JOYn] = { SEL_JOY_PIN,
DOWN_JOY_PIN,
LEFT_JOY_PIN,
RIGHT_JOY_PIN,
UP_JOY_PIN };

const uint8_t JOY_IRQn[JOYn] = { SEL_JOY_EXTI_IRQn,
DOWN_JOY_EXTI_IRQn,
LEFT_JOY_EXTI_IRQn,
RIGHT_JOY_EXTI_IRQn,
UP_JOY_EXTI_IRQn };

USART_TypeDef* COM_USART[COMn] = { DISCOVERY_COM1 };

GPIO_TypeDef* COM_TX_PORT[COMn] = { DISCOVERY_COM1_TX_GPIO_PORT };

GPIO_TypeDef* COM_RX_PORT[COMn] = { DISCOVERY_COM1_RX_GPIO_PORT };

const uint16_t COM_TX_PIN[COMn] = { DISCOVERY_COM1_TX_PIN };

const uint16_t COM_RX_PIN[COMn] = { DISCOVERY_COM1_RX_PIN };

const uint16_t COM_TX_AF[COMn] = { DISCOVERY_COM1_TX_AF };

const uint16_t COM_RX_AF[COMn] = { DISCOVERY_COM1_RX_AF };

static I2C_HandleTypeDef hI2cAudioHandler;
//static I2C_HandleTypeDef hI2cExtHandler;

/**
 * @}
 */

/** @defgroup STM32412G_DISCOVERY_LOW_LEVEL_Private_FunctionPrototypes STM32412G Discovery Low Level Private Prototypes
 * @{
 */
static void I2Cx_MspInit( I2C_HandleTypeDef *i2c_handler );
static void I2Cx_Init( I2C_HandleTypeDef *i2c_handler );

static HAL_StatusTypeDef I2Cx_ReadMultiple( I2C_HandleTypeDef *i2c_handler, uint8_t Addr, uint16_t Reg,
                                            uint16_t MemAddSize, uint8_t *Buffer, uint16_t Length );
static HAL_StatusTypeDef I2Cx_WriteMultiple( I2C_HandleTypeDef *i2c_handler, uint8_t Addr, uint16_t Reg,
                                             uint16_t MemAddSize, uint8_t *Buffer, uint16_t Length );
//static HAL_StatusTypeDef I2Cx_IsDeviceReady( I2C_HandleTypeDef *i2c_handler, uint16_t DevAddress, uint32_t Trials );
static void I2Cx_Error( I2C_HandleTypeDef *i2c_handler, uint8_t Addr );

static void FMC_BANK1_WriteData( uint16_t Data );
static void FMC_BANK1_WriteReg( uint8_t Reg );
static uint16_t FMC_BANK1_ReadData( void );
static void FMC_BANK1_Init( void );
static void FMC_BANK1_MspInit( void );

/* LCD IO functions */
void LCD_IO_Init( void );
void LCD_IO_WriteData( uint16_t RegValue );
void LCD_IO_WriteReg( uint8_t Reg );
void LCD_IO_WriteMultipleData( uint16_t *pData, uint32_t Size );
uint16_t LCD_IO_ReadData( void );
void LCD_IO_Delay( uint32_t Delay );

/* AUDIO IO functions */
void AUDIO_IO_Init( void );
void AUDIO_IO_DeInit( void );
void AUDIO_IO_Write( uint8_t Addr, uint16_t Reg, uint16_t Value );
uint16_t AUDIO_IO_Read( uint8_t Addr, uint16_t Reg );
void AUDIO_IO_Delay( uint32_t Delay );

/* I2C EEPROM IO function */
void EEPROM_IO_Init( void );
HAL_StatusTypeDef EEPROM_IO_WriteData( uint16_t DevAddress, uint16_t MemAddress, uint8_t* pBuffer, uint32_t BufferSize );
HAL_StatusTypeDef EEPROM_IO_ReadData( uint16_t DevAddress, uint16_t MemAddress, uint8_t* pBuffer, uint32_t BufferSize );
HAL_StatusTypeDef EEPROM_IO_IsDeviceReady( uint16_t DevAddress, uint32_t Trials );

/* TouchScreen (TS) IO functions */
void TS_IO_Init( void );
void TS_IO_Write( uint8_t Addr, uint8_t Reg, uint8_t Value );
uint8_t TS_IO_Read( uint8_t Addr, uint8_t Reg );
uint16_t TS_IO_ReadMultiple( uint8_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length );
void TS_IO_WriteMultiple( uint8_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length );
void TS_IO_Delay( uint32_t Delay );

/**
 * @}
 */

/** @defgroup STM32412G_DISCOVERY_LOW_LEVEL_Private_Functions STM32412G Discovery Low Level Private Functions
 * @{
 */

/*******************************************************************************
 BUS OPERATIONS
 *******************************************************************************/

/******************************* I2C Routines *********************************/
/**
 * @brief  Initializes I2C MSP.
 * @param  i2c_handler : I2C handler
 */
static void I2Cx_MspInit( I2C_HandleTypeDef *i2c_handler )
{
    GPIO_InitTypeDef gpio_init_structure;

    if ( i2c_handler == (I2C_HandleTypeDef*) (&hI2cAudioHandler) )
         {
        /* AUDIO I2C MSP init */

        /*** Configure the GPIOs ***/
        /* Enable GPIO clock */
        DISCOVERY_AUDIO_I2Cx_SCL_SDA_GPIO_CLK_ENABLE()
        ;

        /* Configure I2C Tx as alternate function */
        gpio_init_structure.Pin = DISCOVERY_AUDIO_I2Cx_SCL_PIN;
        gpio_init_structure.Mode = GPIO_MODE_AF_OD;
        gpio_init_structure.Pull = GPIO_NOPULL;
        gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
        gpio_init_structure.Alternate = DISCOVERY_AUDIO_I2Cx_SCL_SDA_AF;
        HAL_GPIO_Init( DISCOVERY_AUDIO_I2Cx_SCL_SDA_GPIO_PORT, &gpio_init_structure );

        /* Configure I2C Rx as alternate function */
        gpio_init_structure.Pin = DISCOVERY_AUDIO_I2Cx_SDA_PIN;
        HAL_GPIO_Init( DISCOVERY_AUDIO_I2Cx_SCL_SDA_GPIO_PORT, &gpio_init_structure );

        /*** Configure the I2C peripheral ***/
        /* Enable I2C clock */
        DISCOVERY_AUDIO_I2Cx_CLK_ENABLE()
        ;

        /* Force the I2C peripheral clock reset */
        DISCOVERY_AUDIO_I2Cx_FORCE_RESET();

        /* Release the I2C peripheral clock reset */
        DISCOVERY_AUDIO_I2Cx_RELEASE_RESET();

        /* Enable and set I2Cx Interrupt to a lower priority */
        HAL_NVIC_SetPriority( DISCOVERY_AUDIO_I2Cx_EV_IRQn, 0x0F, 0x00 );
        HAL_NVIC_EnableIRQ( DISCOVERY_AUDIO_I2Cx_EV_IRQn );

        /* Enable and set I2Cx Interrupt to a lower priority */
        HAL_NVIC_SetPriority( DISCOVERY_AUDIO_I2Cx_ER_IRQn, 0x0F, 0x00 );
        HAL_NVIC_EnableIRQ( DISCOVERY_AUDIO_I2Cx_ER_IRQn );
    }
    else
    {
        /* External and Arduino connector I2C MSP init */

        /*** Configure the GPIOs ***/
        /* Enable GPIO clock */
        DISCOVERY_EXT_I2Cx_SCL_SDA_GPIO_CLK_ENABLE()
        ;

        /* Configure I2C Tx as alternate function */
        gpio_init_structure.Pin = DISCOVERY_EXT_I2Cx_SCL_PIN;
        gpio_init_structure.Mode = GPIO_MODE_AF_OD;
        gpio_init_structure.Pull = GPIO_NOPULL;
        gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
        gpio_init_structure.Alternate = DISCOVERY_EXT_I2Cx_SCL_AF;
        HAL_GPIO_Init( DISCOVERY_EXT_I2Cx_SCL_SDA_GPIO_PORT, &gpio_init_structure );

        /* Configure I2C Rx as alternate function */
        gpio_init_structure.Pin = DISCOVERY_EXT_I2Cx_SDA_PIN;
        gpio_init_structure.Alternate = DISCOVERY_EXT_I2Cx_SDA_AF;
        HAL_GPIO_Init( DISCOVERY_EXT_I2Cx_SCL_SDA_GPIO_PORT, &gpio_init_structure );

        /*** Configure the I2C peripheral ***/
        /* Enable I2C clock */
        DISCOVERY_EXT_I2Cx_CLK_ENABLE()
        ;

        /* Force the I2C peripheral clock reset */
        DISCOVERY_EXT_I2Cx_FORCE_RESET();

        /* Release the I2C peripheral clock reset */
        DISCOVERY_EXT_I2Cx_RELEASE_RESET();

        /* Enable and set I2Cx Interrupt to a lower priority */
        HAL_NVIC_SetPriority( DISCOVERY_EXT_I2Cx_EV_IRQn, 0x0F, 0x00 );
        HAL_NVIC_EnableIRQ( DISCOVERY_EXT_I2Cx_EV_IRQn );

        /* Enable and set I2Cx Interrupt to a lower priority */
        HAL_NVIC_SetPriority( DISCOVERY_EXT_I2Cx_ER_IRQn, 0x0F, 0x00 );
        HAL_NVIC_EnableIRQ( DISCOVERY_EXT_I2Cx_ER_IRQn );
    }
}

/**
 * @brief  Initializes I2C HAL.
 * @param  i2c_handler : I2C handler
 */
static void I2Cx_Init( I2C_HandleTypeDef *i2c_handler )
{
//    if ( HAL_I2C_GetState( i2c_handler ) == HAL_I2C_STATE_RESET ) {
//    if (i2c_handler == (I2C_HandleTypeDef*)(&hI2cAudioHandler))
//    {
//      /* Audio and LCD I2C configuration */
//      i2c_handler->Instance = DISCOVERY_AUDIO_I2Cx;
//    }
//    else
//    {
//      /* External, EEPROM and Arduino connector I2C configuration */
//      i2c_handler->Instance = DISCOVERY_EXT_I2Cx;
//    }
//    i2c_handler->Init.ClockSpeed       = DISCOVERY_I2C_SPEED;
//    i2c_handler->Init.DutyCycle        = I2C_DUTYCYCLE_2;
//    i2c_handler->Init.OwnAddress1      = 0;
//    i2c_handler->Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
//    i2c_handler->Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
//    i2c_handler->Init.OwnAddress2      = 0;
//    i2c_handler->Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
//    i2c_handler->Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

        i2c_handler->Instance = I2C1;
        i2c_handler->Init.ClockSpeed = 100000;
        i2c_handler->Init.DutyCycle = I2C_DUTYCYCLE_2;
        i2c_handler->Init.OwnAddress1 = 0;
        i2c_handler->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
        i2c_handler->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
        i2c_handler->Init.OwnAddress2 = 0;
        i2c_handler->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
        i2c_handler->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

        /* Init the I2C */
        I2Cx_MspInit( i2c_handler );
        HAL_I2C_MspInit( i2c_handler );
        HAL_I2C_Init( i2c_handler );
//    }
}

/**
 * @brief  Reads multiple data.
 * @param  i2c_handler : I2C handler
 * @param  Addr: I2C address
 * @param  Reg: Reg address
 * @param  MemAddress: Memory address
 * @param  Buffer: Pointer to data buffer
 * @param  Length: Length of the data
 * @retval HAL status
 */
static HAL_StatusTypeDef I2Cx_ReadMultiple( I2C_HandleTypeDef *i2c_handler,
                                            uint8_t Addr,
                                            uint16_t Reg,
                                            uint16_t MemAddress,
                                            uint8_t *Buffer,
                                            uint16_t Length )
{
    HAL_StatusTypeDef status = HAL_OK;

    status = HAL_I2C_Mem_Read( i2c_handler, Addr, (uint16_t) Reg, MemAddress, Buffer, Length, 1000 );

    /* Check the communication status */
    if ( status != HAL_OK )
         {
        /* I2C error occurred */
        I2Cx_Error( i2c_handler, Addr );
    }
    return status;
}

/**
 * @brief  Writes a value in a register of the device through BUS in using DMA mode.
 * @param  i2c_handler : I2C handler
 * @param  Addr: Device address on BUS Bus.
 * @param  Reg: The target register address to write
 * @param  MemAddress: Memory address
 * @param  Buffer: The target register value to be written
 * @param  Length: buffer size to be written
 * @retval HAL status
 */
static HAL_StatusTypeDef I2Cx_WriteMultiple( I2C_HandleTypeDef *i2c_handler,
                                             uint8_t Addr,
                                             uint16_t Reg,
                                             uint16_t MemAddress,
                                             uint8_t *Buffer,
                                             uint16_t Length )
{
    HAL_StatusTypeDef status = HAL_OK;

    status = HAL_I2C_Mem_Write( i2c_handler, Addr, (uint16_t) Reg, MemAddress, Buffer, Length, 1000 );

    /* Check the communication status */
    if ( status != HAL_OK )
         {
        /* Re-Initialize the I2C Bus */
        I2Cx_Error( i2c_handler, Addr );
    }
    return status;
}

/**
 * @brief  Checks if target device is ready for communication.
 * @note   This function is used with Memory devices
 * @param  i2c_handler : I2C handler
 * @param  DevAddress: Target device address
 * @param  Trials: Number of trials
 * @retval HAL status
 */
//static HAL_StatusTypeDef I2Cx_IsDeviceReady( I2C_HandleTypeDef *i2c_handler, uint16_t DevAddress, uint32_t Trials )
//{
//    return (HAL_I2C_IsDeviceReady( i2c_handler, DevAddress, Trials, 1000 ));
//}

/**
 * @brief  Manages error callback by re-initializing I2C.
 * @param  i2c_handler : I2C handler
 * @param  Addr: I2C Address
 */
static void I2Cx_Error( I2C_HandleTypeDef *i2c_handler, uint8_t Addr )
{
    /* De-initialize the I2C communication bus */
    HAL_I2C_DeInit( i2c_handler );

    /* Re-Initialize the I2C communication bus */
    I2Cx_Init( i2c_handler );
}

/**
 * @brief  Deinitializes I2C interface
 * @param  i2c_handler : I2C handler
 */
static void I2Cx_DeInit( I2C_HandleTypeDef *i2c_handler )
{
    if ( i2c_handler == (I2C_HandleTypeDef*) (&hI2cAudioHandler) )
         {
        /* Audio and LCD I2C configuration */
        i2c_handler->Instance = DISCOVERY_AUDIO_I2Cx;
    }
    else
    {
        /* External, EEPROM and Arduino connector I2C configuration */
        i2c_handler->Instance = DISCOVERY_EXT_I2Cx;
    }

    /* Disable I2C block */
    __HAL_I2C_DISABLE( i2c_handler );

    /* DeInit the I2S */
    HAL_I2C_DeInit( i2c_handler );
}

/*************************** FMC Routines ************************************/
/**
 * @brief  Initializes FMC_BANK1 MSP.
 */
static void FMC_BANK1_MspInit( void )
{
    GPIO_InitTypeDef gpio_init_structure;

    /* Enable FSMC clock */
    __HAL_RCC_FSMC_CLK_ENABLE( );

    /* Enable GPIOs clock */
    __HAL_RCC_GPIOD_CLK_ENABLE()
    ;
    __HAL_RCC_GPIOE_CLK_ENABLE()
    ;
    __HAL_RCC_GPIOF_CLK_ENABLE( );

    /* Common GPIO configuration */
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull = GPIO_PULLUP;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init_structure.Alternate = GPIO_AF12_FSMC;

    /* GPIOD configuration *//* GPIO_PIN_7 is  FMC_NE1 */
    gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 |\
 GPIO_PIN_9
                              | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_7;

    HAL_GPIO_Init( GPIOD, &gpio_init_structure );

    /* GPIOE configuration */
    gpio_init_structure.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |\
 GPIO_PIN_12
                              | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init( GPIOE, &gpio_init_structure );
    
    /* GPIOF configuration */
    gpio_init_structure.Pin = GPIO_PIN_0;
    HAL_GPIO_Init( GPIOF, &gpio_init_structure );
}

/**
 * @brief  Initializes LCD IO.
 */
static void FMC_BANK1_Init( void )
{
    SRAM_HandleTypeDef hsram;
    FMC_NORSRAM_TimingTypeDef sram_timing;
    FMC_NORSRAM_TimingTypeDef sram_timing_write;

    /*** Configure the SRAM Bank 1 ***/
    /* Configure IPs */
    hsram.Instance = FSMC_NORSRAM_DEVICE;
    hsram.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;

    /* Timing for READING */
    sram_timing.AddressSetupTime = 9;
    sram_timing.AddressHoldTime = 1;
    sram_timing.DataSetupTime = 36;
    sram_timing.BusTurnAroundDuration = 1;
    sram_timing.CLKDivision = 2;
    sram_timing.DataLatency = 2;
    sram_timing.AccessMode = FSMC_ACCESS_MODE_A;

    /* Timing for WRITTING*/
    sram_timing_write.AddressSetupTime = 1;
    sram_timing_write.AddressHoldTime = 1;
    sram_timing_write.DataSetupTime = 7;
    sram_timing_write.BusTurnAroundDuration = 0;
    sram_timing_write.CLKDivision = 2;
    sram_timing_write.DataLatency = 2;
    sram_timing_write.AccessMode = FSMC_ACCESS_MODE_A;

    hsram.Init.NSBank = FSMC_NORSRAM_BANK1;
    hsram.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
    hsram.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
    hsram.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
    hsram.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
    hsram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
    hsram.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
    hsram.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
    hsram.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
    hsram.Init.ExtendedMode = FSMC_EXTENDED_MODE_ENABLE;
    hsram.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
    hsram.Init.WriteFifo = FSMC_WRITE_FIFO_DISABLE;
    hsram.Init.PageSize = FSMC_PAGE_SIZE_NONE;
    hsram.Init.ContinuousClock = FSMC_CONTINUOUS_CLOCK_SYNC_ONLY;

    /* Initialize the SRAM controller */
    FMC_BANK1_MspInit( );
    HAL_SRAM_Init( &hsram, &sram_timing, &sram_timing_write );
}

/**
 * @brief  Writes register value.
 * @param  Data: Data to be written
 */
static void FMC_BANK1_WriteData( uint16_t Data )
{
    /* Write 16-bit Reg */
    FMC_BANK1->RAM = Data;
    __DSB( );
}

/**
 * @brief  Writes register address.
 * @param  Reg: Register to be written
 */
static void FMC_BANK1_WriteReg( uint8_t Reg )
{
    /* Write 16-bit Index, then write register */
    FMC_BANK1->REG = Reg;
    __DSB( );
}

/**
 * @brief  Reads register value.
 * @retval Read value
 */
static uint16_t FMC_BANK1_ReadData( void )
{
    return FMC_BANK1->RAM;
}

/*******************************************************************************
 LINK OPERATIONS
 *******************************************************************************/

/********************************* LINK LCD ***********************************/

/**
 * @brief  Initializes LCD low level.
 */
void LCD_IO_Init( void )
{
    FMC_BANK1_Init( );
}

/**
 * @brief  Writes data on LCD data register.
 * @param  RegValue: Data to be written
 */
void LCD_IO_WriteData( uint16_t RegValue )
{
    /* Write 16-bit Reg */
    FMC_BANK1_WriteData( RegValue );
}

/**
 * @brief  Writes several data on LCD data register.
 * @param  pData: pointer on data to be written
 * @param  Size: data amount in 16bits short unit
 */
void LCD_IO_WriteMultipleData( uint16_t *pData, uint32_t Size )
{
    uint32_t i;

    for ( i = 0; i < Size; i++ )
        {
        FMC_BANK1_WriteData( pData[i] );
    }
}

/**
 * @brief  Writes register on LCD register.
 * @param  Reg: Register to be written
 */
void LCD_IO_WriteReg( uint8_t Reg )
{
    /* Write 16-bit Index, then Write Reg */
    FMC_BANK1_WriteReg( Reg );
}

/**
 * @brief  Reads data from LCD data register.
 * @retval Read data.
 */
uint16_t LCD_IO_ReadData( void )
{
    return FMC_BANK1_ReadData( );
}

/**
 * @brief  LCD delay
 * @param  Delay: Delay in ms
 */
void LCD_IO_Delay( uint32_t Delay )
{
    HAL_Delay( Delay );
}

/********************************* LINK AUDIO *********************************/

/**
 * @brief  Initializes Audio low level.
 */
void AUDIO_IO_Init( void )
{
    I2Cx_Init( &hI2cAudioHandler );
}

/**
 * @brief  Deinitializes Audio low level.
 */
void AUDIO_IO_DeInit( void )
{
    I2Cx_DeInit( &hI2cAudioHandler );
}

/**
 * @brief  Writes a single data.
 * @param  Addr: I2C address
 * @param  Reg: Reg address
 * @param  Value: 9-bit data to be written
 */
void AUDIO_IO_Write( uint8_t Addr, uint16_t Reg, uint16_t Value )
{
    uint8_t lowbyte = (uint8_t)Value;
    uint8_t hibyte = (uint8_t)(Value >> 8);

    // 12.9.1.2. 2-WIRE Write Operation, the 9th bit is included in the register address byte.
    uint16_t reg_write = ((uint16_t)( Reg << 1 ) | (hibyte & 0x1));

    I2Cx_WriteMultiple( &hI2cAudioHandler, Addr, reg_write, I2C_MEMADD_SIZE_8BIT, &lowbyte, 1 );

}

/**
 * @brief  Reads a single data.
 * @param  Addr: I2C address
 * @param  Reg: Reg address
 * @retval 9-bit data to be read
 */
uint16_t AUDIO_IO_Read( uint8_t Addr, uint16_t Reg )
{
    uint8_t read_value[2] = {0,0};

    I2Cx_ReadMultiple( &hI2cAudioHandler, Addr, (uint16_t)( Reg << 1 ), I2C_MEMADD_SIZE_8BIT, read_value, 2 );

    // flip the bits into little endian
    uint16_t tmp = read_value[1] + (((uint16_t)read_value[0]) << 8);

    return tmp;

}

/**
 * @brief  AUDIO Codec delay
 * @param  Delay: Delay in ms
 */
void AUDIO_IO_Delay( uint32_t Delay )
{
    HAL_Delay( Delay );
}

/******************************** LINK I2C EEPROM *****************************/

/************************** LINK TS (TouchScreen) *****************************/
/**
 * @brief  Initializes Touchscreen low level.
 */
void TS_IO_Init( void )
{
    I2Cx_Init( &hI2cAudioHandler );
}

/**
 * @brief  Writes a single data.
 * @param  Addr: I2C address
 * @param  Reg: Reg address
 * @param  Value: Data to be written
 */
void TS_IO_Write( uint8_t Addr, uint8_t Reg, uint8_t Value )
{
    I2Cx_WriteMultiple( &hI2cAudioHandler, Addr, (uint16_t) Reg, I2C_MEMADD_SIZE_8BIT, (uint8_t*) &Value, 1 );
}

/**
 * @brief  Reads a single data.
 * @param  Addr: I2C address
 * @param  Reg: Reg address
 * @retval Data to be read
 */
uint8_t TS_IO_Read( uint8_t Addr, uint8_t Reg )
{
    uint8_t read_value = 0;

    I2Cx_ReadMultiple( &hI2cAudioHandler, Addr, Reg, I2C_MEMADD_SIZE_8BIT, (uint8_t*) &read_value, 1 );

    return read_value;
}

/**
 * @brief  Reads multiple data with I2C communication
 *         channel from TouchScreen.
 * @param  Addr: I2C address
 * @param  Reg: Register address
 * @param  Buffer: Pointer to data buffer
 * @param  Length: Length of the data
 * @retval HAL status
 */
uint16_t TS_IO_ReadMultiple( uint8_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length )
{
    return I2Cx_ReadMultiple( &hI2cAudioHandler, Addr, (uint16_t) Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length );
}

/**
 * @brief  Writes multiple data with I2C communication
 *         channel from MCU to TouchScreen.
 * @param  Addr: I2C address
 * @param  Reg: Register address
 * @param  Buffer: Pointer to data buffer
 * @param  Length: Length of the data
 */
void TS_IO_WriteMultiple( uint8_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length )
{
    I2Cx_WriteMultiple( &hI2cAudioHandler, Addr, (uint16_t) Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length );
}

/**
 * @brief  Delay function used in TouchScreen low level driver.
 * @param  Delay: Delay in ms
 */
void TS_IO_Delay( uint32_t Delay )
{
    HAL_Delay( Delay );
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

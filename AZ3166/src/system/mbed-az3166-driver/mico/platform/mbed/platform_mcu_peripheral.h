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

#ifndef __PLATFORM_MCU_PERIPHERAL_H__
#define __PLATFORM_MCU_PERIPHERAL_H__


#include "stm32f4xx.h"
#include "mico_rtos.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

//  /* GPIOA to I */
// #define NUMBER_OF_GPIO_PORTS      (8)

/* Interrupt line 0 to 15. Each line is shared among the same numbered pins across all GPIO ports */
#define NUMBER_OF_GPIO_IRQ_LINES  (16)

// /* USART1 to 6 */
// #define NUMBER_OF_UART_PORTS      (6)


// /* Invalid UART port number */
// #define INVALID_UART_PORT_NUMBER  (0xff)

//  /* SPI1 to SPI3 */
// #define NUMBER_OF_SPI_PORTS       (3)

/******************************************************
 *                   Enumerations
 ******************************************************/
 
/******************************************************
 *                 Type Definitions
 ******************************************************/

// /* GPIO port */
// typedef GPIO_TypeDef  platform_gpio_port_t;

// /* UART port */
// typedef USART_TypeDef platform_uart_port_t;

// /* SPI port */
// typedef SPI_TypeDef   platform_spi_port_t;

/* QSPI port */
#if defined(STM32F412Rx) || defined(STM32F412Vx) || defined(STM32F412Zx) || defined(STM32F469xx) || defined(STM32F479xx)
typedef QUADSPI_TypeDef   platform_qspi_port_t;
#endif

// /* I2C port */
// typedef I2C_TypeDef   platform_i2c_port_t;

// /* GPIO alternate function */
// typedef uint8_t       platform_gpio_alternate_function_t;

/* Peripheral clock function */
typedef void (*platform_peripheral_clock_function_t)(uint32_t clock, FunctionalState state );

// typedef DMA_TypeDef     dma_registers_t;
// typedef FunctionalState functional_state_t;
// typedef uint32_t        peripheral_clock_t;

/******************************************************
 *                    Structures
 ******************************************************/
typedef struct
{
    DMA_TypeDef*        controller;
    DMA_Stream_TypeDef* stream;
    uint32_t            channel;
    IRQn_Type           irq_vector;
    uint32_t            complete_flags;
    uint32_t            error_flags;
} platform_dma_config_t;


#if defined(STM32F412Rx) || defined(STM32F412Vx) || defined(STM32F412Zx) || defined(STM32F469xx) || defined(STM32F479xx)
typedef struct
{
    platform_qspi_port_t* port;
    uint32_t FlashID;
    const platform_gpio_t* pin_d0;
    const platform_gpio_t* pin_d1;
    const platform_gpio_t* pin_d2;
    const platform_gpio_t* pin_d3;
    const platform_gpio_t* pin_clock;
    const platform_gpio_t* pin_cs;
    platform_dma_config_t dma;
    uint8_t gpio_af_d0;
    uint8_t gpio_af_d1;
    uint8_t gpio_af_d2;
    uint8_t gpio_af_d3;
    uint8_t gpio_af_clk;
    uint8_t gpio_af_cs;
}platform_qspi_t;
#endif


typedef struct
{
     const platform_flash_t*    peripheral;
     mico_mutex_t               flash_mutex;
     volatile bool              initialized;
} platform_flash_driver_t;

/******************************************************
 *                 Global Variables
 ******************************************************/


/******************************************************
 *               Function Declarations
 ******************************************************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif




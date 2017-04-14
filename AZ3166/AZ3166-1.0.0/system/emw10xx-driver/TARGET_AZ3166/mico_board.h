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

#ifndef __MICO_BOARD_H__
#define __MICO_BOARD_H__

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
  

   
/******************************************************
 *                   Enumerations
 ******************************************************/

/*
EMW3239 platform pin definitions ...
+-------------------------------------------------------------------------+
| Enum ID       |Pin | STM32| Peripheral  |    Board     |   Peripheral   |
|               | #  | Port | Available   |  Connection  |     Alias      |
|---------------+----+------+-------------+--------------+----------------|
|               | 1  | NC   |             |              |                |
|---------------+----+------+-------------+--------------+----------------|
| MICO_GPIO_2   | 2  | B  2 |   GPIO      |              |                |
|---------------+----+------+-------------+--------------+----------------|
|               | 3  |  NC  |             |              |                |
|---------------+----+------+-------------+--------------+----------------|
| MICO_GPIO_4   | 4  | B 15 | TIM1_CH3N   |              |                |
|               |    |      | TIM8_CH3N   |              |                |
|               |    |      | SPI2_MOSI   |              |                |
|               |    |      | SDIO_CK     |              |                |
|---------------+----+------+-------------+--------------+----------------|
| MICO_GPIO_5   | 5  | B 12 | SPI2_NSS    |              |                |
|               |    |      | SPI4_NSS    |              |                |        
|---------------+----+------+-------------+--------------+----------------|
| MICO_GPIO_6   | 6  | B 13 | TIM1_CH1N   |              |                |
|               |    |      | GPIO        |              |                |        
|               |    |      | SPI2_SCK    |              |                |
|---------------+----+------+-------------+--------------+----------------|
| MICO_GPIO_7   | 7  | B 14 | GPIO        |              |                |
|               |    |      | SDIO_D6     |              |                |
|               |    |      | TIM1_CH2N   |              |                |
|               |    |      | SPI2_MISO   |              |                |
|---------------+----+------+-------------+--------------+----------------|
| MICO_GPIO_8   | 8  | C  6 | TIM3_CH1    | STDIO_UART_TX| MICO_UART_1_TX |
|               |    |      | TIM8_CH1    |              |                |
|               |    |      | USART6_TX   |              |                |
|               |    |      | GPIO        |              |                |
|---------------+----+------+-------------+--------------+----------------|
| MICO_GPIO_9   | 9  | A 15 | TIM2_CH1    |EasyLink_BUTTON|               |
|               |    |      | JTDI        |              |                |
|               |    |      | USART1_TX   |              |                |
|               |    |      | GPIO        |              |                |
|---------------+----+------+-------------+--------------+----------------|
|               | 10 | VBAT |             |
|---------------+----+------+-------------+--------------+----------------|
|               | 11 | NC   |             |              |                |
|---------------+----+------+-------------+--------------+----------------|
| MICO_GPIO_12  | 12 | C  7 | TIM3_CH2    | STDIO_UART_RX| MICO_UART_1_RX |
|               |    |      | TIM8_CH2    |              |                |
|               |    |      | SPI2_SCK    |              |                |
|               |    |      | SDIO_D7     |              |                |
|               |    |      | USART6_RX   |              |                |
|               |    |      | GPIO        |              |                |
|---------------+----+------+-------------+--------------+----------------|
|               | 13 | NRST |             |              |  MICRO_RST_N   |
|---------------+----+------+-------------+--------------+----------------|
| MICO_GPIO_14  | 14 | C 0  | WAKE_UP     |              |                |
|---------------+----+------+-------------+--------------+----------------|
|               | 15 | NC   |             |              |                |
|---------------+----+------+-------------+--------------+----------------|
| MICO_GPIO_16  | 16 | C 13 |     -       |              |                |
|---------------+----+------+-------------+--------------+----------------|
| MICO_SYS_LED  | 17 | B  8 |  TIM4_CH3   |              |                |
|               |    |      |  I2C2_SCL   |              |                |
|               |    |      |  GPIO       |              |                |
|---------------+----+------+-------------+--------------+----------------|
| MICO_GPIO_18  | 18 | B  9 | TIM4_CH3    |              |                |
|               |    |      | TIM10_CH1   |              |                |
|               |    |      | I2C1_SCL    |              |                |
|               |    |      | SDIO_D4     |              |                |
|               |    |      | GPIO        |              |                |
+---------------+----+--------------------+--------------+----------------+
| MICO_GPIO_19  | 19 | B 10 | GPIO        |              |                |
+---------------+----+--------------------+--------------+----------------+
|               | 20 | GND  |             |              |                |
+---------------+----+--------------------+--------------+----------------+
|               | 21 | GND  |             |              |                |
+---------------+----+--------------------+--------------+----------------+
| MICO_GPIO_22  | 22 | B  3 |             |              |                |
+---------------+----+--------------------+--------------+----------------+
| MICO_GPIO_23  | 23 | A 15 | GPIO        |              |  JTAG_TDI      |
|               |    |      | USART1_TX   |              |  SPI1_SSN      |
|               |    |      | TIM2_CH1    |              |                |
|               |    |      | TIM2_ETR    |              |                |
+---------------+----+--------------------+--------------+----------------+
| MICO_GPIO_24  | 24 | B  4 |             |              |                |
+---------------+----+--------------------+--------------+----------------+
| MICO_GPIO_25  | 25 | A 14 | JTCK-SWCLK  |  SWCLK       |                |
|               |    |      |  GPIO       |              |                |  
+---------------+----+--------------------+--------------+----------------+
|MICO_GPIO_26   | 26 | A 13 | JTMS-SWDIO  |  SWDIO       |                |
|               |    |      |  GPIO       |              |                |    
+---------------+----+--------------------+--------------+----------------+
|MICO_GPIO_27   | 27 | B  3 | TIM1_ETR    |              |                |
|               |    |      | USART1_RX   |              |                |         
|               |    |      | GPIO        |              |                |    
+---------------+----+--------------------+--------------+----------------+
|               | 28 | NC   |             |              |                |
+---------------+----+--------------------+--------------+----------------+
| MICO_GPIO_29  | 29 | B  7 | GPIO        |              | MICO_UART_2_RX |
|               |    |      | TIM4_CH2    |              |                |
|               |    |      | USART1_RX   |              |                |
|               |    |      | I2C1_SDA    |              |                |
+---------------+----+--------------------+--------------+----------------+
| MICO_GPIO_30  | 30 | B  6 | GPIO        |              | MICO_UART_2_TX |
|               |    |      | TIM4_CH1    |              |                |
|               |    |      | USART1_TX   |              |                |
|               |    |      | I2C1_SCL    |              |                |
+---------------+----+--------------------+--------------+----------------+
| MICO_GPIO_31  | 31 | B  4 | GPIO        | MICO_RF_LED  |                |
|               |    |      | TIM3_CH1    |              |                |
|               |    |      | SDIO_D0     |              |                |  
+---------------+----+--------------------+--------------+----------------+
|               | 32 |  NC  |             |              |                | 
+---------------+----+--------------------+--------------+----------------+  
| MICO_GPIO_33  | 33 | A 10 | TIM1_CH3    | MICO_SYS_LED |                |  
|               |    |      | SPI5_MOSI   |              |                |  
|               |    |      | USB_FS_ID   |              |                | 
|               |    |      | GPIO        |              |                | 
+---------------+----+--------------------+--------------+----------------+  
| MICO_GPIO_34  | 34 | A 12 | TIM1_ETR    |              |                |  
|               |    |      | USART1_RTS  |              |                |
|               |    |      | USB_FS_DP   |              |                |  
|               |    |      | GPIO        |              |                |
+---------------+----+--------------------+--------------+----------------+  
| MICO_GPIO_35  | 35 | A 11 | TIM1_CH4    |              |                | 
|               |    |      | SPI4_MISO   |              |                |  
|               |    |      | USART1_CTS  |              |                |  
|               |    |      | USART6_TX   |              |                |  
|               |    |      | USB_FS_DM   |              |                |  
|               |    |      | GPIO        |              |                |
+---------------+----+--------------------+--------------+----------------+  
| MICO_GPIO_36  | 36 | A  5 | TIM2_CH1    | BOOT_SEL     |                |  
|               |    |      | TIM2_ETR    |              |                |
|               |    |      | TIM8_CH1N   |              |                |  
|               |    |      | SPI1_SCK    |              |                |
|               |    |      | GPIO        |              |                | 
+---------------+----+--------------------+--------------+----------------+  
| MICO_GPIO_37  | 37 | B  0 | TIM1_CH2N   | MFG_SEL      |                |  
|               |    |      | TIM3_CH3    |              |                |
|               |    |      | TIM8_CH2N   |              |                |  
|               |    |      | GPIO        |              |                | 
+---------------+----+--------------------+--------------+----------------+  
| MICO_GPIO_38  | 38 | A  4 | USART2_CK   |              |                | 
|               |    |      | GPIO        |              |                | 
+---------------+----+--------------------+--------------+----------------+  
|               | 39 | VDD  |             |              |                | 
+---------------+----+--------------------+--------------+----------------+  
|               | 40 | VDD  |             |              |                |
+---------------+----+--------------------+--------------+----------------+  
|               | 41 | ANT  |             |              |                |
+---------------+----+--------------------+--------------+----------------+ 
*/

typedef enum
{
    FLASH_PIN_QSPI_CS,
    FLASH_PIN_QSPI_CLK,
    FLASH_PIN_QSPI_D0,
    FLASH_PIN_QSPI_D1,
    FLASH_PIN_QSPI_D2,
    FLASH_PIN_QSPI_D3,
    
    MICO_GPIO_2,
    MICO_GPIO_4,
    MICO_GPIO_5,
    MICO_GPIO_6,
    MICO_GPIO_7,
    MICO_GPIO_8,
    MICO_GPIO_9,
    MICO_GPIO_12,
    MICO_GPIO_14,
    MICO_GPIO_16,
    MICO_GPIO_17,
    MICO_GPIO_18,
    MICO_GPIO_19,
    MICO_GPIO_27,  
    MICO_GPIO_29,
    MICO_GPIO_30,
    MICO_GPIO_31,
    MICO_GPIO_33,
    MICO_GPIO_34,
    MICO_GPIO_35,
    MICO_GPIO_36,
    MICO_GPIO_37,
    MICO_GPIO_38,
    MICO_GPIO_MAX, /* Denotes the total number of GPIO port aliases. Not a valid GPIO alias */
    MICO_GPIO_NONE,
} mico_gpio_t;

typedef enum
{
  MICO_SPI_1,
  MICO_SPI_MAX, /* Denotes the total number of SPI port aliases. Not a valid SPI alias */
  MICO_SPI_NONE,
} mico_spi_t;

typedef enum
{
  MICO_QSPI_1,
  MICO_QSPI_MAX,/* Denotes the total number of QSPI port aliases. Not a valid QSPI alias */
  MICO_QSPI_NONE,
}mico_qspi_t;

typedef enum
{
  MICO_I2C_1,
  MICO_I2C_MAX, /* Denotes the total number of I2C port aliases. Not a valid I2C alias */
  MICO_I2C_NONE,
} mico_i2c_t;

typedef enum
{
    MICO_PWM_MAX, /* Denotes the total number of PWM port aliases. Not a valid PWM alias */
    MICO_PWM_NONE,
} mico_pwm_t;

typedef enum
{
    MICO_ADC_1,
    MICO_ADC_2,
    MICO_ADC_MAX, /* Denotes the total number of ADC port aliases. Not a valid ADC alias */
    MICO_ADC_NONE,
} mico_adc_t;


typedef enum
{
    MICO_UART_1,
    MICO_UART_2,
    MICO_UART_MAX, /* Denotes the total number of UART port aliases. Not a valid UART alias */
    MICO_UART_NONE,
} mico_uart_t;

typedef enum
{
  MICO_FLASH_EMBEDDED,
  MICO_FLASH_QSPI,
  MICO_FLASH_MAX,
  MICO_FLASH_NONE,
} mico_flash_t;

typedef enum
{
  MICO_PARTITION_FILESYS,
  MICO_PARTITION_USER_MAX
} mico_user_partition_t;

typedef enum
{
    MICO_PARTITION_ERROR = -1,
    MICO_PARTITION_BOOTLOADER = MICO_PARTITION_USER_MAX,
    MICO_PARTITION_APPLICATION,
    MICO_PARTITION_ATE,
    MICO_PARTITION_OTA_TEMP,
    MICO_PARTITION_RF_FIRMWARE,
    MICO_PARTITION_PARAMETER_1,
    MICO_PARTITION_PARAMETER_2,
#ifdef MICO_USE_BT_PARTITION
    MICO_PARTITION_BT_FIRMWARE,
#endif
    MICO_PARTITION_MAX,
    MICO_PARTITION_NONE,
} mico_partition_t;


#ifdef BOOTLOADER
#define MICO_STDIO_UART          (MICO_UART_1)
#define MICO_STDIO_UART_BAUDRATE (115200)
#else
#define MICO_STDIO_UART          (MICO_UART_1)
#define MICO_STDIO_UART_BAUDRATE (115200) 
#endif

#define MICO_UART_FOR_APP        (MICO_UART_2)
#define MICO_MFG_TEST            (MICO_UART_2)
#define MICO_CLI_UART            (MICO_UART_1)

/* Components connected to external I/Os*/
#define USE_QUAD_SPI_FLASH
//#define USE_QUAD_SPI_DMA

#define BOOT_SEL            (MICO_GPIO_NONE)
#define MFG_SEL             (MICO_GPIO_NONE)
#define EasyLink_BUTTON     (MICO_GPIO_NONE)
#define MICO_SYS_LED        (MICO_GPIO_NONE)
#define MICO_RF_LED         (MICO_GPIO_NONE)

/* Arduino extention connector */
#define Arduino_RXD         (MICO_GPIO_29)
#define Arduino_TXD         (MICO_GPIO_30)
#define Arduino_D2          (MICO_GPIO_NONE)
#define Arduino_D3          (MICO_GPIO_NONE)
#define Arduino_D4          (MICO_GPIO_19)
#define Arduino_D5          (MICO_GPIO_16)
#define Arduino_D6          (MICO_GPIO_14)
#define Arduino_D7          (MICO_GPIO_NONE)

#define Arduino_D8          (MICO_GPIO_2)
#define Arduino_D9          (MICO_GPIO_27)
#define Arduino_CS          (MICO_GPIO_5)
#define Arduino_SI          (MICO_GPIO_4)
#define Arduino_SO          (MICO_GPIO_7)
#define Arduino_SCK         (MICO_GPIO_6)
#define Arduino_SDA         (MICO_GPIO_18)
#define Arduino_SCL         (MICO_GPIO_17)

#define Arduino_A0          (MICO_ADC_NONE)
#define Arduino_A1          (MICO_ADC_NONE)
#define Arduino_A2          (MICO_ADC_1)
#define Arduino_A3          (MICO_ADC_2)
#define Arduino_A4          (MICO_ADC_NONE)
#define Arduino_A5          (MICO_ADC_NONE)

#define Arduino_I2C         (MICO_I2C_1)
#define Arduino_SPI         (MICO_SPI_1)
#define Arduino_UART        (MICO_UART_2)

// #define USE_MiCOKit_EXT

// #ifdef USE_MiCOKit_EXT
// #define MICO_I2C_CP         (Arduino_I2C)
// #include "micokit_ext_def.h"
// #else
// #define MICO_I2C_CP         (MICO_I2C_NONE)
// #endif //USE_MiCOKit_EXT


void mico_board_init(void);


#ifdef __cplusplus
} /*extern "C" */
#endif

#endif

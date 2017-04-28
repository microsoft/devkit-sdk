/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "variant.h"


#ifdef __cplusplus
extern "C" {
#endif

/*
 * Pins descriptions
 */
extern const PinDescription g_APinDescription[]=

//  arduino_id  |     ulPin    |   ulPort | mode               |          configured
{
  { ARDUINO_PIN_EXT,   GPIO_PIN_0,  GPIOA, GPIO_PIN_IO|GPIO_PIN_ADC ,                    false },
  { ARDUINO_PIN_EXT,   GPIO_PIN_1,  GPIOA, GPIO_PIN_IO|GPIO_PIN_ADC ,                    false },
  { ARDUINO_PIN_EXT,   GPIO_PIN_2,  GPIOA, GPIO_PIN_IO|GPIO_PIN_UART_TX ,                false },
  { ARDUINO_PIN_EXT,   GPIO_PIN_3,  GPIOA, GPIO_PIN_IO|GPIO_PIN_UART_RX ,                false },
  { ARDUINO_PIN_A2,    GPIO_PIN_4,  GPIOA, GPIO_PIN_IO|GPIO_PIN_ADC,                    false },
  { ARDUINO_PIN_A3,    GPIO_PIN_5,  GPIOA, GPIO_PIN_IO|GPIO_PIN_ADC,                    false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_6,  GPIOA, GPIO_PIN_IO,               false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_7,  GPIOA, GPIO_PIN_IO|GPIO_PIN_PWM,   false },
  { ARDUINO_PIN_EXT,   GPIO_PIN_8,  GPIOA, GPIO_PIN_IO,                                  false },
  { ARDUINO_PIN_EXT,   GPIO_PIN_9,  GPIOA, GPIO_PIN_IO,                                  false },
  { ARDUINO_PIN_EXT,   GPIO_PIN_10, GPIOA, GPIO_PIN_IO,                                  false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_11, GPIOA, GPIO_PIN_MORPHO_ONLY,                         false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_12, GPIOA, GPIO_PIN_MORPHO_ONLY,                         false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_13, GPIOA, GPIO_PIN_MORPHO_ONLY,                         false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_14, GPIOA, GPIO_PIN_MORPHO_ONLY,                         false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_15, GPIOA, GPIO_PIN_MORPHO_ONLY ,                        false },
  { ARDUINO_PIN_EXT,   GPIO_PIN_0,  GPIOB, GPIO_PIN_ADC,                                 false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_1,  GPIOB, GPIO_PIN_MORPHO_ONLY,                         false },
  { ARDUINO_PIN_D8,  GPIO_PIN_2,  GPIOB,  GPIO_PIN_IO,                         false },
  { ARDUINO_PIN_D9,   GPIO_PIN_3,  GPIOB, GPIO_PIN_IO|GPIO_PIN_PWM,                     false },
  { ARDUINO_PIN_EXT,   GPIO_PIN_4,  GPIOB, GPIO_PIN_IO,                     false },
  { ARDUINO_PIN_EXT,   GPIO_PIN_5,  GPIOB, GPIO_PIN_IO,                                  false },
  { ARDUINO_PIN_D1,   GPIO_PIN_6,  GPIOB, GPIO_PIN_IO|GPIO_PIN_SPI_CS,     false },
  { ARDUINO_PIN_D0,   GPIO_PIN_7,  GPIOB, GPIO_PIN_IO,                         false },
  { ARDUINO_PIN_D15,  GPIO_PIN_8,  GPIOB, GPIO_PIN_IO|GPIO_PIN_I2C_SCL,                 false },
  { ARDUINO_PIN_D14,  GPIO_PIN_9,  GPIOB, GPIO_PIN_IO|GPIO_PIN_I2C_SDA,                 false },
  { ARDUINO_PIN_D4,   GPIO_PIN_10, GPIOB, GPIO_PIN_IO|GPIO_PIN_SPI_CS,    false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_11, GPIOB, GPIO_PIN_MORPHO_ONLY|GPIO_PIN_UART_RX,        false },
  { ARDUINO_PIN_D10,  GPIO_PIN_12, GPIOB, GPIO_PIN_IO,                         false },
  { ARDUINO_PIN_D13,  GPIO_PIN_13, GPIOB, GPIO_PIN_IO|GPIO_PIN_SPI_SCK,                         false },
  { ARDUINO_PIN_D12,  GPIO_PIN_14, GPIOB, GPIO_PIN_IO,                         false },
  { ARDUINO_PIN_D11,  GPIO_PIN_15, GPIOB, GPIO_PIN_IO|GPIO_PIN_SPI_MOSI,                         false },
  { ARDUINO_PIN_D6,   GPIO_PIN_0,  GPIOC, GPIO_PIN_IO|GPIO_PIN_ADC,                     false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_1,  GPIOC, GPIO_PIN_IO|GPIO_PIN_ADC,                     false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_2,  GPIOC, GPIO_PIN_MORPHO_ONLY,                         false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_3,  GPIOC, GPIO_PIN_MORPHO_ONLY,                         false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_4,  GPIOC, GPIO_PIN_MORPHO_ONLY,                         false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_5,  GPIOC, GPIO_PIN_MORPHO_ONLY,                         false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_6,  GPIOC, GPIO_PIN_MORPHO_ONLY,                         false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_7,  GPIOC, GPIO_PIN_IO,                     false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_8,  GPIOC, GPIO_PIN_MORPHO_ONLY,                         false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_9,  GPIOC, GPIO_PIN_MORPHO_ONLY,                         false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_10, GPIOC, GPIO_PIN_MORPHO_ONLY,                         false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_11, GPIOC, GPIO_PIN_MORPHO_ONLY,                         false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_12, GPIOC, GPIO_PIN_IO|GPIO_PIN_SPI_MISO,                         false },
  { ARDUINO_PIN_D5,   GPIO_PIN_13, GPIOC, GPIO_PIN_IO,                         false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_14, GPIOC, GPIO_PIN_MORPHO_ONLY,                         false },
  { ARDUINO_PIN_EXT,  GPIO_PIN_15, GPIOC, GPIO_PIN_MORPHO_ONLY,                         false }
} ;

#ifdef __cplusplus
}
#endif

/*
 * UART objects
 */
UARTClass Serial;

/*
* Screen objects
*/
OLEDDisplay Screen;


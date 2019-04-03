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

#include "Arduino.h"
#include "PinNames.h"

/* Types used for the tables below */
typedef struct _PinDescription
{
  uint32_t pin ;
  uint32_t mode;
} PinDescription ;

//defines here the type of the pin
#define GPIO_PIN_MORPHO_ONLY          0x00000000
#define GPIO_PIN_ADC                  0x00000001
#define GPIO_PIN_DAC                  0x00000002
#define GPIO_PIN_PWM                  0x00000004
#define GPIO_PIN_IO                   0x00000008
#define GPIO_PIN_I2C_SCL              0x00000010
#define GPIO_PIN_I2C_SDA              0x00000020
#define GPIO_PIN_SPI_SCK              0x00000100
#define GPIO_PIN_SPI_MISO             0x00000200
#define GPIO_PIN_SPI_MOSI             0x00000400
#define GPIO_PIN_SPI_CS               0x00000800
#define GPIO_PIN_UART_TX              0x00001000
#define GPIO_PIN_UART_RX              0x00002000

/*
 * Pins descriptions
 */
static const PinDescription pinsDescription[]=
//  pin   |                 mode                   
{
  { PB_0,   GPIO_PIN_IO|GPIO_PIN_ADC|GPIO_PIN_SPI_CS  },   // Pin 0
  { PA_5,   GPIO_PIN_IO|GPIO_PIN_ADC                  },   // Pin 4
  { PA_4,   GPIO_PIN_IO|GPIO_PIN_ADC                  },   // Pin 5
  { PB_4,   GPIO_PIN_IO|GPIO_PIN_PWM                  },   // Pin 6
  { PB_3,   GPIO_PIN_IO|GPIO_PIN_PWM                  },   // Pin 7
  { PB_6,   GPIO_PIN_IO|GPIO_PIN_UART_TX              },   // Pin 1
  { PC_13,  GPIO_PIN_IO                               },   // Pin 8
  { PB_10,  GPIO_PIN_IO                               },   // Pin 9
  { PC_7,   GPIO_PIN_IO|GPIO_PIN_PWM                  },   // Pin 10
  { PA_10,  GPIO_PIN_IO                               },   // Pin 11
  { PB_2,   GPIO_PIN_IO                               },   // Pin 12
  { PB_7,   GPIO_PIN_IO|GPIO_PIN_UART_RX              },   // Pin 2
  { PB_13,  GPIO_PIN_IO|GPIO_PIN_SPI_SCK,             },   // Pin 13
  { PB_14,  GPIO_PIN_IO|GPIO_PIN_SPI_MISO,            },   // Pin 14
  { PB_15,  GPIO_PIN_IO|GPIO_PIN_SPI_MOSI,            },   // Pin 15
  { PC_6,   GPIO_PIN_IO,                              },   // Pin 16
  { PB_8,   GPIO_PIN_IO|GPIO_PIN_I2C_SCL              },   // Pin 19
  { PB_9,   GPIO_PIN_IO|GPIO_PIN_I2C_SDA              }    // Pin 20
};

#define NB_PIN_DESCRIPTIONS (sizeof(pinsDescription) / sizeof(PinDescription))

static int _readResolution = 10;
static int _writeResolution = 8;

static inline int32_t get_pin_description(uint32_t ulPin)
{
  int32_t i;

  //find the pin.
  for(i = 0; i < (int32_t)NB_PIN_DESCRIPTIONS; i++)
  {
    if(pinsDescription[i].pin == ulPin)
    {
      return i;
    }
  }
  return -1;
}

void analogReadResolution(int res)
{
  _readResolution = res;
}

void analogWriteResolution(int res)
{
  _writeResolution = res;
}

static inline uint32_t mapResolution(uint32_t value, uint32_t from, uint32_t to)
{
  if (from == to)
  {
    return value;
  }
  if (from > to)
  {
    return value >> (from-to);
  }
  else
  {
    return value << (to-from);
  }
}

//perform the read operation on the selected analog pin.
//the initialization of the analog PIN is done through this function
uint32_t analogRead(uint32_t ulPin)
{
  uint32_t ulValue = 0;
  uint32_t attr = 0;
  int i;

  //find the pin.
  i = get_pin_description(ulPin);
  if(i < 0)
  {
      return 0;
  }
  attr = pinsDescription[i].mode;
  if((attr & GPIO_PIN_ADC) == GPIO_PIN_ADC)
  {
    PinName pinName = PinName(ulPin);
    AnalogIn ain(pinName);
    float fValue = ain.read();
    ulValue =  fValue * 1024;
    ulValue = mapResolution(ulValue, ADC_RESOLUTION, _readResolution);
    return ulValue;
  }
  else
  {
    return 0;
  }
}

void analogOutputInit(void)
{
}

// Right now, PWM output only works on the pins with
// hardware support.  These are defined in the appropriate
// variant.cpp file.  For the rest of the pins, we default
// to digital output.
void analogWrite(uint32_t ulPin, uint32_t ulValue)
{

    uint32_t attr = 0;
    int i;

    //find the pin.
    i = get_pin_description(ulPin);
    if(i < 0)
    {
        return;
    }

    attr = pinsDescription[i].mode;

    if((attr & GPIO_PIN_PWM) == GPIO_PIN_PWM)
    {
      ulValue = mapResolution(ulValue, _writeResolution, PWM_RESOLUTION);
      PinName pinName = PinName(ulPin);
      PwmOut pout(pinName);
      float fValue = (float)ulValue / PWM_MAX_DUTY_CYCLE;
      pout.write(fValue);
    }
    else
    { //DIGITAL PIN ONLY
      // Defaults to digital write
      pinMode(ulPin, OUTPUT);
      ulValue = mapResolution(ulValue, _writeResolution, 8);
      if (ulValue < 128) {
      digitalWrite(ulPin, LOW);
    }
    else
    {
      digitalWrite(ulPin, HIGH);
    }
  }
}

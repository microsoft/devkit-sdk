/**
  * \file STSAFE_A_I2C.cpp
  * \brief STSAFE-A I2C routine.
  * \date 05/04/2016
  * \author Christophe Delaunay
  * \version 1.0
  * \copyright 2016 STMicroelectronics
  *********************************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/myliberty
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  *********************************************************************************************
  */

#include <string.h>
#include "STSAFE_A_I2C.h"

/* Define factorial fonction for waiting delay */
/* #define _FAKT */

#ifdef _FAKT
#define	I2C_POOLING					7
#else
#define	I2C_POOLING					35
#endif


/*
 * Contructor
 */
cSTSAFE_A_I2c::cSTSAFE_A_I2c(PinName sda, PinName scl, I2C *mbedI2c)
{
  if (mbedI2c)
  {
    this->_mbedI2c = mbedI2c;
  }
  else
  {
    this->_mbedI2c = new I2C(sda, scl);
  }

  //this->_mbedI2c->reset();
}

/*
 * Destructor
 */
cSTSAFE_A_I2c::~cSTSAFE_A_I2c(void)
{
  if (this->_mbedI2c)
  {
    delete this->_mbedI2c;
    this->_mbedI2c = NULL;
  }
}


/* Frequency */
/*
 * This function is executed to set-up the I2C protocol frequency.
 */
void cSTSAFE_A_I2c::Frequency(int freq_hz)
{
  this->_mbedI2c->frequency(freq_hz);
}


#ifdef _FAKT
/*
 * Source code to find factorial of a number
 */
unsigned int Factorial(unsigned int n)
{
  return (n == 1 ? n : n * Factorial(n - 1));
}
#endif /* _FAKT */


/* Transmit */
/*
 * This function is executed to send bytes stored through input/output data buffer to STSAFE-A device.
 * \note This function redefines the virtual "Transmit" function of inherited cSTSAFE_ATransport class.
 */
int cSTSAFE_A_I2c::SendBytes(void)
{
  /* Move the header */
  memmove(this->ExchangeBuffer->Data + 1, this->ExchangeBuffer->Data, this->ExchangeBuffer->Length * sizeof(unsigned char));
  memcpy(this->ExchangeBuffer->Data, &this->ExchangeBuffer->Header, sizeof(unsigned char));

  /* send to STSAFE-A */
  int	StatusCode = 0xFF;
  unsigned char loop = 1;
  while (StatusCode && loop <= I2C_POOLING)
  {
    StatusCode = this->_mbedI2c->write(this->ExchangeBuffer->I2cAdd << 1, (char *)this->ExchangeBuffer->Data, this->ExchangeBuffer->Length + 1);
    if (StatusCode)
    {
#ifdef _FAKT
      DELAY(Factorial(loop));
#else
      DELAY(2*loop);
#endif
    }
    loop++;
  }

  return (StatusCode);
}


/* Receive */
/*
 * This function is executed to receive bytes from STSAFE-A device and store its into input/output data buffer.
 * \note This function redefines the virtual "Receive" function of inherited cSTSAFE_ATransport class.
 */
int cSTSAFE_A_I2c::ReceiveBytes(void)
{
  unsigned short ResponseLength = this->ExchangeBuffer->Length;

  int	StatusCode = 0xFF;
  unsigned short loop = 1;

  while (StatusCode && loop <= I2C_POOLING)
  {
    StatusCode = this->_mbedI2c->read(this->ExchangeBuffer->I2cAdd << 1, (char *)&this->ExchangeBuffer->Header, this->ExchangeBuffer->Length + 3);
    if (StatusCode)
    {
#ifdef _FAKT
      DELAY(Factorial(loop));
#else
      DELAY(2*loop);
#endif
    }
    loop++;
  }
	
  this->ExchangeBuffer->Length = SWAP2BYTES(this->ExchangeBuffer->Length);

  if (this->ExchangeBuffer->Length && !StatusCode)
  {
    if (this->ExchangeBuffer->Length > ResponseLength)
    {
      StatusCode = 0xFF;
      loop = 1;
      while (StatusCode && (loop <= I2C_POOLING))
      {
        StatusCode = this->_mbedI2c->read(this->ExchangeBuffer->I2cAdd << 1, (char *)&this->ExchangeBuffer->Header, this->ExchangeBuffer->Length + 3);
        if (StatusCode)
        {
#ifdef _FAKT
          DELAY(Factorial(loop));
#else
          DELAY(2*loop);
#endif
        }
        loop++;
      }
      this->ExchangeBuffer->Length = SWAP2BYTES(this->ExchangeBuffer->Length);
    }
  }

  return (StatusCode);
}

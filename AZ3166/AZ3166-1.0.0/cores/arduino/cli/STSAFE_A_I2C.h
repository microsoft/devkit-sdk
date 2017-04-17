/**
  * \file STSAFE_A_I2C.h
  * \brief cSTSAFE_AI2c class definition.
  * This contains the public member function prototypes of cSTSAFE_AI2c class.
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

#ifndef STSAFE_A_I2C_H
#define STSAFE_A_I2C_H

#include "mbed.h"
#include "STSAFE_A_Transport.h"

/*!
 * \class cSTSAFE_A_I2c
 * \brief Representative class for cSTSAFE_A_I2c object
 * \details This class defines the object used to managed I2C communication through MCU
 */
class cSTSAFE_A_I2c : public cSTSAFE_A_Transport
{
public:
  /*!
   * \brief cSTSAFE_A_I2c class constructor
   * \param sda : mbed sda pin
   * \param scl : mbed scl pin
   * \param mbedI2c : mbed I2C bus handle pointer (Default value is NULL. In this case handle is created)
   */
   cSTSAFE_A_I2c(PinName sda, PinName scl, I2C* mbedI2c = NULL);

  /*!
   * \brief cSTSAFE_A_I2c class Destructor
   */
   ~cSTSAFE_A_I2c(void);

  /* Frequency */
  /*!
   * \brief Set I2C frequency.
   * \param freq_hz	: Frequency value
   */
   void Frequency(int freq_hz = 100000);

  /*!
   * \brief Transmit bytes to STSAFE-A device.\n
   * \param *DataBuffer	: Pointer to sDataBuffer structure containing I2C address and bytes to transmit
   * \return 0 if no error
   */
   virtual int SendBytes(void);

  /*!
   * \brief Receive bytes from STSAFE-A device.\n
   * \param *DataBuffer	: Pointer to sDataBuffer structure containing I2C address and bytes to receive
   * \return 0 if no error
   */
   virtual int ReceiveBytes(void);
		
private:
   /* MBED I2C object */
   I2C* _mbedI2c;
};

#endif /* STSAFE_A_I2C_H */

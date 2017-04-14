/*
 * \file STSAFE_A.cpp
 * \brief STSAFE-A routine for object definition.
 * \date 08/08/2016
 * \author Christophe Delaunay
 * \version 1.2
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

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "STSAFE_A.h"

const int8_t* EccCurveTypesBytes[NUMBER_OF_CURVES] =
{
  (const int8_t*)"\x2A\x86\x48\xCE\x3D\x03\x01\x07",     /* NIST_P_256 */
  (const int8_t*)"\x2B\x81\x04\x00\x22",                 /* NIST_P_384 */
  (const int8_t*)"\x2B\x24\x03\x03\x02\x08\x01\x01\x07", /* BRAINPOOL_P_256 */
  (const int8_t*)"\x2B\x24\x03\x03\x02\x08\x01\x01\x0B"  /* BRAINPOOL_P_384 */
};


const uint16_t EccCurveTypesLength[NUMBER_OF_CURVES] =
{
  8, /* NIST_P_256 */
  5, /* NIST_P_384 */
  9, /* BRAINPOOL_P_256 */
  9, /* BRAINPOOL_P_384 */
};

/********************************************************************************/
/* STSAFE-A Object                                                              */
/********************************************************************************/
/* Constructor */
/*
 * This constructor is executed whenever we create new objects of STSAFE-A class.
 */
cSTSAFE_A::cSTSAFE_A(cSTSAFE_A_Transport* in_transport, uint8_t in_i2c_address,
                     cSTSAFE_A_Crypto* in_crypto, HashTypes in_hash_type,
                     bool in_crc_support)
  : _i2cAddress(in_i2c_address)
  , _CRCsupported(in_crc_support)
  , _Transport(in_transport)
  , _CryptoCtx(in_crypto)
  , _HashType(in_hash_type)
{
}

/* Destructor */
/*
 * This destructor is executed whenever an STSAFE-A class object goes out of scope or
 * whenever the delete expression is applied to an  STSAFE-A class object pointer.
 */
cSTSAFE_A::~cSTSAFE_A()
{
  this->_i2cAddress = 0;
  this->_CRCsupported = 0;
  if (this->_Transport->ExchangeBuffer)
  {
    STS_DELETE(this->_Transport->ExchangeBuffer);
    this->_Transport->ExchangeBuffer = NULL;
  }

  if (this->_Transport)
  {
    this->_Transport = NULL;
  }
}

/********************************************************************************/
/* Generic STSAFE-A cmd                                                         */
/********************************************************************************/
/* Get I2C address */
/*
 * This function is executed to get the I2C address value of the current STSAFE-A class object.
 */
uint8_t cSTSAFE_A::GetI2cAddress(void)
{
  return this->_i2cAddress;
}

/* Set I2C address */
/*
 * This function is executed to set-up the I2C address of the current STSAFE-A class object.
 */
uint8_t cSTSAFE_A::SetI2cAddress(uint8_t in_i2c_address)
{
  this->_i2cAddress = in_i2c_address;
  return this->GetI2cAddress();
}

/* Get CRC support */
/*
 * This function is executed to check if communication with CRC is supported by current STSAFE-A class object.
 */
uint8_t cSTSAFE_A::GetCRCsupport(void)
{
  return this->_CRCsupported;
}

/* Set CRC support */
/*
 * This function is executed to set-up if communication with CRC is supported by current STSAFE-A class object.
 */
uint8_t cSTSAFE_A::SetCRCsupport(uint8_t in_crc_support)
{
  this->_CRCsupported = in_crc_support;
  return this->GetCRCsupport();
}

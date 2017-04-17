/*
 * \file STSAFE_A_Transport.h
 * \brief cSTSAFE_A_Transport class definition.
 * This contains the public member function prototypes of cSTSAFE_A_Transport class.
 * \date 08/08/2016
 * \author Christophe Delaunay
 * \version 1.1
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

#ifndef STSAFE_A_TRANSPORT_H
#define STSAFE_A_TRANSPORT_H

#ifdef STSAFE_A_TYPES_H_FILE
#include STSAFE_A_TYPES_H_FILE
#else
#include "STSAFE_A_Types.h"
#endif /* STSAFE_A_TYPES_H_FILE */

/*!
 * \class cSTSAFE_A_Transport
 * \brief Representative class for cSTSAFE_A_Transport object
 * \details This class define cSTSAFE_A_Transport object
 */
class cSTSAFE_A_Transport
{
public:
  /*!
   * \brief cSTSAFE_A_Transport class constructor.
   */
  cSTSAFE_A_Transport(void) : ExchangeBufferSize(0x1FB), ExchangeBuffer(NULL) {};

  /*!
   * \brief Transmit bytes to STSAFE-A device.\n
   * This function is executed to send bytes stored through input/output data buffer to STSAFE-A device.
   * \note This member function is virtual; It shall be redefined in a derived class (while preserving its calling properties).
   * \return 0 if no error
   */
  virtual int SendBytes(void) = 0;

  /*!
   * \brief Receive bytes from STSAFE-A device.\n
   * This function is executed to receive bytes from STSAFE-A device and store its into input/output data buffer.
   * \note This member function is virtual; It shall be redefined in a derived class (while preserving its calling properties).
   * \return 0 if no error
   */
  virtual int ReceiveBytes(void) = 0;

  /* Data Buffer Size */
  /*!
   * \brief Allocated data buffer size variable.\n
   * This variable contains the size of the allocated memory for input/output data exchange buffer.
   * \note This member variable is static (It's allocated only once with a scope till the program lifetime).
   */
  uint16_t ExchangeBufferSize;

  /* Data buffer */
  /*!
   * \brief Allocated data buffer pointer.\n
   * This variable contains allocated memory pointer for input/output data exchange buffer.
   * \note This member pointer is static (It's allocated only once with a scope till the program lifetime).
   */
  DataExchangeBuffer* ExchangeBuffer;
};

#endif /* STSAFE_A_TRANSPORT_H */

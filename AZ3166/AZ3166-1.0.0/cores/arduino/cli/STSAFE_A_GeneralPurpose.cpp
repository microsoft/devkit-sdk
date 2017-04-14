/*
 * \file STSAFE_A_GeneralPurpose.cpp
 * \brief STSAFE-A General Purpose routine.
 * \date 23/08/2016
 * \author Christophe Delaunay
 * \version 1.3
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

#ifndef _AUTHENT_ONLY_

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "STSAFE_A.h"


#define RND_USED_FOR_AUTHENTICATION_LENGTH   ((uint16_t)16)


/************************/
/* Echo                 */
/************************/
/*
 * This function is executed to return as response the data received in this command.
 */
ResponseCode cSTSAFE_A::Echo(uint8_t* in_echo_data, uint16_t in_echo_size,
                             EchoBuffer** out_echo, Mac in_mac)
{
  uint8_t status_code = 0x20 + ECHO;

  if (this->_Transport->ExchangeBuffer && in_echo_size && in_echo_data)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(ECHO | (in_mac & 0xE0));
    memcpy(this->_Transport->ExchangeBuffer->Data , in_echo_data, in_echo_size * sizeof(uint8_t));
    this->_Transport->ExchangeBuffer->Length = in_echo_size;

    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = in_echo_size + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 5ms */
      DELAY(5);
      status_code = this->Receive();

      if (!status_code)
      {
        *out_echo = (EchoBuffer*)&this->_Transport->ExchangeBuffer->Length;
      }
    }
  }

  return (ResponseCode)status_code;
}


/************************/
/* Reset                */
/************************/
/*
 * This function is executed to reset the volatile attributes to their initial value.
 */
ResponseCode cSTSAFE_A::Reset(void)
{
  uint8_t status_code = 0x20 + WARM_RESET;

  if (this->_Transport->ExchangeBuffer)
  {
    this->_Transport->ExchangeBuffer->Header = WARM_RESET;
    this->_Transport->ExchangeBuffer->Length = 0;

    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 0;
      /* Wait 30ms */
      DELAY(30);
      status_code = this->Receive();
    }
  }

  return (ResponseCode)status_code;
}


/************************/
/* Hibernate            */
/************************/
/*
 * This function is executed to put STSAFE-A device in hibernation.
 */
ResponseCode cSTSAFE_A::Hibernate(WakeUpModeFromHibernate in_wake_up_mode)
{
  uint8_t status_code = 0x20 + HIBERNATE;

  if (this->_Transport->ExchangeBuffer)
  {
    this->_Transport->ExchangeBuffer->Header = HIBERNATE;
    this->_Transport->ExchangeBuffer->Length = 0;
    *this->_Transport->ExchangeBuffer->Data = (in_wake_up_mode-1) ? WAKEUP_FROM_RESET : WAKEUP_FROM_I2C_START_OR_RESET;
    status_code = this->Transmit();
  }

  return (ResponseCode)status_code;
}


/************************/
/* Generate Random      */
/************************/
/*
 * This function is executed to generate a number of random bytes.
 * It's also used for generating the challenge used in the authentication entity
 * with digital signature.
 */
ResponseCode cSTSAFE_A::GenerateRandom(RndSubject in_subject, uint8_t in_length,
                                       GenerateRandomBuffer** out_generate_random,
                                       Mac in_mac)
{
  uint8_t status_code = 0x20 + GENERATE_RANDOM;

  if (this->_Transport->ExchangeBuffer && in_length)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(GENERATE_RANDOM | (in_mac & 0xE0));
    this->_Transport->ExchangeBuffer->Data[0] = in_subject ? AUTHENTICATION : EPHEMERAL;

    if (!in_subject)
    {
      this->_Transport->ExchangeBuffer->Data[1] = in_length;
    }

    this->_Transport->ExchangeBuffer->Length = in_subject ? 1 : 2;

    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = (in_subject ? RND_USED_FOR_AUTHENTICATION_LENGTH : in_length) + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 15ms */
      DELAY(15);
      status_code = this->Receive();

      if (!status_code)
      {
        *out_generate_random = (GenerateRandomBuffer*)&this->_Transport->ExchangeBuffer->Length;
      }
    }
  }

  return (ResponseCode)status_code;
}



/***************************/
/* Start Signature Session */
/***************************/
/*
 * This function is executed to start an asymmetric cryptographic session.\n
 * This command notifies the chip that a signature computation must be started.\n
 * There can be only a single asymmetric cryptographic session on going at any moment.
 */
ResponseCode cSTSAFE_A::StartSignatureSession(KeySlotNumber in_slot_number,
                                              StartSignatureSessionBuffer** out_start_signature_session)
{
  uint8_t status_code = 0x20 + START_SESSION + SIGNATURE_SESSION;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Build command */
    this->_Transport->ExchangeBuffer->Header = START_SESSION;
    this->_Transport->ExchangeBuffer->Data[0] = SIGNATURE_SESSION;
    this->_Transport->ExchangeBuffer->Data[1] = in_slot_number;
    this->_Transport->ExchangeBuffer->Length = 2;

    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 0;

      /* Setup HASH context */
      if (this->_CryptoCtx)
      {
        this->_CryptoCtx->Sha_Init(this->_HashType);
      }

      /* Wait 25ms */
      DELAY(25);
      status_code = this->Receive();

      if (!status_code)
      {
        *out_start_signature_session =
          (StartSignatureSessionBuffer*)&this->_Transport->ExchangeBuffer->Length;
      }
    }
  }

  return (ResponseCode)status_code;
}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

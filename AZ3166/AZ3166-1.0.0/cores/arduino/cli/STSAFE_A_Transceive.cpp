/*
 * \file STSAFE_A_Transceive.cpp
 * \brief STSAFE-A Transceive routine.
 * \date 23/08/2016
 * \author Christophe Delaunay
 * \version 1.4
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
#include <string.h>
#include "STSAFE_A.h"

/************************/
/* Transmit             */
/************************/
/*
 * This function is executed to transmit bytes stored through a private buffer to the current STSAFE-A class object.
 */
ResponseCode cSTSAFE_A::Transmit(void)
{
  int32_t status_code = 0;
#ifndef _AUTHENT_ONLY_
  if (this->_CryptoCtx)
  {
    /* Pre-compute RMAC */
    if (this->_Transport->ExchangeBuffer->Header & R_MAC)
    {
      this->PrecomputeRMAC();
    }

    /* Compute & concatenate CMAC to buffer */
    if (this->_Transport->ExchangeBuffer->Header & C_MAC)
    {
      status_code = this->ComputeCMAC();
    }

    if (status_code)
    {
      return (ResponseCode)status_code;
    }

    /* Compute command hash */
    if (this->_CryptoCtx->Check_Sha_Context(this->_HashType))
    {
      uint8_t hash_buffer[HASH_LENGTH];
      hash_buffer[0] = CMD_TAG;

      uint16_t Length = SWAP2BYTES(this->_Transport->ExchangeBuffer->Length + 1);
      memcpy(hash_buffer + 1, &Length, sizeof(Length));
      hash_buffer[3] = this->_Transport->ExchangeBuffer->Header;
      this->_CryptoCtx->Sha_Update(this->_HashType, hash_buffer, sizeof(hash_buffer) / sizeof(hash_buffer[0]));
      this->_CryptoCtx->Sha_Update(this->_HashType,
                                   this->_Transport->ExchangeBuffer->Data,
                                   this->_Transport->ExchangeBuffer->Length);
    }
  }
#endif

  /* Compute & concatenate CRC to buffer */
  if (this->_CRCsupported)
  {
    this->STcrc16();
  }

  this->_Transport->ExchangeBuffer->I2cAdd = this->_i2cAddress;
  status_code = this->_Transport->SendBytes();

  return (status_code ? COMMUNICATION_ERROR : OK);
}

/************************/
/* Receive              */
/************************/
/*
 * This function is executed to receive and store from current STSAFE-A class object response bytes through a private buffer.
 */
ResponseCode cSTSAFE_A::Receive(void)
{
  /* Increase buffer size in case of CRC */
  if (this->_CRCsupported)
  {
    this->_Transport->ExchangeBuffer->Length += CRC_LENGTH;
  }

  this->_Transport->ExchangeBuffer->I2cAdd = this->_i2cAddress;
  int32_t status_code = this->_Transport->ReceiveBytes();
  //(void)printf("[DEBUG] func: %s, file: %s, line: %d, para: status_code == %d!\r\n", __FUNCTION__, __FILE__, __LINE__, status_code);

  /* Check CRC */
  if (!status_code && this->_CRCsupported)
  {
    this->_Transport->ExchangeBuffer->Length -= CRC_LENGTH;
    uint16_t crc;
    memcpy(&crc,
           this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length,
           sizeof(crc));
    this->STcrc16();
    if (memcmp(&crc,
               this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length - 2,
               sizeof(crc)))

    {
      return (INVALID_CRC);
    }
    else
    {
      this->_Transport->ExchangeBuffer->Length -= CRC_LENGTH;
    }
  }

#ifndef _AUTHENT_ONLY_
  if (this->_CryptoCtx)
  {
    /* Compute RMAC */
    if (!status_code && (this->_Transport->ExchangeBuffer->Header & R_MAC))
    {
      uint8_t rmac[MAC_LENGTH];
      this->_Transport->ExchangeBuffer->Length -= MAC_LENGTH;
      memcpy(rmac,
             this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length,
             sizeof(rmac) * sizeof(rmac[0]));
      this->ComputeRMAC();
      if (memcmp(rmac,
                 this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length - 4,
                 sizeof(rmac) * sizeof(rmac[0])))
      {
        memcpy(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length - 4,
               rmac,
               sizeof(rmac) * sizeof(rmac[0]));
        return (INVALID_RMAC);
      }
      else
      {
        this->_Transport->ExchangeBuffer->Length -= MAC_LENGTH;
      }

    }

    /* Compute command hash */
    if (!status_code && this->_CryptoCtx->Check_Sha_Context(this->_HashType))
    {
      uint8_t hash_buffer[HASH_LENGTH];
      hash_buffer[0] = RSP_TAG;
      uint16_t Length = SWAP2BYTES(this->_Transport->ExchangeBuffer->Length + 1);
      memcpy(hash_buffer + 1, &Length, sizeof(Length));
      hash_buffer[3] = this->_Transport->ExchangeBuffer->Header;
      this->_CryptoCtx->Sha_Update(this->_HashType,
                                   hash_buffer,
                                   sizeof(hash_buffer) / sizeof(hash_buffer[0]));
      this->_CryptoCtx->Sha_Update(this->_HashType,
                                   this->_Transport->ExchangeBuffer->Data,
                                   this->_Transport->ExchangeBuffer->Length);
    }
  }
#endif

  return (ResponseCode)(status_code ? COMMUNICATION_ERROR : this->_Transport->ExchangeBuffer->Header & ~R_MAC);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

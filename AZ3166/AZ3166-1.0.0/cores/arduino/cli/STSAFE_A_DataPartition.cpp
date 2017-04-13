/*
 * \file STSAFE_A_DataPartition.cpp
 * \brief STSAFE-A Data Partition routine.
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
/* Read                 */
/************************/
/*
 * This function is executed to read data from a data partition zone.\n
 * It will first possibly change the read access conditions (including the read AC change right),
 * then read the data starting from the specified offset within the zone and with the requested length.\n
 * It will check the access conditions and only return data starting from the specified offset up to the zone boundary.\n
 * If command is applied to a one way counter type zone, the counter value will be returned first.
 * \note Change of the read AC change right is irreversible and that changing the
 *  read access conditions (can only result in a more strict value).
 */
ResponseCode cSTSAFE_A::Read(uint8_t in_change_ac_indicator, uint8_t in_new_read_ac_right,
                             AccessCondition in_new_read_ac, uint8_t in_zone_index,
                             uint16_t in_offset, uint16_t in_length,
                             ReadBuffer** out_read, Mac in_mac)
{
  uint8_t status_code = 0x20 + READ;
  if (this->_Transport->ExchangeBuffer)
  {
#ifndef _AUTHENT_ONLY_
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }
#endif

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(READ | (in_mac & 0xE0));

    /* Change AC indicator + New AC change right + New update AC */
    this->_Transport->ExchangeBuffer->Data[0] = in_change_ac_indicator ? 0x10 |
                                                (in_new_read_ac_right ? 0x08 : 0x00) |
                                                in_new_read_ac :
                                                0x00;

    /* Zone index */
    this->_Transport->ExchangeBuffer->Data[1] = in_zone_index;

    /* Zone offset */
    in_offset = SWAP2BYTES(in_offset);
    memcpy(this->_Transport->ExchangeBuffer->Data + 2, &in_offset, sizeof(in_offset));

    /* Length */
    in_length = SWAP2BYTES(in_length);
    memcpy(this->_Transport->ExchangeBuffer->Data + 4, &in_length, sizeof(in_length));
    this->_Transport->ExchangeBuffer->Length = 6;

    /* Transmit command */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 4 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 5ms */
      DELAY(5);
      status_code = this->Receive();

      if (!status_code)
      {
        *out_read = (ReadBuffer*)&this->_Transport->ExchangeBuffer->Length;
      }
    }
  }

  return (ResponseCode)status_code;
}

#ifndef _AUTHENT_ONLY_
/************************/
/* Decrement            */
/************************/
/*
 * This command is applied to a counter zone and performs following processing:\n
 * - It changes the update access conditions and the update AC change right.\n
 * - It decrements the one way counter with the amount given as parameter.\n
 * - It updates data in its data segment with data buffer pointed.
 * \note Change of the update AC change right is irreversible and that changing the update access conditions
 * (can only result in a more strict value).
 */
ResponseCode cSTSAFE_A::Decrement(uint8_t in_change_ac_indicator,
                                  uint8_t in_new_decrement_ac_right,
                                  AccessCondition in_new_decrement_ac,
                                  uint8_t in_zone_index, uint16_t in_offset,
                                  uint32_t in_amount, DataBuffer* in_data,
                                  DecrementBuffer** out_decrement, Mac in_mac)
{
  uint8_t	status_code = 0x20 + DECREMENT;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(DECREMENT | (in_mac & 0xE0));

    /* Change AC indicator + New AC change right + New update AC */
    this->_Transport->ExchangeBuffer->Data[0] = in_change_ac_indicator ?
             0x10 | (in_new_decrement_ac_right ? 0x08 : 0x00) | in_new_decrement_ac_right :
             0x00;

    /* Zone index */
    this->_Transport->ExchangeBuffer->Data[1] = in_zone_index;

    /* Zone offset */
    in_offset = SWAP2BYTES(in_offset);
    memcpy(this->_Transport->ExchangeBuffer->Data + 2, &in_offset, sizeof(in_offset));

    /* Amount */
    in_amount = SWAP4BYTES(in_amount);
    memcpy(this->_Transport->ExchangeBuffer->Data + 4, &in_amount, sizeof(in_amount));

    this->_Transport->ExchangeBuffer->Length = 8;

    /* Data */
    if (in_data)
    {
      memcpy(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length,
             in_data->Data,
             in_data->Length * sizeof(in_data->Data[0]));
      this->_Transport->ExchangeBuffer->Length += in_data->Length;
    }

    /* Transmit command */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = sizeof((*out_decrement)->OneWayCounter) + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 15ms */
      DELAY(15);
      /* Read response */
      status_code = this->Receive();

      if (!status_code)
      {
        *out_decrement = (DecrementBuffer*)&this->_Transport->ExchangeBuffer->Length;
        (*out_decrement)->OneWayCounter = SWAP4BYTES((*out_decrement)->OneWayCounter);
      }
    }
  }

  return (ResponseCode)status_code;
}

/************************/
/* Update               */
/************************/
ResponseCode cSTSAFE_A::Update(uint8_t in_atomicity, uint8_t in_change_ac_indicator,
                               uint8_t in_new_update_ac_right, AccessCondition in_new_update_ac,
                               uint8_t in_zone_index, uint16_t in_offset,
                               DataBuffer* in_data, Mac in_mac)
{
  uint8_t status_code = 0x20 + UPDATE;
  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(UPDATE | (in_mac & 0xE0));

    /* Atomicity mode + Change AC indicator + New AC change right + New update AC */
    this->_Transport->ExchangeBuffer->Data[0] = (in_atomicity ? 0x80 : 0x00) |
                                                (in_change_ac_indicator ?
                                                0x10 | (in_new_update_ac_right ? 0x08 : 0x00) | in_new_update_ac :
                                                0x00);

    /* Zone index */
    this->_Transport->ExchangeBuffer->Data[1] = in_zone_index;

    /* Zone offset */
    in_offset = SWAP2BYTES(in_offset);
    memcpy(this->_Transport->ExchangeBuffer->Data + 2, &in_offset, sizeof(in_offset));

    this->_Transport->ExchangeBuffer->Length = 4;

    /* Data */
    if (in_data)
    {
      memcpy(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length,
             in_data->Data,
             in_data->Length * sizeof(in_data->Data[0]));
      this->_Transport->ExchangeBuffer->Length += in_data->Length;
    }

    /* Transmit command */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 4 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 5ms */
      DELAY(5);
      status_code = this->Receive();
    }
  }

  return (ResponseCode)status_code;
}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

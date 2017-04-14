/*
 * \file STSAFE_A_Administrative.cpp
 * \brief STSAFE-A Administrative routine.
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

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "STSAFE_A.h"


/************************/
/* Product data query   */
/************************/
/*
 * This command is executed to get product informations.
 */
ResponseCode cSTSAFE_A::ProductDataQuery(ProductDataBuffer** out_product_data, Mac in_mac)
{
  uint8_t status_code = 0x20 + QUERY;

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
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(QUERY | (in_mac & 0xE0));
    this->_Transport->ExchangeBuffer->Data[0] = PRODUCT_DATA_TAG;
    this->_Transport->ExchangeBuffer->Length = 1;

    /* Transmit */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 28 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 5ms */
      DELAY(5);
      /* Receive */
      status_code = this->Receive();
      if (!status_code)
      {
        *out_product_data = (ProductDataBuffer*)&this->_Transport->ExchangeBuffer->Length;
        (*out_product_data)->InputOutputBufferSize = SWAP2BYTES((*out_product_data)->InputOutputBufferSize);
        (*out_product_data)->AtomicityBufferSize = SWAP2BYTES((*out_product_data)->AtomicityBufferSize);
        (*out_product_data)->NonVolatileMemorySize = SWAP2BYTES((*out_product_data)->NonVolatileMemorySize);
        (*out_product_data)->TestDateSize = SWAP2BYTES((*out_product_data)->TestDateSize);
        (*out_product_data)->InternalProductVersionSize = SWAP2BYTES((*out_product_data)->InternalProductVersionSize);
        (*out_product_data)->ModuleDateSize = SWAP2BYTES((*out_product_data)->ModuleDateSize);
      }
    }
  }

  return (ResponseCode)status_code;
}


#ifndef _AUTHENT_ONLY_
/************************/
/* Data partition query */
/************************/
/*
 * This command is executed to get zone informations.
 */
ResponseCode cSTSAFE_A::DataPartitionQuery(DataPartitionBuffer** out_data_partition, ZoneInformationRecordBuffer** out_zone_information_record, Mac in_mac)
{
  uint8_t status_code = 0x20 + QUERY;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(QUERY | (in_mac & 0xE0));
    this->_Transport->ExchangeBuffer->Data[0] = DATA_PARTITION_TAG;
    this->_Transport->ExchangeBuffer->Length = 1;

    /* Transmit */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = (in_mac & R_MAC) ? MAC_LENGTH : 0;
      /* Wait 5ms */
      DELAY(5);
      /* Receive */
      status_code = this->Receive();

      *out_zone_information_record = NULL;

      if (!status_code)
      {
        *out_data_partition = (DataPartitionBuffer*)&this->_Transport->ExchangeBuffer->Length;


        if (*out_data_partition && (*out_data_partition)->NumberOfZones)
        {
          *out_zone_information_record = (ZoneInformationRecordBuffer*)((uint32_t)*out_data_partition + (uint32_t)sizeof(DataPartitionBuffer));

          uint16_t num_bytes = (*out_data_partition)->Length - 1;

          (void)printf("[DEBUG] func: %s, file: %s, line: %d, para: NumberOfZones == %x!\r\n", __FUNCTION__, __FILE__, __LINE__, (*out_data_partition)->NumberOfZones);
          for (unsigned i(0); i < (*out_data_partition)->NumberOfZones; i++)
          {
            //(void)printf("[DEBUG] Index == %x!\r\n", __FUNCTION__, __FILE__, __LINE__, (*out_zone_information_record)->Index);
            num_bytes -=  sizeof(ZoneInformationRecordBuffer);

            //(void)printf("[DEBUG] Type == %x!\r\n", __FUNCTION__, __FILE__, __LINE__, (*out_zone_information_record)->ZoneType);
            if (!((*out_zone_information_record)->ZoneType & ONE_WAY_COUNTER_PRESENCE))
            {
              num_bytes +=  sizeof((*out_zone_information_record)->OneWayCounter);
              memmove(*out_zone_information_record + 1,
                      (ZoneInformationRecordBuffer*)
                      ((uint32_t)(*out_zone_information_record + 1) -
                      (uint32_t)sizeof((*out_zone_information_record)->OneWayCounter)),
                      num_bytes * sizeof(uint8_t));
              (*out_zone_information_record)->OneWayCounter = 0;
              (*out_data_partition)->Length += sizeof((*out_zone_information_record)->OneWayCounter);
            }

            (*out_zone_information_record)->DataSegmentLength = SWAP2BYTES((*out_zone_information_record)->DataSegmentLength);
            (*out_zone_information_record)->OneWayCounter = SWAP4BYTES((*out_zone_information_record)->OneWayCounter);

            (*out_zone_information_record)++;
          }

          *out_zone_information_record = (ZoneInformationRecordBuffer*)((uint32_t)*out_data_partition + (uint32_t)sizeof(DataPartitionBuffer));

          /*(void)printf("Index == %d!\r\n", __FUNCTION__, __FILE__, __LINE__, (*out_zone_information_record)[0]->Index);
          (void)printf("ZoneType == %d!\r\n", __FUNCTION__, __FILE__, __LINE__, (*out_zone_information_record)[0]->ZoneType);
          (void)printf("Index == %d!\r\n", __FUNCTION__, __FILE__, __LINE__, (*out_zone_information_record)[0]->AccessCondition);
          (void)printf("DataSegmentLength == %d!\r\n", __FUNCTION__, __FILE__, __LINE__, (*out_zone_information_record)[0]->DataSegmentLength);
          (void)printf("OneWayCounter == %d!\r\n", __FUNCTION__, __FILE__, __LINE__, (*out_zone_information_record)[0]->OneWayCounter);*/
        }
      }
    }
  }

  return (ResponseCode)status_code;
}


/************************/
/* I2C parameter query  */
/************************/
/*
 * This command is executed to get I2C informations and low power mode configuration.
 */
ResponseCode cSTSAFE_A::I2cParameterQuery(I2cParameterBuffer** out_i2c_parameter, Mac in_mac)
{
  uint8_t status_code = 0x20 + QUERY;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(QUERY | (in_mac & 0xE0));
    this->_Transport->ExchangeBuffer->Data[0] = I2C_PARAMETER_TAG;
    this->_Transport->ExchangeBuffer->Length = 1;

    /* Transmit */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = (in_mac & R_MAC) ? MAC_LENGTH : 0;
      /* Wait 5ms */
      DELAY(5);
      /* Receive */
      status_code = this->Receive();

      if (!status_code)
      {
        *out_i2c_parameter = (I2cParameterBuffer*)&this->_Transport->ExchangeBuffer->Length;
      }
    }
  }

  return (ResponseCode)status_code;
}

/**************************/
/* Life cycle state query */
/**************************/
/*
 * This command is executed to get life cycle state product status.
 */
ResponseCode cSTSAFE_A::LifeCycleStateQuery(LifeCycleStateBuffer** out_life_cycle_state, Mac in_mac)
{
  uint8_t status_code = 0x20 + QUERY;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(QUERY | (in_mac & 0xE0));
    this->_Transport->ExchangeBuffer->Data[0] = LIFE_CYCLE_STATE_TAG;
    this->_Transport->ExchangeBuffer->Length = 1;

    /* Transmit */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = (in_mac & R_MAC) ? MAC_LENGTH : 0;
      /* Wait 5ms */
      DELAY(5);
      /* Receive */
      status_code = this->Receive();

      if (!status_code)
      {
        *out_life_cycle_state = (LifeCycleStateBuffer*)&this->_Transport->ExchangeBuffer->Length;
      }
    }
  }

  return (ResponseCode)status_code;
}

#if 0
/***************************************/
/* Private Key Mode Of Operation query */
/***************************************/
/*
 * This command is executed to check if the authorization flags can be change.
 * It also executed to check if signature generation (over command & response sequence or
 * over host message digest), dynamic pairing & key establishment is authorized.
 */
ResponseCode cSTSAFE_A::PrivateKeyTableQuery(PrivateKeyTableBuffer** out_private_key_table, Mac in_mac)
{
  uint8_t status_code = 0x20 + QUERY;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_Mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(QUERY | (in_Mac & 0xE0));
    this->_Transport->ExchangeBuffer->Data[0] = PRIVATE_KEY_TABLE_TAG;
    this->_Transport->ExchangeBuffer->Length = 1;

    /* Transmit */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = (in_mac & R_MAC) ? MAC_LENGTH : 0;
      /* Wait 5ms */
      DELAY(5);
      /* Receive */
      status_code = this->Receive();

      if (!status_code)
      {
        *out_private_key_table = (PrivateKeyTableBuffer*)&this->_Transport->ExchangeBuffer->Length;
      }
    }
  }

  return (ResponseCode)status_code;
}
#endif /* 0 */

/*************************/
/* Public Key Slot query */
/*************************/
/*
 * This command is executed to get public key information.
 */
ResponseCode cSTSAFE_A::PublicKeySlotQuery(PublicKeySlotBuffer** out_pub_Key_slot,
                                           CoordinateBuffer** out_pub_x,
                                           CoordinateBuffer** out_pub_y,
                                           CurveIdBuffer** out_curve_id, Mac in_mac)
{
  uint8_t status_code = 0x20 + QUERY;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(QUERY | (in_mac & 0xE0));
    this->_Transport->ExchangeBuffer->Data[0] = PUBLIC_KEY_SLOT_TAG;
    this->_Transport->ExchangeBuffer->Length = 1;

    /* Transmit */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 1 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 5ms */
      DELAY(5);
      /* Receive */
      status_code = this->Receive();

      if (!status_code)
      {
        *out_pub_Key_slot = (PublicKeySlotBuffer*)&this->_Transport->ExchangeBuffer->Length;
        if ((*out_pub_Key_slot)->Length > 1 && (*out_pub_Key_slot)->PresenceFlag)
        {
          *out_pub_x = (CoordinateBuffer*)(&this->_Transport->ExchangeBuffer->Header +
                                           sizeof(PublicKeySlotBuffer));
          *out_pub_y = (CoordinateBuffer*)(&this->_Transport->ExchangeBuffer->Header +
                                           sizeof(PublicKeySlotBuffer) +
                                           sizeof(CoordinateBuffer) +
                                           (*out_pub_x)->Length);
          *out_curve_id = (CurveIdBuffer*)(&this->_Transport->ExchangeBuffer->Header +
                                           sizeof(PublicKeySlotBuffer) +
                                           (2 * sizeof(CoordinateBuffer)) +
                                           (*out_pub_x)->Length +
                                           (*out_pub_y)->Length);
        }
        else
        {
          *out_pub_x = NULL;
          *out_pub_y = NULL;
          *out_curve_id = NULL;
        }
      }
    }
  }

  return (ResponseCode)status_code;
}

/*************************/
/* Host Key Slot query   */
/*************************/
/*
 * This command is executed to get host key information.
 */
ResponseCode cSTSAFE_A::HostKeySlotQuery(HostKeySlotBuffer** out_host_key_slot, Mac in_mac)
{
  (void)printf("[DEBUG] func: %s, file: %s, line: %d, entered the func!\r\n", __FUNCTION__, __FILE__, __LINE__);
  uint8_t status_code = 0x20 + QUERY;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(QUERY | (in_mac & 0xE0));
    this->_Transport->ExchangeBuffer->Data[0] = HOST_KEY_SLOT_TAG;
    this->_Transport->ExchangeBuffer->Length = 1;

    /* Transmit */
    status_code = this->Transmit();
    //(void)printf("[DEBUG] func: %s, file: %s, line: %d, para: status_code == %d!\r\n", __FUNCTION__, __FILE__, __LINE__, status_code);

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 4 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 5ms */
      DELAY(5);
      /* Receive */
      status_code = this->Receive();
      (void)printf("[DEBUG] func: %s, file: %s, line: %d, para: status_code == %d!\r\n", __FUNCTION__, __FILE__, __LINE__, status_code);

      if (!status_code)
      {
        *out_host_key_slot = (HostKeySlotBuffer*)&this->_Transport->ExchangeBuffer->Length;
        (void)printf("[DEBUG] func: %s, file: %s, line: %d, HostKeyPresenceFlag = %d.\r\n", __FUNCTION__, __FILE__, __LINE__, (*out_host_key_slot)->HostKeyPresenceFlag);
        (void)printf("[DEBUG] func: %s, file: %s, line: %d, HostCMacSequenceCounter = %x.\r\n", __FUNCTION__, __FILE__, __LINE__, (*out_host_key_slot)->HostCMacSequenceCounter[0]);
        if (!(*out_host_key_slot)->HostKeyPresenceFlag)
        {
          memset((*out_host_key_slot)->HostCMacSequenceCounter, 0xFF, 3 * sizeof((*out_host_key_slot)->HostCMacSequenceCounter[0]));
        }
      }
    }
  }
	return (ResponseCode)status_code;
}


/*********************************/
/* Local Envelope Key Slot query */
/*********************************/
/*
 *  This command is executed to get host key information.
 */
ResponseCode cSTSAFE_A::LocalEnvelopeKeySlotQuery(
  LocalEnvelopeKeyTableBuffer** out_local_envelope_key_table,
  LocalEnvelopeKeyInformationRecordBuffer** out_local_envelope_key_slot0_information_record,
  LocalEnvelopeKeyInformationRecordBuffer** out_local_envelope_key_slot1_information_record,
  Mac in_mac)
{

  uint8_t status_code = 0x20 + QUERY;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(QUERY | (in_mac & 0xE0));
    this->_Transport->ExchangeBuffer->Data[0] = LOCAL_ENVELOPE_KEY_TABLE_TAG;
    this->_Transport->ExchangeBuffer->Length = 1;

    /* Transmit */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 4 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);

      /* Wait 5ms */
      DELAY(5);
      /* Receive */
      status_code = this->Receive();

      if (!status_code)
      {
				*out_local_envelope_key_table = (LocalEnvelopeKeyTableBuffer*)&this->_Transport->ExchangeBuffer->Length;
        *out_local_envelope_key_slot0_information_record = NULL;
        *out_local_envelope_key_slot1_information_record = NULL;
        if ((*out_local_envelope_key_table)->NumberOfSlots)
        {
          *out_local_envelope_key_slot0_information_record = (LocalEnvelopeKeyInformationRecordBuffer*)
                                                             ((uint32_t)(*out_local_envelope_key_table) +
                                                              (uint32_t)sizeof(LocalEnvelopeKeyTableBuffer));
          if ((*out_local_envelope_key_slot0_information_record)->PresenceFlag)
          {
            *out_local_envelope_key_slot1_information_record = (LocalEnvelopeKeyInformationRecordBuffer*)
                                                               ((uint32_t)*out_local_envelope_key_slot0_information_record +
                                                                (uint32_t)sizeof(LocalEnvelopeKeyInformationRecordBuffer));

          }
          else
          {
            *out_local_envelope_key_slot1_information_record = (LocalEnvelopeKeyInformationRecordBuffer*)
						                                                   ((uint32_t)*out_local_envelope_key_slot0_information_record +
						                                                    (uint32_t)sizeof(LocalEnvelopeKeyInformationRecordBuffer) - 1);
            *out_local_envelope_key_slot0_information_record = NULL;
          }

          if (!(*out_local_envelope_key_slot1_information_record)->PresenceFlag)
          {
            *out_local_envelope_key_slot1_information_record = NULL;
          }
        }
      }
    }
  }

  return (ResponseCode)status_code;
}


/************************/
/* Put attribute        */
/************************/
/*
 *  This command is executed to put attribute.
 */
ResponseCode cSTSAFE_A::PutAttribute(AttributeTag in_attribute_tag, uint8_t* in_data,
                                     uint16_t in_data_size, Mac in_mac)
{
  uint8_t status_code = 0x20 + PUT_ATTRIBUTE;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(PUT_ATTRIBUTE | (in_mac & 0xE0));
    /* Attribute Tag */
    this->_Transport->ExchangeBuffer->Data[0] = in_attribute_tag;
    /* Use limit */
    memcpy(this->_Transport->ExchangeBuffer->Data + 1, in_data, in_data_size * sizeof(uint8_t));

    this->_Transport->ExchangeBuffer->Length = 1 + in_data_size;

    /* Transmit */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 3 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 15ms */
      DELAY(15);
      /* Receive */
      status_code = this->Receive();
    }
  }

  return (ResponseCode)status_code;
}


/************************/
/* Generate key pair    */
/************************/
/*
 * This command is executed to generate a key pair.
 */
ResponseCode  cSTSAFE_A::GenerateKeyPair(KeySlotNumber in_private_key_slot_number,
                                         uint16_t in_use_limit,
                                         uint8_t in_change_authorization_flags_right,
                                         AuthorizationFlags in_authorization_flags,
                                         CurveId in_curve_id,
                                         uint8_t** out_point_reprensentation_id,
                                         CoordinateBuffer** out_pub_x,
                                         CoordinateBuffer** out_pub_y,
                                         Mac in_mac)
{
  uint8_t status_code = 0x20 + GENERATE_KEY;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(GENERATE_KEY | (in_mac & 0xE0));
    /*Attribute Tag */
    this->_Transport->ExchangeBuffer->Data[0] = PRIVATE_KEY_SLOT_TAG;
    /*Slot number */
    this->_Transport->ExchangeBuffer->Data[1] = in_private_key_slot_number;
    /*Use limit */
    in_use_limit = SWAP2BYTES(in_use_limit);
    memcpy(this->_Transport->ExchangeBuffer->Data + 2, &in_use_limit, sizeof(in_use_limit));
    /* Mode of operation */
    in_use_limit = SWAP2BYTES((in_change_authorization_flags_right ?
                               PRIVATE_KEY_MODE_OF_OPERATION_CHANGE_RIGHT_MASK :
                               0x0000) | (in_authorization_flags & PRIVATE_KEY_MODE_OF_OPERATION_AUTHORISATION_FLAGS_MASK));
    memcpy(this->_Transport->ExchangeBuffer->Data + 4, &in_use_limit, sizeof(in_use_limit));
    /* Curve ID */
    in_use_limit = SWAP2BYTES(EccCurveTypesLength[in_curve_id]);
    memcpy(this->_Transport->ExchangeBuffer->Data + 6, &in_use_limit, sizeof(in_use_limit)); /* Curve identifier size */
    memcpy(this->_Transport->ExchangeBuffer->Data + 8,
           EccCurveTypesBytes[in_curve_id],
           EccCurveTypesLength[in_curve_id] * sizeof(*(EccCurveTypesBytes[0]))); /* Curve identifier */

    this->_Transport->ExchangeBuffer->Length = 8 + EccCurveTypesLength[in_curve_id];

    /* Transmit */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 3 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 15ms */
      DELAY(15);
      /* Receive */
      status_code = this->Receive();

      if (!status_code)
      {
        if (this->_Transport->ExchangeBuffer->Length >= 1)
          *out_point_reprensentation_id = this->_Transport->ExchangeBuffer->Data;

        if (this->_Transport->ExchangeBuffer->Length >= 3)
        {
          *out_pub_x = (CoordinateBuffer*)(this->_Transport->ExchangeBuffer->Data + 1);
          (*out_pub_x)->Length = SWAP2BYTES((*out_pub_x)->Length);
        }
        else
					*out_pub_x = NULL;

        if ( *out_pub_x && (this->_Transport->ExchangeBuffer->Length >= 4 + (*out_pub_x)->Length ))
        {
          *out_pub_y = (CoordinateBuffer*)(this->_Transport->ExchangeBuffer->Data + 3 + (*out_pub_x)->Length);
          (*out_pub_y)->Length = SWAP2BYTES((*out_pub_y)->Length);
        }
        else
          *out_pub_y = NULL;
      }
    }
  }

  return (ResponseCode)status_code;
}



/*******************************/
/* Generate local envelope key */
/*******************************/
/*
 * This command is executed to generate a key in a local envelope key slot.
 */
ResponseCode cSTSAFE_A::GenerateLocalEnveloppeKey(KeySlotNumber in_key_slot_number,
    AesTypes in_key_types,
    uint8_t* in_seed, uint16_t in_seed_size, Mac in_mac)
{
  uint8_t status_code = 0x20 + GENERATE_KEY;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(GENERATE_KEY | (in_mac & 0xE0));
    /*Attribute Tag */
    this->_Transport->ExchangeBuffer->Data[0] = LOCAL_ENVELOPE_KEY_TABLE_TAG;
    /*Slot number */
    this->_Transport->ExchangeBuffer->Data[1] = in_key_slot_number;
    /*AES key types */
    this->_Transport->ExchangeBuffer->Data[2] = in_key_types ? AES_256 : AES_128;

    /*Seed */
    if (in_seed && in_seed_size)
      memcpy(this->_Transport->ExchangeBuffer->Data + 3, in_seed, in_seed_size * sizeof(uint8_t));
    else
      in_seed_size = 0;

    this->_Transport->ExchangeBuffer->Length = 3 + in_seed_size;

    /* Transmit */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 3 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 15ms */
      DELAY(15);
      /* Receive */
      status_code = this->Receive();
    }
  }

  return (ResponseCode)status_code;
}


/************************/
/* Delete password      */
/************************/
/*
 *  This command is used to delete password from its slot.
 */
ResponseCode cSTSAFE_A::DeletePassword(void)
{
  uint8_t status_code = 0x20 + DELETE_KEY;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Build command */
    this->_Transport->ExchangeBuffer->Header = DELETE_KEY;
    this->_Transport->ExchangeBuffer->Data[0] = PASSWORD_SLOT_TAG;
    this->_Transport->ExchangeBuffer->Length = 1;

    /* Transmit */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 3;
      /* Wait 15ms */
      DELAY(15);
      /* Receive */
      status_code = this->Receive();
    }
  }

  return (ResponseCode)status_code;
}


#if 0
/*********************** */
/* Delete key */
/*********************** */
/*
 * This command is used to delete key from host key table, a key slot from
 * local envelope key table, public key slot or password slot.
 */
ResponseCode cSTSAFE_A::DeleteKey(AttributeTag key_type,
                                  KeySlotNumber local_envelope_key_slot_number,
                                  Mac in_mac)
{
  uint8_t status_code = 0x20 + DELETE_KEY;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(DELETE_KEY | (mac & 0xE0));
    this->_Transport->ExchangeBuffer->Data[0] = key_type;
    this->_Transport->ExchangeBuffer->Length = 1;

    /*Slot number */
    if (key_type == LOCAL_ENVELOPE_KEY_TABLE_TAG)
    {
      this->_Transport->ExchangeBuffer->Data[1]= local_envelope_key_slot_number;
      this->_Transport->ExchangeBuffer->Length++;
    }

    /* Transmit */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 3 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 15ms */
      DELAY(15);
      /* Receive */
      status_code = this->Receive();
    }
  }

  return (ResponseCode)status_code;
}


/*********************** */
/* Unwrap issuer envelope */
/*********************** */
/*
 * This command is executed to unwrap a crypographic received envelope.\n
 * The envelope cryptogram is decrypted by device and re-encrypted with host cipher key.
 */
ResponseCode cSTSAFE_A::UnwrapIssuerEnvelope(uint8_t* envelope_crypogram,
                                             uint16_t envelope_crypogram_size,
                                             uint8_t envelope_mac[4],
                                             sEnvelopeCryptogram** STS_EncryptedEnvelopeCrypogram,
                                             Mac in_mac)
{
  uint8_t status_code = 0x20 + UNWRAP_ISSUER_ENVELOPE;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(UNWRAP_ISSUER_ENVELOPE | (mac & 0xE0));
    /*Envelope crypogram */
    memcpy(this->_Transport->ExchangeBuffer->Data, envelope_crypogram, envelope_crypogram_size * sizeof(*envelope_crypogram));
    /*Envelope MAC */
    memcpy(this->_Transport->ExchangeBuffer->Data + envelope_crypogram_size, envelope_mac, sizeof(envelope_mac) * sizeof(envelope_mac[0]));

    this->_Transport->ExchangeBuffer->Length = envelope_crypogram_size + 4;

    /* Transmit */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 3 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 200ms */
      DELAY(200);
      /* Receive */
      status_code = this->Receive();

      if (!status_code)
      {
        status_code = this->DataDecryption();

        if (!status_code)
        {
          *STS_EncryptedEnvelopeCrypogram = (EnvelopeCryptogramBuffer *)&this->_Transport->ExchangeBuffer->Length;

        }
      }
    }

    return (ResponseCode)status_code;
}
#endif

/************************/
/* Wrap local envelope  */
/************************/
/*
 * This command is executed to wrap data and typically working keys managed by the host.
 */
ResponseCode cSTSAFE_A::WrapLocalEnvelope(KeySlotNumber in_key_slot_number,
                                          uint8_t* in_data, uint16_t in_data_size,
                                          LocalEnvelopeBuffer** out_local_envelope,
                                          Mac in_mac)
{
  uint8_t status_code = 0x20 + WRAP_LOCAL_ENVELOPE;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build Wrap Local Envelope command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(WRAP_LOCAL_ENVELOPE | (in_mac & 0xE0));
    this->_Transport->ExchangeBuffer->Data[0] = in_key_slot_number;
    memcpy(this->_Transport->ExchangeBuffer->Data + 1, in_data, in_data_size * sizeof(uint8_t));
    this->_Transport->ExchangeBuffer->Length = in_data_size + 1;

    /* Encrypt data */
    status_code = this->DataEncryption();

    /* Transmit */
    if (!status_code)
    {
      status_code = this->Transmit();

      if (!status_code)
      {
        this->_Transport->ExchangeBuffer->Length = in_data_size + 8 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);

        /* Wait 200 ms */
        DELAY(200);
        /* Receive */
        status_code = this->Receive();

        if (!status_code)
        {
          *out_local_envelope = (LocalEnvelopeBuffer*)&this->_Transport->ExchangeBuffer->Length;
        }
      }
    }
  }

  return (ResponseCode)status_code;
}

/*************************/
/* unwrap local envelope */
/*************************/
/*
 * This command is executed to unwrap a local envelope with a local envelope key.\n
 * The result is a working key which can be further used by the host.
 */
ResponseCode cSTSAFE_A::UnwrapLocalEnvelope(KeySlotNumber in_key_slot_number,
                                            uint8_t* in_local_envelope,
                                            uint16_t in_local_envelope_size,
                                            DataBuffer** out_data, Mac in_mac)
{
  uint8_t	status_code = 0x20 + UNWRAP_LOCAL_ENVELOPE;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build Unwrap Local Envelope command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(UNWRAP_LOCAL_ENVELOPE | (in_mac & 0xE0));
    this->_Transport->ExchangeBuffer->Data[0] = in_key_slot_number;
    memcpy(this->_Transport->ExchangeBuffer->Data + 1, in_local_envelope, in_local_envelope_size * sizeof(uint8_t));
    this->_Transport->ExchangeBuffer->Length = in_local_envelope_size + 1;

    /* Transmit */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = in_local_envelope_size + 8 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 400 ms */
      DELAY(400);

      /* Receive */
      status_code = this->Receive();

      if (!status_code)
      {
        status_code = this->DataDecryption();

        if (!status_code)
        {
          *out_data = (DataBuffer*)&this->_Transport->ExchangeBuffer->Length;
        }
      }
    }
  }

  return (ResponseCode)status_code;
}

/************************/
/* Verify password      */
/************************/
/*
 *  This command is executed to perform a password verification.
 */
ResponseCode cSTSAFE_A::VerifyPassword(uint8_t* in_password,
                                       VerifyPasswordBuffer** out_verify_password,
                                       Mac in_mac)
{
  uint8_t status_code = 0x20 + VERIFY_PASSWORD;
  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(VERIFY_PASSWORD | (in_mac & 0xE0));
    /* Password */
    memcpy(this->_Transport->ExchangeBuffer->Data, in_password, PASSWORD_SIZE * sizeof(uint8_t));
    this->_Transport->ExchangeBuffer->Length = PASSWORD_SIZE;

    /* Transmit */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 3 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 15ms */
      DELAY(15);
      /* Receive */
      status_code = this->Receive();

      if (!status_code)
      {
        *out_verify_password = (VerifyPasswordBuffer*)&this->_Transport->ExchangeBuffer->Length;
      }
    }
  }

  return (ResponseCode)status_code;
}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

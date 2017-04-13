/*
 * \file STSAFE_A_PrivatePublicKey.cpp
 * \brief STSAFE-A private & public keys routine.
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
/* Generate signature   */
/************************/
/*
 * This command is executed to get an ECDSA signature over a message digest.
 */
ResponseCode cSTSAFE_A::GenerateSignature(KeySlotNumber in_private_key_slot_number,
                                          uint8_t* in_digest, HashTypes in_digest_type,
                                          SignatureBuffer** out_signature, Mac in_mac)
{
  uint8_t	status_code = 0x20 + GENERATE_SIGNATURE;

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
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(GENERATE_SIGNATURE | (in_mac & 0xE0));

    /* Private key slot number */
    this->_Transport->ExchangeBuffer->Data[0] = in_private_key_slot_number;

    /* Digest size */
    this->_Transport->ExchangeBuffer->Data[1] = ((in_digest_type ? 48 : 32) & 0xFF00) >> 8;
    this->_Transport->ExchangeBuffer->Data[2] = ((in_digest_type ? 48 : 32) & 0x00FF);

    /* Digest */
    memcpy(this->_Transport->ExchangeBuffer->Data + 3, in_digest, (in_digest_type ? 48 : 32) * sizeof(uint8_t));

    /* Length */
    this->_Transport->ExchangeBuffer->Length = 3 + (in_digest_type ? 48 : 32);

    /* Transmit command */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 4 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 150ms */
      DELAY(150);
      /* Read response */
      status_code = this->Receive();

      if (!status_code)
      {
        /* Signature */
        *out_signature = (SignatureBuffer*)&this->_Transport->ExchangeBuffer->Length;
      }
    }
  }

  return (ResponseCode)status_code;
}

#ifndef _AUTHENT_ONLY_
/************************/
/* Get signature        */
/************************/
/*
 * This command is executed to get on the commands & responses sequence since the start of a signature session.
 */
ResponseCode cSTSAFE_A::GetSignature(uint8_t* in_challenge, uint16_t in_challenge_size,
                                     SignatureBuffer** out_signature,
                                     Mac in_mac)
{
  uint8_t	status_code = 0x20 + GET_SIGNATURE;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(GET_SIGNATURE | (in_mac & 0xE0));

    /* Challenge */
    if (in_challenge && in_challenge_size)
    {
      memcpy(&this->_Transport->ExchangeBuffer->Data[0], in_challenge, in_challenge_size * sizeof(uint8_t));
      this->_Transport->ExchangeBuffer->Length = in_challenge_size;
    }
    else
      this->_Transport->ExchangeBuffer->Length = 0;

    /* Transmit command */
    status_code = this->Transmit();

    /* Compute HASH */
    if (this->_CryptoCtx && this->_CryptoCtx->Check_Sha_Context(this->_HashType))
    {
      this->_CryptoCtx->Sha_Final(this->_HashType, this->_hash);
    }

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 4 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 450ms */
      DELAY(450);
      /* Read response */
      status_code = this->Receive();

      if (!status_code)
      {
        /* Signature */
        *out_signature = (SignatureBuffer*)&this->_Transport->ExchangeBuffer->Length;
      }
    }
  }

  return (ResponseCode)status_code;
}

/****************************/
/* Verify Message Signature */
/****************************/
/*
 * This command can be executed for message authentication.
 */
ResponseCode cSTSAFE_A::VerifyMessageSignature(CurveId in_curve_id,
                                               CoordinateBuffer* in_pub_x,
                                               CoordinateBuffer* in_pub_y,
                                               SignatureBuffer* in_r_signature,
                                               SignatureBuffer* in_s_signature,
                                               DataBuffer* in_digest,
                                               VerifySignatureBuffer** out_verify_signature,
                                               Mac in_mac)
{
  uint8_t	status_code = 0x20 + VERIFY_SIGNATURE;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(VERIFY_SIGNATURE | (in_mac & 0xE0));

    /* Verify signature [message authentication] */
    this->_Transport->ExchangeBuffer->Data[0] = 0x00;

    /* Curve ID */
    this->_Transport->ExchangeBuffer->Data[1] = (EccCurveTypesLength[in_curve_id] & 0xFF00) >> 8;
    this->_Transport->ExchangeBuffer->Data[2] = (EccCurveTypesLength[in_curve_id] & 0x00FF);
    memcpy(this->_Transport->ExchangeBuffer->Data + 3,
           EccCurveTypesBytes[in_curve_id],
           EccCurveTypesLength[in_curve_id] * sizeof(this->_Transport->ExchangeBuffer->Data[0]));		//Curve identifier
    this->_Transport->ExchangeBuffer->Length = 3 + EccCurveTypesLength[in_curve_id];

    /* Public key */
    this->_Transport->ExchangeBuffer->Data[this->_Transport->ExchangeBuffer->Length] = POINT_REPRESENTATION_ID;
    this->_Transport->ExchangeBuffer->Length++;

    if (in_pub_x)
    {
      this->_Transport->ExchangeBuffer->Data[this->_Transport->ExchangeBuffer->Length] = (in_pub_x->Length & 0xFF00) >> 8;
      this->_Transport->ExchangeBuffer->Data[this->_Transport->ExchangeBuffer->Length + 1] = (in_pub_x->Length & 0x00FF);
      memcpy(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length + 2, &in_pub_x->Data, in_pub_x->Length * sizeof(in_pub_x->Data[0]));
      this->_Transport->ExchangeBuffer->Length += 2 + in_pub_x->Length;
    }

    if (in_pub_y)
    {
      this->_Transport->ExchangeBuffer->Data[this->_Transport->ExchangeBuffer->Length] = (in_pub_y->Length & 0xFF00) >> 8;
      this->_Transport->ExchangeBuffer->Data[this->_Transport->ExchangeBuffer->Length + 1] = (in_pub_y->Length & 0x00FF);
      memcpy(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length + 2, &in_pub_y->Data, in_pub_y->Length * sizeof(in_pub_y->Data[0]));
      this->_Transport->ExchangeBuffer->Length += 2 + in_pub_y->Length;
    }

    /* Signature */
    if (in_r_signature)
    {
      this->_Transport->ExchangeBuffer->Data[this->_Transport->ExchangeBuffer->Length] = (in_r_signature->Length & 0xFF00) >> 8;
      this->_Transport->ExchangeBuffer->Data[this->_Transport->ExchangeBuffer->Length + 1] = (in_r_signature->Length & 0x00FF);
      memcpy(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length + 2, &in_r_signature->Data, in_r_signature->Length * sizeof(in_r_signature->Data[0]));
      this->_Transport->ExchangeBuffer->Length += 2 + in_r_signature->Length;
    }

    if (in_s_signature)
    {
      this->_Transport->ExchangeBuffer->Data[this->_Transport->ExchangeBuffer->Length] = (in_s_signature->Length & 0xFF00) >> 8;
      this->_Transport->ExchangeBuffer->Data[this->_Transport->ExchangeBuffer->Length + 1] = (in_s_signature->Length & 0x00FF);
      memcpy(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length + 2, &in_s_signature->Data, in_s_signature->Length * sizeof(in_s_signature->Data[0]));
      this->_Transport->ExchangeBuffer->Length += 2 + in_s_signature->Length;
    }

    /* Hash */
    if (in_digest)
    {
      /* Digest size */
      this->_Transport->ExchangeBuffer->Data[this->_Transport->ExchangeBuffer->Length] = (in_digest->Length & 0xFF00) >> 8;
      this->_Transport->ExchangeBuffer->Data[this->_Transport->ExchangeBuffer->Length + 1] = (in_digest->Length & 0x00FF);
      /* Digest */
      memcpy(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length + 2, in_digest->Data, in_digest->Length * sizeof(in_digest->Data[0]));
      this->_Transport->ExchangeBuffer->Length += 2 + in_digest->Length;
    }

    /* Transmit command */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 1 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 145ms */
      DELAY(145);
      /* Read response */
      status_code = this->Receive();

      if (!status_code)
      {
        *out_verify_signature = (VerifySignatureBuffer*)&this->_Transport->ExchangeBuffer->Length;
      }
    }
  }

  return (ResponseCode)status_code;
}

/***************************/
/* Verify Entity Signature */
/***************************/
/*
 * This command can be executed for entity authentication.
 */
ResponseCode cSTSAFE_A::VerifyEntitySignature(SignatureBuffer* in_r_signature,
                                              SignatureBuffer* in_s_signature,
                                              VerifySignatureBuffer** out_verify_signature,
                                              Mac in_mac)
{
  uint8_t status_code = 0x20 + VERIFY_SIGNATURE;

  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(VERIFY_SIGNATURE | (in_mac & 0xE0));

    /* Verify signature [entity authentication] */
    this->_Transport->ExchangeBuffer->Data[0] = 0x01;

    /* Signature */
    if (in_r_signature)
    {
      this->_Transport->ExchangeBuffer->Data[this->_Transport->ExchangeBuffer->Length] = (in_r_signature->Length & 0xFF00) >> 8;
      this->_Transport->ExchangeBuffer->Data[this->_Transport->ExchangeBuffer->Length + 1] = (in_r_signature->Length & 0x00FF);
      memcpy(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length + 2, &in_r_signature->Data, in_r_signature->Length * sizeof(in_r_signature->Data[0]));
      this->_Transport->ExchangeBuffer->Length += 2 + in_r_signature->Length;
    }
    if (in_s_signature)
    {
      this->_Transport->ExchangeBuffer->Data[this->_Transport->ExchangeBuffer->Length] = (in_s_signature->Length & 0xFF00) >> 8;
      this->_Transport->ExchangeBuffer->Data[this->_Transport->ExchangeBuffer->Length + 1] = (in_s_signature->Length & 0x00FF);
      memcpy(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length + 2, &in_s_signature->Data, in_s_signature->Length * sizeof(in_s_signature->Data[0]));
      this->_Transport->ExchangeBuffer->Length += 2 + in_s_signature->Length;
    }

    /* Transmit command */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 1 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 15ms */
      DELAY(15);
      /* Read response */
      status_code = this->Receive();

      if (!status_code)
      {
        *out_verify_signature = (VerifySignatureBuffer*)&this->_Transport->ExchangeBuffer->Length;
      }
    }
  }

  return (ResponseCode)status_code;
}

/************************/
/* Establish Key        */
/************************/
/*
 * This command can be executed to establish shared secret between 2 hosts using asymmetric cryptography.
 */
ResponseCode cSTSAFE_A::EstablishKey(KeySlotNumber in_priv_key_slot_number,
                                     CoordinateBuffer* in_pub_x,
                                     CoordinateBuffer* in_pub_y,
                                     SharedSecretBuffer** out_shared_secret,
                                     Mac in_mac)
{
  uint8_t status_code = 0x20 + ESTABLISH_KEY;
  if (this->_Transport->ExchangeBuffer)
  {
    /* Get CMAC counter value */
    if (in_mac & H_MAC)
    {
      this->GetHostCMacSequenceCounter();
    }

    /* Build command */
    this->_Transport->ExchangeBuffer->Header = (uint8_t)(ESTABLISH_KEY | (in_mac & 0xE0));

    /* Private key slot number */
    this->_Transport->ExchangeBuffer->Data[0] = in_priv_key_slot_number;
    /* Point representation identifier */
    this->_Transport->ExchangeBuffer->Data[1] = POINT_REPRESENTATION_ID;
    this->_Transport->ExchangeBuffer->Length = 2;

    /* X public key */
    if (in_pub_x)
    {
      uint16_t Lx = in_pub_x->Length;
      in_pub_x->Length = SWAP2BYTES(Lx);
      memcpy(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length, in_pub_x, (Lx + 2) * sizeof(in_pub_x->Data[0]));
      this->_Transport->ExchangeBuffer->Length += 2 + Lx;
    }

    /* Y public key */
    if (in_pub_y)
    {
      uint16_t Ly = in_pub_y->Length;
      in_pub_y->Length = SWAP2BYTES(Ly);
      memcpy(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length, in_pub_y, (Ly + 2) * sizeof(in_pub_y->Data[0]));
      this->_Transport->ExchangeBuffer->Length += 2 + Ly;
    }

    /* Transmit command */
    status_code = this->Transmit();

    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Length = 3 + ((in_mac & R_MAC) ? MAC_LENGTH : 0);
      /* Wait 200ms */
      DELAY(200);
      /* Read response */
      status_code = this->Receive();

      if (!status_code)
      {
        status_code = this->DataDecryption();

        if (!status_code)
        {
          *out_shared_secret = (SharedSecretBuffer*)&this->_Transport->ExchangeBuffer->Length;
          uint16_t Ls =(*out_shared_secret)->SharedSecret.Length;
          (*out_shared_secret)->SharedSecret.Length = SWAP2BYTES(Ls);
        }
      }
    }
  }

  return (ResponseCode)status_code;
}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

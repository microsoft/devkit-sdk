/*
 * \file STSAFE_A_Types.h
 * \brief define STSAFE-A API types
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

#ifndef STSAFE_A_TYPES_H
#define STSAFE_A_TYPES_H

/********************************************************************************/
/* Defines                                                                      */
/********************************************************************************/
#ifdef _MSC_VER               /* Windows */
#include <stdint.h>
#include <Windows.h>

#elif __MBED__                /* Mbed */
#include "mbed.h"

#elif defined USE_HAL_DRIVER  /* STM32Cube */
#if defined (STM32F0)
#include "stm32f0xx.h"
#elif defined (STM32L0)
#include "stm32l0xx.h"
#elif defined (STM32F1)
#include "stm32f1xx.h"
#elif defined (STM32L1)
#include "stm32l1xx.h"
#elif defined (STM32F2)
#include "stm32f2xx.h"
#elif defined (STM32F3)
#include "stm32f3xx.h"
#elif defined (STM32F4)
#include "stm32f4xx.h"
#elif defined (STM32L4)
#include "stm32l4xx.h"
#elif defined (STM32F7)
#include "stm32f7xx.h"
#else
#error "STM32 not family not defined (STM32F0 or STM32L0 or ...)"
#endif

#endif /* USE_HAL_DRIVER */

/* Allocation */
#define STS_NEW(type, size)         new type[size]
#define STS_DELETE(arg)             if (arg) delete [] arg


/* Delay */
#ifdef _MSC_VER
#define DELAY(x)                    Sleep(x)

#elif __MBED__
#define DELAY(x)                    wait_ms(x)

#elif defined USE_HAL_DRIVER
#define DELAY(x)                    HAL_Delay(x)
#endif /* DELAY(x) */

/* CRC length */
#define	CRC_LENGTH	                ((uint8_t)2)

/* MAC length */
#define	MAC_LENGTH	                ((uint8_t)4)

/* HASH length */
#define	HASH_LENGTH	                ((uint8_t)4)

/* Hash length */
#define	HASH_SIZE(x)                16*(x+2)

/* Constants */
#define MASK_ID                     ((uint8_t)3)
#define ST_NUMBER                   ((uint8_t)9)

/*Host mac key length */
#define HOST_KEY_LENGTH             ((uint8_t)128)

/*
 * \def SWAP2BYTES(int16_t)
 * \brief Swap the 2 bytes of an int16_t or uint16_t.
 */
#define SWAP2BYTES(x)               (((x) & 0x00FF) << 8) | (((x) & 0xFF00) >> 8)

/*
 * \def SWAP4BYTES(int32_t)
 * \brief Swap the 4 bytes of a int32_t or uint32_t.
 */
#define SWAP4BYTES(x)               (((x)>>24) & 0xFF) | (((x)<<8) & 0xFF0000) | (((x)>>8) & 0xFF00) | (((x)<<24) & 0xFF000000)


/* Tag for SHA256 processing */
#define CMD_TAG ((uint8_t)0x01)
#define RSP_TAG ((uint8_t)0x02)

#define PRIVATE_KEY_MODE_OF_OPERATION_CHANGE_RIGHT_MASK        ((uint16_t)0x8000)
#define PRIVATE_KEY_MODE_OF_OPERATION_AUTHORISATION_FLAGS_MASK ((uint16_t)0x000D)

/* Point representation ID */
#define POINT_REPRESENTATION_ID     ((uint8_t)4)

/* Password size */
#define PASSWORD_SIZE               ((uint16_t)16)

/* One way counter presence */
#define ONE_WAY_COUNTER_PRESENCE    ((uint8_t)1)

/********************************************************************************/
/* Enumeration                                                                  */
/********************************************************************************/
/*
 * \enum CommandCode
 * \brief STSAFE-A command codes enumeration
 */
typedef enum
{
  ECHO                   = 0x00,
  WARM_RESET             = 0x01,
  GENERATE_RANDOM        = 0x02,
  START_SESSION          = 0x03,
  DECREMENT              = 0x04,
  READ                   = 0x05,
  UPDATE                 = 0x06,
  INCREMENT              = 0x07,
  CACHE_BASE_KEY         = 0x08,
  GENERATE_CMAC_RMAC     = 0x09,
  VERIFY_RMAC            = 0x0A,
  UNWRAP_ISSUER_ENVELOPE = 0x0B,
  DELETE_KEY             = 0x0C,
  HIBERNATE              = 0x0D,
  WRAP_LOCAL_ENVELOPE    = 0x0E,
  UNWRAP_LOCAL_ENVELOPE  = 0x0F,
  PUT_ATTRIBUTE          = 0x10,
  GENERATE_KEY           = 0x11,
  PUT_KEY                = 0x12,
  CONFIGURE              = 0x13,
  QUERY                  = 0x14,
  GET_SIGNATURE          = 0x15,
  GENERATE_SIGNATURE     = 0x16,
  VERIFY_SIGNATURE       = 0x17,
  ESTABLISH_KEY          = 0x18,
  STAND_BY               = 0x19,
  VERIFY_PASSWORD        = 0x1A
#ifdef _MSC_VER
  ,MAX_CMD_CODE
#endif /* _MSC_VER */
} CommandCode;

/*
 * \enum ResponseCode
 * \brief STSAFE-A response codes enumeration
 */
typedef enum
{
  OK                           = 0x00,
  COMMUNICATION_ERROR          = 0x01,
  INCONSISTENT_COMMAND_DATA    = 0x02,
  VALUE_OUT_OF_RANGE           = 0x03,
  UNSUPPORTED_COMMAND_CODE     = 0x04,
  UNEXPECTED_ERROR             = 0x05,
  BUFFER_LENGTH_EXCEEDED       = 0x06,
  WRONG_SESSION                = 0x07,
#if 0
  CMAC_MISSING                 = 0x08,
#endif /* 0 */
  CMAC_COUNTER_LIMIT_REACHED   = 0x09,
  KEY_NOT_FOUND                = 0x0A,
  KEY_BLOCKED                  = 0x0B,
  WRONG_KEY_TYPE               = 0x0C,
  SESSION_COUNTER_OUT_OF_RANGE = 0x0D,
  DEPTH_OUT_OF_RANGE           = 0x0E,
  INVALID_LIFE_CYCLE_STATE     = 0x0F,
  ENTRY_NOT_FOUND              = 0x10,
  UNSATISFIED_ACCESS_CONDITION = 0x11,
  INCORRECT_ZONE_TYPE          = 0x12,
  COUNTER_LIMIT_EXCEEDED       = 0x13,
  BOUNDARY_EXCEEDED            = 0x14,
  INVALID_PRIVATE_KEY          = 0x15,
  INVALID_CMAC                 = 0x16,
  CHALLENGE_MISSING            = 0x17,
  COMMAND_DECRYPTION_ERROR     = 0x18,
  INVALID_PUBLIC_KEY           = 0x19,
  PASSWORD_BLOCKED             = 0x1A,
  CRYPTO_LIB_ISSUE             = 0x1C,
  INVALID_RMAC                 = 0x1D,
  INVALID_CRC                  = 0x1E
#ifdef _MSC_VER
  ,MAX_RSP_CODE
#endif /* _MSC_VER */
} ResponseCode;

/*
 * \enum AttributeTag
 * \brief Attribute tags enumeration
 */
typedef enum
{
  ADMIN_BASE_KEY_SLOT_TAG         = 0x01,
  PERIPHERAL_BASE_KEY_SLOT_TAG    = 0x02,
  SERVICE_BASE_KEY_SLOT_TAG       = 0x03,
  MASTER_KEY_TABLE_TAG            = 0x04,
  CACHE_BASE_KEY_TABLE_TAG        = 0x05,
  LOCAL_ENVELOPE_KEY_TABLE_TAG    = 0x07,
  LIFE_CYCLE_STATE_TAG            = 0x08,
  PASSWORD_SLOT_TAG               = 0x09,
  I2C_PARAMETER_TAG               = 0x0A,
  PUBLIC_KEY_SLOT_TAG             = 0x0B,
  REMAINING_CACHE_DERIVATIONS_TAG = 0x0C,
  PRODUCT_DATA_TAG                = 0x11,
  DATA_PARTITION_TAG              = 0x12,
  PRIVATE_KEY_SLOT_TAG            = 0x13,
  PRIVATE_KEY_TABLE_TAG           = 0x14,
  HOST_KEY_SLOT_TAG               = 0x17
} AttributeTag;

/*
 * \enum WakeUpModeFromHibernate
 * \brief Wake up from hibernate modes enumeration
 */
typedef enum
{
  WAKEUP_FROM_I2C_START_OR_RESET = 0x01,
  WAKEUP_FROM_RESET              = 0x02
} WakeUpModeFromHibernate;

/*
 * \enum LifeCycleState
 * \brief Life Cycle State enumeration
 */
typedef enum
{
  BORN                    = 0x01,
  OPERATIONNAL            = 0x03,
  TERMINATED              = 0x04,
  BORN_AND_LOCKED         = 0x81,
  OPERATIONNAL_AND_LOCKED = 0x83
} LifeCycleState;

/*
 * \enum AccessCondition
 * \brief Access conditions enumeration
 */
typedef enum
{
  ALWAYS = 0x00,
  HOST   = 0x01,
  AUTH   = 0x02,
  MAC    = 0x03,
  NEVER
} AccessCondition;

/*
 * \enum StartSession
 * \brief Start session types enumeration
 */
typedef enum
{
  PERIPHERAL_SESSION      = 0x01,
  ADMIN_SESSION           = 0x02,
  UNWRAP_ENVELOPE_SESSION = 0x03,
  COMPANION_SESSION       = 0x06,
  SIGNATURE_SESSION       = 0x08
} StartSession;

/*
 * \enum Mac
 * \brief MAC types enumeration
 */
typedef enum
{
  NO_MAC                 = 0x00,
  H_MAC                  = 0x20,
  R_MAC                  = 0x40,
  C_MAC                  = 0x80,
  ADMIN_PERIPHERAL_C_MAC = C_MAC,
  ADMIN_PERIPHERAL_R_MAC = (C_MAC | R_MAC),
  HOST_C_MAC             = (C_MAC | H_MAC),
  HOST_R_MAC             = (C_MAC | R_MAC | H_MAC)
} Mac;

/* Generate random subject */
/*
 * \enum RndSubject
 * \brief Random subject enumeration
 * \details Set if the random number must or mustn't be used in the entity authentication protocol
 */
typedef enum
{
  EPHEMERAL      = 0x00,
  AUTHENTICATION = 0x01
} RndSubject;

/* Slot number */
/*
 * \enum KeySlotNumber
 * \brief Key slot number enumeration
 * \details Specifies the key slot number through specific key table
 */
typedef enum
{
  SLOT_0  = 0x00,
  SLOT_1  = 0x01
} KeySlotNumber;

/* Authorization flags */
/*
 * \enum AuthorizationFlags
 * \brief Key slot number enumeration
 * \details Specifies key usage authorized operation.
 */
typedef enum
{
  COMMAND_RESPONSE_SIGNATURE = 0x08,
  MESSAGE_DIGEST_SIGNATURE   = 0x04,
  KEY_ESTABLISHMENT          = 0x01
} AuthorizationFlags;

/* Curve identifier */
/*
 * \enum CurveId
 * \brief Curve identifier enumeration
 * \details Specifies the elliptic curve type.
 */
typedef enum
{
  NIST_P_256      = 0,
  NIST_P_384      = 1,
  BRAINPOOL_P_256 = 2,
  BRAINPOOL_P_384 = 3
} CurveId;

/* Number of supported curves */
#define NUMBER_OF_CURVES ((uint8_t)(BRAINPOOL_P_384 + 1))

/* Curve identifier string array */
extern const int8_t* EccCurveTypesBytes[NUMBER_OF_CURVES];
extern const uint16_t EccCurveTypesLength[NUMBER_OF_CURVES];

/* AES key type */
/*
 * \enum AesTypes
 * \brief AES key types enumeration
 * \details Specifies the type of the AES key.
 */
typedef enum
{
  AES_128 = 0,
  AES_256 = 1
} AesTypes;

/* HASH type */
/*
 * \enum HashTypes
 * \brief Hash types enumeration
 * \details Specifies the type of the hash.
 */
typedef enum
{
  SHA_256 = 0,
  SHA_384 = 1
} HashTypes;

/* Initial value type */
/*
 * \enum InitialValue
 * \brief Initial value types enumeration
 * \details Specifies the type of the initial value.
 */
typedef enum
{
  CMAC_COMPUTATION = 0,
  RMAC_COMPUTATION = 1,
  C_ENCRYPTION     = 2,
  R_ENCRYPTION     = 3
} InitialValue;

/********************************************************************************/
/* Structures                                                                   */
/********************************************************************************/
/*
 * \struct DataExchangeBuffer
 * \brief Data buffer structure
 * \details This structure contains data sent to STSAFE-A & data response received from STSAFE-A.
 */
#pragma pack(push, 1)
#ifdef _MSC_VER
#pragma warning(disable : 4200)
#endif
typedef struct
{
  uint8_t  I2cAdd;
  uint8_t  Header;
  uint16_t Length;
  uint8_t  Data[1];
} DataExchangeBuffer;
#pragma pack(pop)


/*
 * \struct TLV
 * \brief TLV structure
 * \details This structure formats the response to Tag Length Value format.
 */
#pragma pack(push, 1)
#ifdef _MSC_VER
#pragma warning(disable : 4200)
#endif
typedef struct
{
  uint8_t  Header;
  uint16_t Length;
  uint8_t  Data[1];
} TLV;
#pragma pack(pop)


/*
 * \struct LV
 * \brief LV structure
 * \details This structure formats the response to Length Value format.
 */
#pragma pack(push, 1)
#ifdef _MSC_VER
#pragma warning(disable : 4200)
#endif
typedef struct
{
  uint16_t Length;
  uint8_t  Data[1];
} LV;
#pragma pack(pop)


/*
 * \struct EchoBuffer
 * \brief Echo buffer structure
 * \details This structure formats the response to an echo command.
 */
#define EchoBuffer LV


/*
* \struct GenerateRandomBuffer
* \brief Generate random structure
* \details This structure formats the response to a generate random command.
*/
#define GenerateRandomBuffer LV


/*
 * \struct ReadBuffer
 * \brief Read structure
 * \details This structure formats the response to a read command.
 */
#define ReadBuffer LV


/*
 * \struct StartSignatureSessionBuffer
 * \brief Start signature session structure
 * \details This structure formats the response to a start signature session command.
 */
#define StartSignatureSessionBuffer LV


/*
 * \struct VerifySignatureBuffer
 * \brief Verify signature structure
 * \details This structure formats the response to a verify signature command.
 */
#pragma pack(push, 1)
typedef struct
{
  uint16_t Length;
  uint8_t  SignatureValidity;
} VerifySignatureBuffer;
#pragma pack(pop)


/*
 * \struct EnvelopeCryptogramBuffer
 * \brief Unwrap issuer envelope structure
 * \details This structure formats the response to unwrap issuer envelope command.
 */
#define EnvelopeCryptogramBuffer LV


/*
 * \struct LocalEnvelopeBuffer
 * \brief Wrap local envelope structure
 * \details This structure formats the response to wrap local envelope command.
 */
#define LocalEnvelopeBuffer LV


/*
 * \struct DataBuffer
 * \brief Data structure
 * \details This structure formats the response to unwrap local envelope command.
 */
#define DataBuffer LV


/*
 * \struct ProductDataBuffer
 * \brief Product data structure
 * \details This structure formats the response to a product data query.
 */
#pragma pack(push, 1)
typedef struct
{
  uint16_t Length;
  uint8_t  MaskIdentificationTag;
  uint8_t  MaskIdentificationLength;
  uint8_t  MaskIdentification[MASK_ID];
  uint8_t  STNumberTag;
  uint8_t  STNumberLength;
  uint8_t  STNumber[ST_NUMBER];
  uint8_t  InputOutputBufferSizeTag;
  uint8_t  InputOutputBufferSizeLength;
  uint16_t InputOutputBufferSize;
  uint8_t  AtomicityBufferSizeTag;
  uint8_t  AtomicityBufferSizeLength;
  uint16_t AtomicityBufferSize;
  uint8_t  NonVolatileMemorySizeTag;
  uint8_t  NonVolatileMemorySizeLength;
  uint16_t NonVolatileMemorySize;
  uint8_t  TestDateTag;
  uint8_t  TestDateLength;
  uint16_t TestDateSize;
  uint8_t  InternalProductVersionTag;
  uint8_t  InternalProductVersionLength;
  uint16_t InternalProductVersionSize;
  uint8_t  ModuleDateTag;
  uint8_t  ModuleDateLength;
  uint16_t ModuleDateSize;
} ProductDataBuffer;
#pragma pack(pop)


/*
 * \struct DataPartitionBuffer
 * \brief Data partition configuration structure
 * \details This structure formats the response to a data partition configuration query.
 */
#pragma pack(push, 1)
typedef struct
{
  uint16_t Length;
  uint8_t  NumberOfZones;
} DataPartitionBuffer;
#pragma pack(pop)


/*
 * \struct ZoneInformationRecordBuffer
 * \brief Zone information record structure
 * \details This structure provides zone information record.
 */
#pragma pack(push, 1)
typedef struct
{
  uint8_t  Index;
  uint8_t  ZoneType;
  uint8_t  AccessCondition;
  uint16_t DataSegmentLength;
  uint32_t OneWayCounter;
} ZoneInformationRecordBuffer;
#pragma pack(pop)


/*
 * \struct I2cParameterBuffer
 * \brief I2C parameter structure
 * \details This structure formats the response to a I2C parameter query.
 */
#pragma pack(push, 1)
typedef struct
{
  uint16_t Length;
  uint8_t  I2cAddress;
  uint8_t  LowPowerMode_And_I2cParameterLock;
} I2cParameterBuffer;
#pragma pack(pop)


/*
 * \struct LifeCycleStateBuffer
 * \brief Life cycle state structure
 * \details This structure formats the response to a life cycle state query.
 */
#pragma pack(push, 1)
typedef struct
{
  uint16_t       Length;
  LifeCycleState LifeCycleStatus;
} LifeCycleStateBuffer;
#pragma pack(pop)


/*
 * \struct DecrementBuffer
 * \brief Decrement structure
 * \details This structure formats the response to a decrement command.
 */
#pragma pack(push, 1)
typedef struct
{
  uint16_t Length;
  uint32_t OneWayCounter;
} DecrementBuffer;
#pragma pack(pop)


/*
 * \struct CoordinateBuffer
 * \brief Coordinate coding structure
 * \details This structure formats the coordinate.
 */
#define CoordinateBuffer LV


/*
 * \struct SharedSecretBuffer
 * \brief Shared secret structure
 * \details This structure formats the response to a Establish Key command.
 */
#pragma pack(push, 1)
typedef struct
{
  uint16_t Length;
  LV       SharedSecret;
} SharedSecretBuffer;
#pragma pack(pop)


/*
 * \struct CurveIdBuffer
 * \brief Curve identifier structure
 * \details This structure formats the curve identifier structure.
 */
#define CurveIdBuffer LV


#if 0
/*
 * \struct PrivateKeyTableBuffer
 * \brief Private key table structure
 * \details This structure formats the response to a private key table query.
 */
#pragma pack(push, 1)
typedef struct
{
  uint16_t Length;
  uint8_t  NumberOfSlots;
  uint16_t GlobalUsageLimit;
} PrivateKeyTableBuffer;
#pragma pack(pop)


/*
 * \struct PrivateKeyInformationRecordBuffer
 * \brief Private key information structure
 * \details This structure contains private key information record.
 */
#pragma pack(push, 1)
typedef struct
{
  uint16_t      Length;
  uint8_t       SlotNumber;
  uint8_t       PresenceFlag;
  uint8_t       PrivateKeyModeOfOperation[2];
  CurveIdBuffer CurveId;
} PrivateKeyInformationRecordBuffer;
#pragma pack(pop)
#endif /* 0 */


/*
 * \struct SignatureBuffer
 * \brief Signature structure
 * \details This structure contains ECDSA signature.
 */
#define SignatureBuffer LV


/*
 * \struct PublicKeySlotBuffer
 * \brief Public key coding structure
 * \details This structure formats the ECC public key slot structure.
 */
#pragma pack(push, 1)
typedef struct
{
  uint16_t Length;
  uint8_t  PresenceFlag;
  uint8_t  PointRepresentationId;
} PublicKeySlotBuffer;
#pragma pack(pop)


/*
 * \struct VerifyPasswordBuffer
 * \brief Verify password structure
 * \details This structure formats the response to a verify password command.
 */
#pragma pack(push, 1)
typedef struct
{
  uint16_t Length;
  uint8_t  VerificationStatus;
  uint8_t  RemainingTries;
} VerifyPasswordBuffer;
#pragma pack(pop)


/*
 * \struct HostKeySlotBuffer
 * \brief Host key slot structure
 * \details This structure formats the response to host key slot query.
 */
#pragma pack(push, 1)
typedef struct
{
  uint16_t Length;
  uint8_t  HostKeyPresenceFlag;
  uint8_t  HostCMacSequenceCounter[3];
} HostKeySlotBuffer;
#pragma pack(pop)


/*
 * \struct LocalEnvelopeKeyTableBuffer
 * \brief Local envelope key table structure
 * \details This structure formats the response beginning of to a local envelope key table query.
 */
#pragma pack(push, 1)
typedef struct
{
  uint16_t Length;
  uint8_t  NumberOfSlots;
} LocalEnvelopeKeyTableBuffer;
#pragma pack(pop)


/*
 * \struct LocalEnvelopeKeyInformationRecordBuffer
 * \brief Local envelope key information structure
 * \details This structure contains local envelope key information record.
 */
#pragma pack(push, 1)
typedef struct
{
  KeySlotNumber SlotNumber;
  uint8_t       PresenceFlag;
  AesTypes      KeyLength;
} LocalEnvelopeKeyInformationRecordBuffer;
#pragma pack(pop)


#endif /* STSAFE_A_TYPES_H */

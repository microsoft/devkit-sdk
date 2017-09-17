#include "DiceCore.h"
#include "DiceSha256.h"

#if defined(__GNUC__)
DICE_CMPND_ID __attribute__((section(".vdata_protected_data"))) DiceCDI = { DICE_CMPND_TAG , { 0x00 } };
#else
DICE_CMPND_ID DiceCDI = { DICE_CMPND_TAG , { 0x00 } };
#endif 

#if defined(__GNUC__)
uint8_t __attribute__((section(".vdata_protected_data"))) vDigest[DICE_DIGEST_LENGTH] = { 0x00 };
#else
uint8_t vDigest[DICE_DIGEST_LENGTH] = { 0x00 };
#endif 

#if defined(__GNUC__)
uint8_t __attribute__((section(".vdata_protected_data"))) rDigest[DICE_DIGEST_LENGTH] = { 0x00 };
#else
uint8_t rDigest[DICE_DIGEST_LENGTH] = { 0x00 };
#endif 

#if defined(__GNUC__)
DICE_SHA256_CONTEXT __attribute__((section(".vdata_protected_data"))) DiceHashCtx = { 0x00 };
#else
DICE_SHA256_CONTEXT  DiceHashCtx = { 0x00 };
#endif 

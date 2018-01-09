// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#ifndef __DICE_RIOT_H__
#define __DICE_RIOT_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
	int DiceRIoTStart(const char *RegistrationId, uint8_t* riotCore, uint32_t riotSize, uint8_t* riotFw, uint32_t riotFwSize, char * aliasCertBuffer, uint32_t aliasCertSize);
#ifdef __cplusplus
}
#endif

#endif  // __DICE_RIOT_H__

// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include "DiceRIoT.h"
#include "DiceCore.h"
#include "RiotCore.h"
#include <stdio.h>

// Protected data
extern uint8_t udsBytes[DICE_UDS_LENGTH];
DICE_UDS DiceUDS = { DICE_UDS_TAG, 0 };

// Non-protected data
DICE_DATA DiceData = { 0 };
DICE_CMPND_ID DiceCDI = { DICE_CMPND_TAG , { 0x00 } };

static int DiceInit(uint8_t* riotCore, uint32_t riotSize)
{
	for (int i = 0; i < DICE_UDS_LENGTH; i++) {
		DiceUDS.bytes[i] = udsBytes[i];
	}

    // Up-front sanity check
    if (DiceUDS.tag != DICE_UDS_TAG) {
        return -1;
    }

    // Initialize CDI structure
    memset(&DiceCDI, 0x00, sizeof(DICE_CMPND_ID));
    DiceCDI.tag = DICE_CMPND_TAG;

    // Pointers to protected DICE Data
    DiceData.UDS = &DiceUDS;
    DiceData.CDI = &DiceCDI;

    // Start of RIoT Invariant Code
    DiceData.riotCore = (uint8_t*)riotCore;

    // Calculate size of RIoT Core
    if((DiceData.riotSize = riotSize) == 0){
        return -1;
    }

    return 0;
}

int DiceRIoTStart(const char *RegistrationId, uint8_t* riotCore, uint32_t riotSize, uint8_t* riotFw, uint32_t riotFwSize, char * aliasCertBuffer, uint32_t aliasCertSize)
{
    // Initialize DICE
    if (DiceInit(riotCore, riotSize) != 0){
        return -1;
    }

    // Launch protected DICE code. This will measure RIoT Core, derive the
    // CDI value. It must execute with interrupts disabled. Therefore, it
    // must return so we can restore interrupt state.
    if (DiceCore(DiceData) != 0){
        return -1;
    }

    // If DiceCore detects an error condition, it will not enable access to
    // the volatile storage segment. This attempt to transfer control to RIoT
    // will trigger a system reset. We will not be able to proceed.
    // TODO: DETECT WHEN A RESET HAS OCCURRED AND TAKE SOME ACTION.
    if (RiotStart(DiceCDI.bytes, (uint16_t)DICE_DIGEST_LENGTH, RegistrationId, riotFw, riotFwSize) != 0){
        return -1;
    }

	// Retrieve generated alias key certificate
	unsigned int length = 0;
	const char * buffer;
	buffer = RIoTGetAliasCert(&length);
	if (aliasCertSize > length) {
		strncpy(aliasCertBuffer, buffer, length);
	}

    return 0;
}

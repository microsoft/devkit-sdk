/*(Copyright)

Microsoft Copyright 2015, 2016
Confidential Information

*/
#ifndef __DICE_CORE_H
#define __DICE_CORE_H

#include <stdint.h>
#include <string.h>

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#define DICE_DIGEST_LENGTH  0x20

// The size of our Unique Device Secret value
#define DICE_UDS_LENGTH     0x20

// Used to pad out protected area to predefined alignment
#define DICE_PRDATA_ALIGN   (0x100)
#define DICE_PRDATA_PADDING ((DICE_PRDATA_ALIGN) - (sizeof(DICE_UDS) + sizeof(DICE_CMPND_ID)))

// Pointer to first byte of protected storage for DICE.
#define DICE_DATA_START     (0x080FF000)

// Tag used to determine if the Unique Device Secret has been provisioned.
#define DICE_UDS_TAG        (0x00534455)

// Tag used to determine if the Compound ID has been provisioned.
#define DICE_CMPND_TAG      (0x44495041)

// The structure of our UDS.
typedef struct DICE_UDS {
    uint32_t    tag;
    uint8_t     bytes[DICE_UDS_LENGTH];
} DICE_UDS;

typedef struct DICE_CMPND_ID {
    uint32_t    tag;
    uint8_t     bytes[DICE_DIGEST_LENGTH];
} DICE_CMPND_ID;

typedef struct DICE_PROTECTED_DATA {
    DICE_UDS        UDS;
    DICE_CMPND_ID   CompoundID;
    uint8_t         reserved[DICE_PRDATA_PADDING];
} DICE_PROTECTED_DATA;

typedef struct DICE_DATA {
    DICE_UDS       *UDS;
    DICE_CMPND_ID  *CDI;
    uint8_t        *riotCore;
    uint32_t        riotSize;
} DICE_DATA;

void
DiceCore(
    void
);

//#define RIOT_BASE_ADDR  Load$$ER_IROM4$$Base
//#define RIOT_BASE_ADDR  (0x08070800)
//#define RIOT_LIMIT_ADDR Load$$ER_IROM4$$Limit
//#define RIOT_LIMIT_ADDR (0x08073478)

//extern uint8_t RIOT_BASE_ADDR[];
//extern uint8_t RIOT_LIMIT_ADDR[];

#endif

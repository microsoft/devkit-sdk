
/*(Copyright)

Microsoft Copyright 2015, 2016
Confidential Information

*/
#ifndef _RIOT_CORE_H
#define _RIOT_CORE_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include "Riot.h"
#include "RiotDerEnc.h"
#include "RiotX509Bldr.h"

char * RIoTGetDeviceID(unsigned int *len);
char * RIoTGetAliasKey(unsigned int *len);
char * RIoTGetAliasCert(unsigned int *len);
char * RIoTGetDeviceCert(unsigned int *len);

int RiotStart(uint8_t *CDI, uint16_t CDILen, const char *RegistrationId, uint8_t* riotFw, uint32_t riotFwSize);

#ifdef __cplusplus
}
#endif
#endif

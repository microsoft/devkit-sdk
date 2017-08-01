// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef _SECURE_STORAGE_INTERFACE_
#define _SECURE_STORAGE_INTERFACE_

#define ZONE_DATA 8
#define ENVELOPE_SIZE                       8*60
/* Exported functions */

unsigned char secureStoreData (char* buf, unsigned int size, unsigned char zone);
unsigned char secureGetData (char* buf, unsigned int size, unsigned char zone);
unsigned char secureEraseData (unsigned char zone);
unsigned char secureGenerateRandom(char* buf, unsigned int size);


#endif

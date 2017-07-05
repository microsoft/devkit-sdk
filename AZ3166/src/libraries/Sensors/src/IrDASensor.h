// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "stm32f4xx_hal.h"
#include "mico_common.h"

#ifndef __IRDA_SENSOR_H__
#define __IRDA_SENSOR_H__

#ifdef __cplusplus

class IRDASensor
{
public:
    IRDASensor();
    ~IRDASensor();
    
    int init();

    unsigned char IRDATransmit( unsigned char *pData, int size, int timeout);
};

#endif  // __cplusplus
#endif  // __IRDA_SENSOR_H__
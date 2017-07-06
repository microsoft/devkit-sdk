// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "IrDASensor.h"
#include "PinNames.h"
#include "PeripheralPins.h"


IRDA_HandleTypeDef IrdaHandle;
void USART3_IRQHandler( void )
{
    HAL_IRDA_IRQHandler( &IrdaHandle );
}


IRDASensor::IRDASensor()
{
}

IRDASensor::~IRDASensor()
{
}


int IRDASensor::init()
{
    OSStatus err = kNoErr;

    pinmap_pinout( PB_10, PinMap_UART_TX );
    pin_mode( PB_10, PushPullNoPull );

    __HAL_RCC_USART3_CLK_ENABLE( );

    __HAL_RCC_USART3_FORCE_RESET( );
    __HAL_RCC_USART3_RELEASE_RESET( );

    IrdaHandle.Instance = USART3;

    IrdaHandle.Init.BaudRate = 38400;
    IrdaHandle.Init.WordLength = UART_WORDLENGTH_8B;
    IrdaHandle.Init.Parity = UART_PARITY_NONE;
    IrdaHandle.Init.Mode = UART_MODE_TX_RX;
    IrdaHandle.Init.Prescaler = 1;
    IrdaHandle.Init.IrDAMode = IRDA_POWERMODE_NORMAL;

    /* Enable and set I2Sx Interrupt to a lower priority */
    HAL_NVIC_SetPriority( USART3_IRQn, 0x0F, 0x00 );
    NVIC_SetVector( USART3_IRQn, (uint32_t) & USART3_IRQHandler );
    HAL_NVIC_EnableIRQ( USART3_IRQn );

    err = HAL_IRDA_Init( &IrdaHandle );
    return err;
}


unsigned char IRDASensor::IRDATransmit( unsigned char *pData, int size, int timeout)
{
    return HAL_IRDA_Transmit(&IrdaHandle, (uint8_t *)pData, (uint16_t)size, (uint32_t)timeout);
}

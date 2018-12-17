/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@arduino.cc>
 * Copyright (c) 2014 by Paul Stoffregen <paul@pjrc.com> (Transaction API)
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include "mbed.h"
#include "variant.h"

// This defines are not representing the real Divider of the STM32F412
// the Defines match to an AVR Arduino on 16MHz for better compatibility
#define SPI_CLOCK_DIV2    8000000   //8 MHz
#define SPI_CLOCK_DIV4    4000000   //4 MHz
#define SPI_CLOCK_DIV8    2000000   //2 MHz
#define SPI_CLOCK_DIV16   1000000   //1 MHz
#define SPI_CLOCK_DIV32   500000    //500 KHz
#define SPI_CLOCK_DIV64   250000    //250 KHz
#define SPI_CLOCK_DIV128  125000    //125 KHz

#define SPI_SPEED_CLOCK_DEFAULT_HZ SPI_CLOCK_DIV16
#define SPI_DEFAULT_BITS_PER_FRAME 8

const uint8_t SPI_MODE0 = 0x00; ///<  CPOL: 0  CPHA: 0
const uint8_t SPI_MODE1 = 0x01; ///<  CPOL: 0  CPHA: 1
const uint8_t SPI_MODE2 = 0x10; ///<  CPOL: 1  CPHA: 0
const uint8_t SPI_MODE3 = 0x11; ///<  CPOL: 1  CPHA: 1

class SPISettings
{
public:
  SPISettings() : _clock(SPI_SPEED_CLOCK_DEFAULT_HZ), _bitOrder(MSBFIRST), _dataMode(SPI_MODE0) {}
  SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) : _clock(clock), _bitOrder(bitOrder), _dataMode(dataMode) {}
  uint32_t _clock;
  uint8_t _bitOrder;
  uint8_t _dataMode;
};

class SPIClass
{
public:
  SPIClass();

  void begin(void);
  void end(void);

  void beginTransaction(SPISettings settings);
  void endTransaction(void);

  void setBitOrder(uint8_t bitOrder);
  void setDataMode(uint8_t dataMode);
  void setFrequency(uint32_t freq);

  uint8_t transfer(uint8_t data);

private:
  void spi_setup(void);

  SPISettings spiSetting;

  MbedSPI *deviceSPI;
};

#if SPI_INTERFACES_COUNT > 0
extern SPIClass SPI;
#endif

#endif

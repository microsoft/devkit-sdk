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

#include "AZ3166SPI.h"

SPIClass SPI;

SPIClass::SPIClass()
{
  deviceSPI = NULL;
}

void SPIClass::begin()
{
  end();
  
  deviceSPI = new MbedSPI(SPI_MOSI, SPI_MISO, SPI_CLK, SPI_SS);
  spi_setup();
}

void SPIClass::end(void)
{
  if (deviceSPI)
  {
    delete deviceSPI;
    deviceSPI = NULL;
  }
}

void SPIClass::beginTransaction(SPISettings settings)
{
  if (deviceSPI == NULL)
  {
    begin();
  }
  // Un-select audio codec
  pinMode(PB_12, OUTPUT);
  digitalWrite(PB_12, HIGH);

  setFrequency(settings._clock);
  setDataMode(settings._dataMode);
}

void SPIClass::endTransaction(void)
{
}

void SPIClass::setBitOrder(uint8_t bitOrder)
{
  // Only support MSB
}

void SPIClass::setDataMode(uint8_t dataMode)
{
  if (deviceSPI == NULL)
  {
    begin();
  }
  
  if (dataMode < 4)
  {
    spiSetting._dataMode = dataMode;
    deviceSPI->format(SPI_DEFAULT_BITS_PER_FRAME, spiSetting._dataMode);
  }
}

void SPIClass::setFrequency(uint32_t freq)
{
  if (deviceSPI == NULL)
  {
    begin();
  }

  if (freq <= SPI_CLOCK_DIV2 && freq >= SPI_CLOCK_DIV128)
  {
    spiSetting._clock = freq;
    deviceSPI->frequency(spiSetting._clock);
  }
}

uint8_t SPIClass::transfer(uint8_t data)
{
  if (deviceSPI == NULL)
  {
    begin();
  }
  
  return (uint8_t)deviceSPI->write(data);
}

void SPIClass::spi_setup(void)
{
  if (deviceSPI)
  {
    spiSetting._clock = SPI_SPEED_CLOCK_DEFAULT_HZ;
    deviceSPI->frequency(spiSetting._clock);

    spiSetting._dataMode = SPI_MODE0;
    deviceSPI->format(SPI_DEFAULT_BITS_PER_FRAME, spiSetting._dataMode);
  }
}

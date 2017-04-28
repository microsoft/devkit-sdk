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

SPIClass::SPIClass() : g_active_id(-1)
{
}

//begin using the default chip select
void SPIClass::begin()
{
  begin(BOARD_SPI_OWN_SS);
}

//Begin with a chip select defined
void SPIClass::begin(uint8_t _pin)
{
  if(_pin > SPI_CHANNELS_NUM)
    return;

  if(_pin != BOARD_SPI_OWN_SS) {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, HIGH);
  }

  spi_init(&obj, SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS);
  g_active_id = _pin;
}

void SPIClass::usingInterrupt(uint8_t interruptNumber)
{
  //Not implemented
}

void SPIClass::set_CPOL_CPHA(uint8_t mode)
{
  if (mode == SPI_MODE_0 || mode == SPI_MODE_1)
  {
    obj.spi.handle.Init.CLKPolarity = SPI_POLARITY_LOW;
  }
  else
  {
    obj.spi.handle.Init.CLKPolarity = SPI_POLARITY_HIGH;
  }
  if (mode == SPI_MODE_0 || mode == SPI_MODE_2)
  {
    obj.spi.handle.Init.CLKPhase = SPI_PHASE_1EDGE;
  }
  else if (mode == SPI_MODE_2)
  {
    obj.spi.handle.Init.CLKPhase = SPI_PHASE_2EDGE;
  }
}

void SPIClass::beginTransaction(uint8_t _pin, SPISettings settings)
{
  if(_pin > SPI_CHANNELS_NUM)
    return;

  spiSettings[_pin].clk = settings.clk;
  spiSettings[_pin].dMode = settings.dMode;
  set_CPOL_CPHA(settings.dMode);
  spiSettings[_pin].bOrder = settings.bOrder;

  if (spiSettings[_pin].bOrder == MSBFIRST)
  {
    spiSettings[_pin].msb = MSBFIRST;
    obj.spi.handle.Init.FirstBit = SPI_FIRSTBIT_MSB;
  }
  else
  {
    spiSettings[_pin].msb = LSBFIRST;
    obj.spi.handle.Init.FirstBit = SPI_FIRSTBIT_LSB;
  }
  spi_init(&obj, SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS);
  g_active_id = _pin;
}

void SPIClass::endTransaction(void)
{
  g_active_id = -1;
}

void SPIClass::end(uint8_t _pin)
{
  end();
}

void SPIClass::end()
{
  spi_free(&obj);
  g_active_id = -1;
}

void SPIClass::setBitOrder(uint8_t _pin, BitOrder _bitOrder)
{
  if(_pin > SPI_CHANNELS_NUM)
    return;

  if (MSBFIRST == _bitOrder)
  {
    spiSettings[_pin].msb = MSBFIRST;
    spiSettings[_pin].bOrder = MSBFIRST;
    obj.spi.handle.Init.FirstBit = SPI_FIRSTBIT_MSB;
  }
  else
  {
    spiSettings[_pin].msb = LSBFIRST;
    spiSettings[_pin].bOrder = LSBFIRST;
    obj.spi.handle.Init.FirstBit = SPI_FIRSTBIT_LSB;
  }

  spi_init(&obj, SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS);
  g_active_id = _pin;
}

void SPIClass::setDataMode(uint8_t _pin, uint8_t _mode)
{
  if(_pin > SPI_CHANNELS_NUM)
    return;

  if(SPI_MODE0 == _mode) {
    spiSettings[_pin].dMode = SPI_MODE_0;
  } else if(SPI_MODE1 == _mode) {
    spiSettings[_pin].dMode = SPI_MODE_1;
  } else if(SPI_MODE2 == _mode) {
    spiSettings[_pin].dMode = SPI_MODE_2;
  } else if(SPI_MODE3 == _mode) {
    spiSettings[_pin].dMode = SPI_MODE_3;
  }

  set_CPOL_CPHA(_mode);
  spi_init(&obj, SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS);
  g_active_id = _pin;
}

void SPIClass::setClockDivider(uint8_t _pin, uint8_t _divider)
{
  if(_pin > SPI_CHANNELS_NUM)
    return;

  switch(_divider) {
    case (SPI_CLOCK_DIV2) :
      spiSettings[_pin].clk = SPI_SPEED_CLOCK_DIV2_MHZ;
      obj.spi.handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    break;
    case (SPI_CLOCK_DIV4) :
      spiSettings[_pin].clk = SPI_SPEED_CLOCK_DIV4_MHZ;
      obj.spi.handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    break;
    case (SPI_CLOCK_DIV8) :
      spiSettings[_pin].clk = SPI_SPEED_CLOCK_DIV8_MHZ;
      obj.spi.handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    break;
    case (SPI_CLOCK_DIV16) :
      spiSettings[_pin].clk = SPI_SPEED_CLOCK_DIV16_MHZ;
      obj.spi.handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    break;
    case (SPI_CLOCK_DIV32) :
      spiSettings[_pin].clk = SPI_SPEED_CLOCK_DIV32_MHZ;
      obj.spi.handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    break;
    case (SPI_CLOCK_DIV64) :
      spiSettings[_pin].clk = SPI_SPEED_CLOCK_DIV64_MHZ;
      obj.spi.handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    break;
    case (SPI_CLOCK_DIV128) :
      spiSettings[_pin].clk = SPI_SPEED_CLOCK_DIV128_MHZ;
      obj.spi.handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
    break;
    case (SPI_CLOCK_DIV256) :
      spiSettings[_pin].clk = SPI_SPEED_CLOCK_DIV256_MHZ;
      obj.spi.handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    break;
    default:
      spiSettings[_pin].clk = SPI_SPEED_CLOCK_DIV16_MHZ;
      obj.spi.handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    break;
  }

  spi_init(&obj, SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS);
  g_active_id = _pin;
}


//Transfer a message on the selected SPI. The _pin is the CS of the SPI that
//identifies the SPI instance.
//If the _mode is set to SPI_CONTINUE, keep the spi instance alive.
byte SPIClass::transfer(uint8_t _pin, uint8_t data, SPITransferMode _mode)
{
  uint8_t rx_buffer = 0;

  if (_pin > SPI_CHANNELS_NUM)
    return rx_buffer;

  if(_pin != g_active_id) {
    spi_init(&obj, SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS);
    g_active_id = _pin;
  }

  if(_pin != BOARD_SPI_OWN_SS)
    digitalWrite(_pin, LOW);

  spi_master_write(&obj, (int)data);

  if((_pin != BOARD_SPI_OWN_SS) && (_mode == SPI_LAST))
    digitalWrite(_pin, HIGH);

  return rx_buffer;
}

uint16_t SPIClass::transfer16(uint8_t _pin, uint16_t data, SPITransferMode _mode)
{
  uint16_t rx_buffer = 0;

  if (_pin > SPI_CHANNELS_NUM)
    return rx_buffer;

  if(_pin != g_active_id) {
    spi_init(&obj, SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS);
    g_active_id = _pin;
  }

  if(_pin != BOARD_SPI_OWN_SS)
    digitalWrite(_pin, LOW);

  spi_master_write(&obj, (int)data);
  /*   // enable interrupt?
  //const void *tx, size_t tx_length, void *rx, size_t rx_length, uint8_t bit_width, uint32_t handler, uint32_t event, DMAUsage hint)
  //(uint8_t *)&data, (uint8_t *)&rx_buffer, sizeof(uint16_t), 10000000);
  //uint8_t * tx_buffer, uint8_t * rx_buffer, uint16_t len, uint32_t Timeout)
  spi_master_tranfer(&obj, (void *)&data, tx_length, (unit16_t *)&rx_buffer, rx_length, );
  */

  if((_pin != BOARD_SPI_OWN_SS) && (_mode == SPI_LAST))
    digitalWrite(_pin, HIGH);

  return rx_buffer;
}

void SPIClass::transfer(uint8_t _pin, void *_buf, size_t _count, SPITransferMode _mode)
{
  if ((_count == 0) || (_pin > SPI_CHANNELS_NUM))
    return;

  if(_pin != g_active_id) {
    spi_init(&obj, SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS);
    g_active_id = _pin;
  }

  if(_pin != BOARD_SPI_OWN_SS)
    digitalWrite(_pin, LOW);

  spi_send(&obj, (uint8_t *)_buf, _count, 10000);

  if((_pin != BOARD_SPI_OWN_SS) && (_mode == SPI_LAST))
    digitalWrite(_pin, HIGH);
}

int SPIClass::spi_send(spi_t *obj,uint8_t *Data, uint16_t len, uint32_t Timeout)
{
  struct spi_s *spiobj = SPI_S(obj);
  SPI_HandleTypeDef *handle = &(spiobj->handle);
  int ret = 0;
  HAL_StatusTypeDef hal_status;

  hal_status = HAL_SPI_Transmit(handle, Data, len, Timeout);

  if(hal_status == HAL_TIMEOUT) {
    ret = 1;
  } else if(hal_status != HAL_OK) {
    ret = 2;
  }
  return ret;
}

void SPIClass::attachInterrupt(void) {
  // Should be enableInterrupt()
}

void SPIClass::detachInterrupt(void) {
	// Should be disableInterrupt()
}

/*
  TwoWire.h - TWI/I2C library for Arduino & Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts
*/

#ifndef TwoWire_h
#define TwoWire_h

#include <inttypes.h>
#include "i2c_api.h"

#define BUFFER_LENGTH 32

#define MASTER_ADDRESS 0x33

#define WIRE_ERROR (-1)

// WIRE_HAS_END means Wire has end()
#define WIRE_HAS_END 1

#define I2C_OK 0
#define I2C_TIMEOUT 1

class TwoWire
{
  private:
    unsigned char rxBuffer[BUFFER_LENGTH];
    unsigned char rxBufferIndex;
    unsigned char rxBufferLength;

    unsigned char txAddress;
    unsigned char txBuffer[BUFFER_LENGTH];
    unsigned char txBufferIndex;
    unsigned char txBufferLength;
    
    unsigned char transmitting;
    
    unsigned char ownAddress;
    bool master;
    I2CName p_i2c_instance; 
    
    void (*user_onRequest)(void);
    void (*user_onReceive)(int);
    i2c_t obj;
    //static void onRequestService(I2CName);
    //static void onReceiveService(I2CName, unsigned char*, int);

  public:
    TwoWire(I2CName i2c_instance);
    void begin();
    void begin(unsigned char);
    void begin(int);
    void end();
    void setClock(uint32_t);
    void beginTransmission(unsigned char);
    void beginTransmission(int);
    unsigned char endTransmission(void);
    unsigned char endTransmission(unsigned char);
    unsigned char requestFrom(unsigned char, unsigned char);
    unsigned char requestFrom(unsigned char, unsigned char, unsigned char);
	  unsigned char requestFrom(unsigned char, unsigned char, unsigned int, unsigned char, unsigned char);
    uint8_t requestFrom(uint8_t, uint8_t, uint32_t, uint8_t, uint8_t);
    unsigned char requestFrom(int, int);
    unsigned char requestFrom(int, int, int);
    virtual size_t write(unsigned char);
    virtual size_t write(const unsigned char *, size_t);
    virtual int available(void);
    virtual int read(void);
    virtual int peek(void);
    virtual void flush(void);
    void onReceive( void (*)(int) );
    void onRequest( void (*)(void) );

    inline size_t write(unsigned long n) { return write((unsigned char)n); }
    inline size_t write(long n) { return write((unsigned char)n); }
    inline size_t write(unsigned int n) { return write((unsigned char)n); }
    inline size_t write(int n) { return write((unsigned char)n); }
};

extern TwoWire Wire;
extern TwoWire Wire1;

#endif


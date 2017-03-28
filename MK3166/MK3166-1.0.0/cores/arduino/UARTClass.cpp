/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "UARTClass.h"

void UARTClass::begin(const uint32_t dwBaudRate)
{
  serial->baud(dwBaudRate);
}

void UARTClass::end(void)
{
  // TODO
}

int  UARTClass::read(void)
{
  return 0;
}

size_t  UARTClass::write(const uint8_t c)
{
  serial->putc(c);
  return 1;
}


int UARTClass::available( void )
{
  //return the amount of data available
  return 0;
}

int UARTClass::peek( void )
{
  //return the last element of the rx buffer without removing it from the buffer
  return 0;
}

void UARTClass::flush( void )
{
}
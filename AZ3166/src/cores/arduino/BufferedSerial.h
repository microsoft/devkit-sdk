
/**
 * @file    BufferedSerial.h
 * @brief   Software Buffer - Extends mbed Serial functionality adding irq driven TX and RX
 * @author  sam grove
 * @version 1.0
 * @see     
 *
 * Copyright (c) 2013
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BUFFEREDSERIAL_H
#define BUFFEREDSERIAL_H
 
#include "mbed.h"
#include "RingBuffer.h"

/** A serial port (UART) for communication with other serial devices
 *
 * Can be used for Full Duplex communication, or Simplex by specifying
 * one pin as NC (Not Connected)
 */

/**
 *  @class BufferedSerial
 *  @brief Software buffers and interrupt driven tx and rx for Serial
 */  
class BufferedSerial : public RawSerial 
{
private:
    RingBuffer _rxbuf;
    RingBuffer _txbuf;
    uint32_t _buf_size;
    uint32_t _tx_multiple;
 
    void rxIrq(void);
    void txIrq(void);
    void prime(void);
    
public:
    /** Create a BufferedSerial port, connected to the specified transmit and receive pins
     *  @param tx Transmit pin
     *  @param rx Receive pin
     *  @param buf_size printf() buffer size
     *  @param tx_multiple amount of max printf() present in the internal ring buffer at one time
     *  @param name optional name
     *  @note Either tx or rx may be specified as NC if unused
     */
    BufferedSerial(PinName tx, PinName rx, uint32_t buf_size = 128, uint32_t tx_multiple = 4, const char* name=NULL, int sample_rate = 115200);
    
    /** Destroy a BufferedSerial port
     */
    virtual ~BufferedSerial(void);
    
    /** Check on how many bytes are in the rx buffer
     *  @return 1 if something exists, 0 otherwise
     */
    virtual int readable(void);
    
    /** Check to see if the tx buffer has room
     */
    virtual int writable(void);
    
    virtual int peek(void);

    /** Get a single byte from the BufferedSerial Port.
     *  Should check readable() before calling this.
     *  @return A byte that came in on the Serial Port
     */
    virtual int getc(void);
    
    /** Write a single byte to the BufferedSerial Port.
     *  @param c The byte to write to the Serial Port
     *  @return The byte that was written to the Serial Port Buffer
     */
    virtual int putc(int c);
    
    /** Write a string to the BufferedSerial Port. Must be NULL terminated
     *  @param s The string to write to the Serial Port
     *  @return The number of bytes written to the Serial Port Buffer
     */
    virtual int puts(const uint8_t *s);
    
    /** Write a formatted string to the BufferedSerial Port.
     *  @param format The string + format specifiers to write to the Serial Port
     *  @return The number of bytes written to the Serial Port Buffer
     */
    virtual int printf(const char* format, ...);
    
    /** Write data to the Buffered Serial Port
     *  @param s A pointer to data to send
     *  @param length The amount of data being pointed to
     *  @return The number of bytes written to the Serial Port Buffer
     */
    virtual ssize_t write(const void *s, std::size_t length);

    virtual void flush(void);
};

#endif

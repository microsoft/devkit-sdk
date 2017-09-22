/*
 * mbed library for RingBuffer
 * Copyright (c) 2010 Hiroshi Suga
 * Released under the MIT License: http://mbed.org/license/mit
 */

/** @file RingBuffer.h
 * @brief Ring Buffer
 */

#ifndef RingBuffer_H
#define RingBuffer_H

#include "mbed.h"

class RingBuffer {
public:
    /** init Stack class
     * @param p_size size of ring buffer
     */
    RingBuffer (int p_size);
    ~RingBuffer ();

    /**Get the number of bytes available int the buffer.
     */
    int available();
    int use();

    /** put to ring buffer
     * @param dat data
     * @return data / -1:error
     */
    int putc(uint8_t data);

    /** put to ring buffer
     * @param dat data
     * @param len length
     * @return put length
     */
    int put(uint8_t *data, int len);

    int peek();
    
    /** get from ring buffer
     * @param dat data
     * @retval 0:ok / -1:error
     */
    int getc();

    /** get from ring buffer
     * @param dat data
     * @param len length
     * @return get length
     */

    int get(uint8_t *data, int len);

    void clear();

private:
    uint8_t *buf;
    int size;
    int addr_w, addr_r;
};

#endif

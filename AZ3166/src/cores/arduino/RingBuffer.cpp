/*
 * mbed library for RingBuffer
 * Copyright (c) 2010 Hiroshi Suga
 * Released under the MIT License: http://mbed.org/license/mit
 */

/** @file RingBuffer.cpp
 * @brief Ring Buffer
 */
 
#include "RingBuffer.h"

RingBuffer::RingBuffer (int p_size)
{
    size = p_size + 1;
    buf = new uint8_t[size];
    addr_w = 0;
    addr_r = 0;
}

RingBuffer::~RingBuffer ()
{
    delete [] buf;
}

int RingBuffer::putc(uint8_t data)
{
    int next;

    next = (addr_w + 1) % size;
    if (next == addr_r) {
        return -1;
    }
    buf[addr_w] = data;
    addr_w = next;
    return -1;
}

int RingBuffer::put(uint8_t *data, int len)
{
    len = len < available() ? len : available();
    if (addr_w + len < size) {
        memcpy(buf + addr_w, data, len);
        addr_w += len;
    }
    else
    {
        int remaining = size - addr_w;
        memcpy(buf + addr_w, data, remaining);
        memcpy(buf, data + remaining, len - remaining);
        addr_w = len - remaining;
    }
    return len;
}

int RingBuffer::peek()
{
    if (addr_r == addr_w) {
        return -1;
    }

    return buf[addr_r];
}

int RingBuffer::getc()
{
    if (addr_r == addr_w) {
        return -1;
    }

    uint8_t data = buf[addr_r];
    addr_r = (addr_r + 1) % size;

    return data;
}

int RingBuffer::get(uint8_t *data, int len)
{
    len = len < use() ? len : use();
    if (addr_r + len < size) {
        memcpy(data, buf + addr_r, len);
        addr_r += len;
    }
    else
    {
        int remaining = size - addr_r;
        memcpy(data, buf + addr_r, remaining);
        memcpy(data + remaining, buf, len - remaining);
        addr_r = len - remaining;
    }
    return len;
}

int RingBuffer::available()
{
    if (addr_w < addr_r) {
        return addr_r - addr_w - 1;
    } else {
        return (size - addr_w) + addr_r - 1;
    }
}

int RingBuffer::use()
{
    if (addr_w < addr_r) {
        return size - (addr_r - addr_w);
    } else {
        return addr_w - addr_r;
    }

    //return size - available() - 1;
}

void RingBuffer::clear()
{
    addr_w = 0;
    addr_r = 0;
}

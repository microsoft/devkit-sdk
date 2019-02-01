/*
 Print.h - Base class that provides print() and println()
 Copyright (c) 2008 David A. Mellis.  All right reserved.

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
 */

#ifndef Print_h
#define Print_h

#include <stdint.h>
#include <stddef.h>

#include "WString.h"
#include "Printable.h"

#define BASE_DEC 10
#define BASE_HEX 16
#define BASE_OCT 8
#define BASE_BIN 2

class Print {
    private:
        int write_error;
        size_t printNumber(unsigned long, uint8_t);
        size_t printFloat(double, uint8_t);
    protected:
        void setWriteError(int err = 1) {
            write_error = err;
        }
    public:
        Print() :
                write_error(0) {
        }

        int getWriteError() {
            return write_error;
        }
        void clearWriteError() {
            setWriteError(0);
        }

        virtual size_t write(unsigned char) = 0;
        virtual size_t write(const unsigned char *buffer, size_t size);
        size_t write(const char *str) {
            if(str == NULL)
                return 0;
            return write((const unsigned char *) str, strlen(str));
        }
        size_t write(const char *buffer, size_t size) {
            return write((const unsigned char *) buffer, size);
        }

        size_t printf(const char * format, ...)  __attribute__ ((format (printf, 2, 3)));
        size_t printf_P(PGM_P format, ...) __attribute__((format(printf, 2, 3)));
        size_t print(const __FlashStringHelper *);
        size_t print(const String &);
        size_t print(const char[]);
        size_t print(char);
        size_t print(unsigned char, int = BASE_DEC);
        size_t print(int, int = BASE_DEC);
        size_t print(unsigned int, int = BASE_DEC);
        size_t print(long, int = BASE_DEC);
        size_t print(unsigned long, int = BASE_DEC);
        size_t print(double, int = 2);
        size_t print(const Printable&);

        size_t println(const __FlashStringHelper *);
        size_t println(const String &s);
        size_t println(const char[]);
        size_t println(char);
        size_t println(unsigned char, int = BASE_DEC);
        size_t println(int, int = BASE_DEC);
        size_t println(unsigned int, int = BASE_DEC);
        size_t println(long, int = BASE_DEC);
        size_t println(unsigned long, int = BASE_DEC);
        size_t println(double, int = 2);
        size_t println(const Printable&);
        size_t println(void);
};

#endif

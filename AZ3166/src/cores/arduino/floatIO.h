// Copyright (c) 2015 Volodymyr Shymanskyy. All rights reserved.
// Licensed under the MIT license.
#ifndef __FLOAT_IO_H__
#define __FLOAT_IO_H__

#ifdef __cplusplus
extern "C"
{
#endif
char *f2s(float f, int p);

char* dtostrf (double val, signed char width, unsigned char prec, char *s);

#ifdef __cplusplus
}
#endif

#endif /* __FLOAT_IO_H__ */

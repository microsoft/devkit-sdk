/**
 ******************************************************************************
 * @file    httpd_sys.c
 * @author  QQ DING
 * @version V1.0.0
 * @date    1-September-2015
 * @brief   System abstraction layer for the HTTP server. The abstraction provides
 *          independence from the details of the filesystem used.
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2014 MXCHIP Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 */

#include "httpd.h"
#include "http-strings.h"
#include "mico.h"

int htsys_getln_soc(int sd, char *data_p, int buflen)
{
	int len = 0;
	char *c_p;
	int result;

//	ASSERT(data_p != NULL);
	c_p = data_p;

	/* Read one byte at a time */
	while ((result = httpd_recv(sd, c_p, 1, 0)) != 0) {
		/* error on recv */
		if (result == -1) {
			*c_p = 0;
			httpd_d("recv failed len: %d", len);
			return -kInProgressErr;
		}

		/* If new line... */
		if ((*c_p == ISO_nl) || (*c_p == ISO_cr)) {
			result = httpd_recv(sd, c_p, 1, 0);
			if ((*c_p != ISO_nl) && (*c_p != ISO_cr)) {
				httpd_d("should get double CR LF: %d, %d",
				      (int)*c_p, result);
			}
			break;
		}
		len++;
		c_p++;

		/* give up here since we'll at least need 3 more chars to
		 * finish off the line */
		if (len >= buflen - 1) {
			httpd_d("buf full: recv didn't read complete line.");
			break;
		}
	}

	*c_p = 0;
	return len;
}

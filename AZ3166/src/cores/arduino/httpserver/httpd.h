/**
 ******************************************************************************
 * @file    httpd.h
 * @author  QQ DING
 * @version V1.0.0
 * @date    1-September-2015
 * @brief   The main HTTPD server thread and its initialization.
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

#ifndef _HTTPD_H_
#define _HTTPD_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "helper.h"
#include "http_parse.h"
#include "httpd_handle.h"
#include "httpd_utility.h"
#include "httpd_wsgi.h"

/** @brief Initialize the httpd
 *
 *  @note  This function must be called before any of the other API functions.  If any
 *  of the initialization steps fail, the function will fail.
 *
 *  @return WM_SUCCESS if successful
 *  @return -WM_FAIL otherwise
 */
int httpd_init(void);

/** @brief  Shutdown the httpd
 *
 *  @note  This function is the opposite of httpd_init().  It cleans up any
 *  resources used by the httpd.  If the httpd thread is running (i.e.,
 *  httpd_stop has not been called), it will be halted.
 *
 *  @return WM_SUCCESS   :if successful
 *  @return -WM_FAIL     :otherwise
 *
 */
int httpd_shutdown(void);

/** @brief   Start the httpd
 *
 *  @note    Upon WM_SUCCESS, the user can expect that the server is running and
 *  ready to receive connections.  httpd_stop() should be called to stop the
 *  server.  A return value of -WM_FAIL generally indicates that the thread
 *  has already been started.
 *
 *  @return   WM_SUCCESS      :if successful
 *  @return   WM_FAIL         :otherwise
 */
int httpd_start(void);

/** @brief    Stop the httpd
  *
  * @note     This function is the opposite of httpd_start().  It stops the httpd.  If
  *  WM_SUCCESS is returned, the server has been stopped and will no longer
  *  accept connections.  -WM_FAIL is returned if the httpd is uninitialized, or
  *  if an internal error occurs (which is not expected).  Note that this
  *  function forces the httpd thread to stop and closes any open sockets. If
  *  a handler is being executed during this time, it will be terminated.
  *
  * @return   WM_SUCCESS      : if successful
  * @return   WM_FAIL         : otherwise
  */
int httpd_stop(void);

/** @brief    Check if httpd is running
 *
 *  @return   0              : if httpd is not running
 *  @return   non-zero       : if it is running
 */
int httpd_is_running(void);

bool httpd_is_https_active( void );

#ifdef __cplusplus
}
#endif

#endif
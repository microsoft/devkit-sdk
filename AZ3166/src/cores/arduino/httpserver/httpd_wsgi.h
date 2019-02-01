/**
 ******************************************************************************
 * @file    httpd_wsgi.h
 * @author  QQ DING
 * @version V1.0.0
 * @date    1-September-2015
 * @brief   This file contains the functions for the WSGI handling of the
 *          httpd. Additionally, two WSGI handlers are also registered. These
 *          handlers perform the basic tasks of parsing the HTTPD headers and
 *          the form data in case of non-WSGI requests.
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

#ifndef _HTTPD_WSGI_H_
#define _HTTPD_WSGI_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "httpd_utility.h"

#define MAX_WSGI_HANDLERS 32

/** @brief  Register a WSGI handler
 *
 *  @note   WSGI handlers declared must be registered with the
 *  httpd by calling this function.
 *
 *  @param[in]   wsgi_call       pointer to a struct httpd_wsgi_call. The memory
 *  location pointed to by this pointer should be available until
 *  the httpd handler is unregistered.
 *
 *  @return  WM_SUCCESS         :if successful
 *  @return -WM_FAIL            :otherwise
 */
int httpd_register_wsgi_handler(struct httpd_wsgi_call *wsgi_call);

/** @brief Register a list of WSGI handlers
 *
 *  @note  WSGI handler list declared must be registered with the
 *  httpd by calling this function.
 *
 *  @param[in] wsgi_call_list     pointer to a list of struct httpd_wsgi_call. The
 *  memory location pointed to by this pointer should be available until
 *  the httpd handler list is unregistered.
 *  @param[in] handler_cnt number of WSGI handlers in wsgi_call_list
 *
 *  @return WM_SUCCESS         :if successful
 *  @return -WM_FAIL           :otherwise
 */
int httpd_register_wsgi_handlers(struct httpd_wsgi_call *wsgi_call_list,
					int handler_cnt);
/** @brief  Unregister a WSGI handler
 *
 *  @note  This call unregisters a WSGI handler.
 *
 *  @param[in] wsgi_call     pointer to a struct httpd_wsgi_call
 *
 *  @return  WM_SUCCESS     :if successful
 *  @return -WM_FAIL        :otherwise
 */
int httpd_unregister_wsgi_handler(struct httpd_wsgi_call *wsgi_call);

/** @brief  Unregister a list of WSGI handlers
 *
 *  @note  WSGI handler list declared can be unregistered with the
 *  httpd by calling this function.
 *
 *  @param[in] wsgi_call_list      list of struct httpd_wsgi_call
 *  @param[in] handler_cnt         number of WSGI handlers in wsgi_call_list
 *
 *  @return  WM_SUCCESS           :if successful
 *  @return  -WM_FAIL             :otherwise
 */
int httpd_unregister_wsgi_handlers(struct httpd_wsgi_call *wsgi_call_list,
					int handler_cnt);

/** @brief Validate an incoming request with a registered handler
 *
 *  @note  This function is used to match the incoming request URI against the one that
 *  is registered with HTTPD
 *
 *  @param[in] req_uri        URI of the incoming HTTP request
 *  @param[in] uri            URI of an handler registered with HTTPD
 *  @param[in] flags          Flag indicating if exact match is required or not \ref
 *  APP_HTTP_FLAGS_NO_EXACT_MATCH
 *
 *  @return    WM_SUCCESS    :if successful
 *  @return    -WM_FAIL      :otherwise
 */
int httpd_validate_uri(char *req_uri, const char *uri, int flags);

#define HTTPD_SEND_BODY_DATA_MAX_LEN 1024

/** @brief Purge the headers of the incoming HTTP request
 *
 *  @note  This function is used to purge the headers of the incoming HTTPD request
 *
 *  Note that it is mandatory to consume all the HTTP headers for the proper
 *  functioning of web server
 *
 *  @param[in] sock        the socket of the incoming HTTP request
 *
 *  @return WM_SUCCESS    :if successful
 *  @return -WM_FAIL      :otherwise
 */
int httpd_purge_headers(int sock);

/** @brief Get the incoming data in case of HTTP POST request
 *
 *  @note  This function first parses the HTTP header tags and then receives the data
 *  in the buffer provided by the user.
 *
 *  It can be called multiple times if the size of user buffer is less than the
 *  actual data that is received. Care has been taken that, in such situations,
 *  the HTTP headers are parsed only once. When it returns zero it indicates
 *  that all the data has been received.
 *
 *  Note that this API is optional and WSGI handlers may prefer to parse the
 *  headers and get the data as per their wish using other HTTPD APIs.
 *
 *  @param[in] req      The incoming HTTP request \ref httpd_request_t
 *  @param[out] content The buffer in which the data is to be received
 *  @param[in] length   The length of the content buffer
 *  @return  The number of bytes still remaining to be read if successful
 *  @return  -WM_FAIL   :otherwise
 */
int httpd_get_data(httpd_request_t *req, char *content, int length);

/* Initialise the WSGI handler data structures */
int httpd_wsgi_init(void);

/* Check if there are any matching WSGI all_wsgi_calls, and if so, execute them. */
int httpd_wsgi(httpd_request_t *req_p);

#ifdef __cplusplus
}
#endif

#endif




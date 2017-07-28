/**
 ******************************************************************************
 * @file    httpd_handle.h
 * @author  QQ DING
 * @version V1.0.0
 * @date    1-September-2015
 * @brief   This is the main processing of an HTTP request. There could be three
 *          types of requests, a WSGI, an SSI or a file. This file contains
 *          routines that deal with this.
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
 *****************************************************************************
 */

#ifndef _HTTPD_HANDLE_H_
#define _HTTPD_HANDLE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "httpd_utility.h"

/** @brief  Helper function to send a buffer over a connection.
 *
 *  @note  WSGI handlers will often want to send data over a connection.  This can be
 *  achieved with the regular send() system call, however that call may not send
 *  all of the data.  This helper function calls send in a loop repeatedly until
 *  all data has been sent or until an error occurs. This function internally
 *  calls tls_send() if TLS is enabled, else it calls send()
 *
 *  @param[in] sock         The socket on which the data is to be sent
 *  @param[in] buf          Pointer to the buffer
 *  @param[in] len          Length of the buffer
 *
 *  @return  WM_SUCCESS     : if successful
 *  @return  -WM_FAIL       : otherwise
 *
 *  \see httpd_send_chunk
 */
int httpd_send(int sock, const char *buf, int len);

/** @brief  Helper function to send an HTTP chunk of data over a connection
 *
 *  @note   This function sends HTTP chunked data. It includes sending the chunked
 *  header (which consists of the chunk size in ascii followed by \\r\\n
 *  characters), then the actual http data and at the end chunk end indicator
 *  (\\r\\n characters)
 *  If the len is passed as 0 to this function, it sends the last chunk i.e.
 *  0\\r\\n
 *
 *  Note that while using chunked data, HTTP Header "Transfer Encoding: Chunked"
 *  should be sent in the response
 *
 *  Send either an HTTP chunk of data or the terminating HTTP chunk.
 *
 *  @param[in]  sock        The socket on which the data is to be sent
 *  @param[in]  buf         Pointer to the buffer or NULL for last chunk
 *  @param[in]  len         Length of the buffer or 0 for last chunk
 *
 *  @return  WM_SUCCESS    : if successful
 *  @return  -WM_FAIL      : otherwise
 *
 */
int httpd_send_chunk(int sock, const char *buf, int len);

/** @brief  Helper function to send a HTTP chunk header
 *
 *  @note  This function is not required when \ref httpd_send_chunk is used.
 *  \ref httpd_send_chunk internally calls this function
 *
 *  This function can be followed by a call to \ref httpd_send to send the
 *  actual content followed by sending the chunk end indicator (\\r\\n)
 *
 *  @param[in] sock           The socket over which chunk header is to be sent
 *  @param[in] size           Length of the chunk to be sent
 *
 *  @return    WM_SUCCESS     :if successful
 *  @return   - WM_FAIL       :otherwise
 *
 */
int httpd_send_chunk_begin(int sock, int size);

/** @brief Send HTTP Status Headers
 *
 *  @note  This function sends the corresponding HTTP headers on the socket.
 *
 *  @param[in] sock          the socket to send headers on
 *  @param[in] stat_code     the status code WM_SUCCESS, -WM_E_HTTPD_HANDLER_404 etc.
 *  @param[in] content_type  the content type \ref http_content_type
 *
 *  @return   WM_SUCCESS     :if successful
 *  @return   -WM_FAIL       :otherwise
 */
int httpd_send_hdr_from_code(int sock, int stat_code,
			     enum http_content_type content_type);


/** @brief Helper function to receive data over a socket
 *
 *  @note  This function abstracts the TCP/IP recv call. We need the abstraction
 *  because recv could be through TLS connection in which case we do not
 *  call the TCP/IP stack 'recv' function directly.
 *
 *  This API is only for the httpd handlers who have been passed the socket
 *  descriptors by httpd.
 *
 *  @param[in]  sock     The socket on which the data is to be received
 *  @param[out] buf      The buffer in which data will be received
 *  @param[in]  n        Number of bytes to be received
 *  @param[in] flags     Same values as that of flag parameter in man page of recv
 *
 *  @return  WM_SUCCESS   :if successful
 *  @return  -WM_FAIL     :otherwise
 */
int httpd_recv(int sock, void *buf, size_t n, int flags);

/** @brief Send the entire HTTP response
 *
 *  @note  This is a helper function which can be used by the WSGI handlers
 *  to send the entire HTTP response.
 *  This function first purges the incoming headers (this step is valid only for
 *  HTTP GET or HEAD request.) Then it sends the first line of the response i.e.
 *  200 OK, 404 Not Found etc. as specified by the user. Further, it sends the
 *  default HTTP headers as per the value of hdr_fields parameter of that WSGI.
 *  After that, it sends the Content-Type HTTP header using the content_type
 *  sent by the user. If the request type is HTTP HEAD, only the headers are
 *  sent out and this function terminates, else data provided by the user
 *  (chunked/non chunked data) follows.
 *
 *  Note that this API is optional and WSGI handlers may prefer to
 *  send the response as per their wish using other HTTPD APIs.
 * 
 *  @param[in] req          The incoming HTTP request \ref httpd_request_t
 *  @param[in] first_line   First line of the response. for e.g.: To send 200 OK
 *  it will be: "HTTP/1.1 200 OK\r\n"
 *  @param[in] content      The data to be sent
 *  @param[in] length       The length of data to be sent
 *  @param[in] content_type The content type of the response. for e.g.:
 *  To send JSON response it will be: "application/json"
 *
 *  @return WM_SUCCESS     :if successful
 *  @return -WM_FAIL       :otherwise
 */
int httpd_send_response(httpd_request_t *req, const char *first_line,
		char *content, int length, const char *content_type);

/** @brief Send HTTP response 301: Moved Permanently
 *
 *  @note  This is a helper function which can be used by the WSGI handlers to send
 *  HTTP response HTTP 301 Moved Permanently. This function first purges the
 *  incoming headers (this step is valid only for HTTP GET or HEAD request.)
 *  Then it sends the first line of the response i.e. 301 Moved Permanently.
 *  Further, it sends the default HTTP headers as per the value of hdr_fields
 *  parameter of that WSGI. After that, it sends the HTTP header Location which
 *  is the new link for redirection. Then, it sends the Content-Type HTTP header
 *  using the content_type sent by the user. If the request type is HTTP HEAD,
 *  only the headers are sent out and this function terminates, else data
 *  provided by the user (chunked/non chunked data) follows.
 *
 *  Note that this API is optional and WSGI handlers may prefer to
 *  send the response as per their wish using other HTTPD APIs.
 *
 *  @param[in] req          The incoming HTTP request \ref httpd_request_t
 *  @param[in] location     The new link for redirection
 *  @param[in] content_type The content type of the response. for e.g.:
 *  To send JSON response it will be: "application/json"
 *  @param[in] content      The data to be sent (optional)
 *  @param[in] content_len  The length of data to be sent
 *
 *  @return WM_SUCCESS      :if successful
 *  @return -WM_FAIL        :otherwise
 */
int httpd_send_response_301(httpd_request_t *req, char *location, const char
		*content_type, char *content, int content_len);

/** @brief Send an HTTP header
 *
 *  @note  This function can be to send out an HTTP header.
 *
 *  It accepts a name value pair as input and sends it out as: "name: value\r\n"
 *  for e.g.:
 *  To send header "Content-Type: application/json\r\n" call
 *  httpd_send_header(sock, "Content-Type", "application/json");
 *
 *  @param[in] sock the socket to send header on
 *  @param[in] name name of the HTTP header
 *  @param[in] value value of the HTTP header 'name'
 *
 *  @return WM_SUCCESS      :if successful
 *  @return -WM_FAIL        :otherwise
 */
int httpd_send_header(int sock, const char *name, const char *value);

/** @brief Send all http header
 *
 *  @note  This function can be to send out all http header.
 *  Only for array types
 *
 *  @return WM_SUCCESS      :if successful
 *  @return -WM_FAIL        :otherwise
 */
int httpd_send_all_header(httpd_request_t *req, const char *first_line, int body_length, const char *content_type);

/** Send an HTTP body
 *
 *  This function can be to send out an HTTP body.
 *  Only for array types
 *
 *  @return WM_SUCCESS       :if successful
 *  @return -WM_FAIL         :otherwise
 */
int httpd_send_body(int sock, const unsigned char *body_image, uint32_t body_size);

/** @brief Send the default HTTP Headers
 *
 *  @note  This function can be used by the WSGI handlers to send out some or all
 *  headers present in \ref httpd_hdr_field_sel_t. The WSGI handlers when
 *  declared can decide which headers they want to send out in the response and
 *  accordingly populate hdr_fields parameter in the httpd_wsgi_call structure
 *
 *  @param[in] sock       The socket to send headers on
 *  @param[in] hdr_fields The hdr_fields parameter of the matching WSGI for the
 *  HTTP request, selected from \ref httpd_hdr_field_sel_t
 *
 *  @return WM_SUCCESS     :if successful
 *  @return -WM_FAIL       :otherwise
 */
int httpd_send_default_headers(int sock, int hdr_fields);

/** Set the httpd-wide error message
 *
 * Often, when something fails, a 500 Internal Server Error will be emitted.
 * This is especially true when the reason is something arbitrary, like the
 * maximum header line that the httpd can handle is exceeded.  When this
 * happens, a function can set the error string to be passed back to the user.
 * This facilitates debugging.  Note that the error message will also be
 * httpd_d'd, so no need to add extra lines of code for that.
 *
 * Note that this function is not re-entrant.
 *
 * Note that at most HTTPD_MAX_ERROR_STRING characters will be stored.
 *
 * Note: no need to have a \r\n on the end of the error message.
 */
#define HTTPD_MAX_ERROR_STRING 256
void httpd_set_error(const char *fmt, ...);

int httpd_handle_message(int conn);

#ifdef __cplusplus
}
#endif
#endif




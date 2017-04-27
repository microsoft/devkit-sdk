/**
 ******************************************************************************
 * @file    http_parse.h
 * @author  QQ DING
 * @version V1.0.0
 * @date    1-September-2015
 * @brief   This maintains the functions that are commonly used by both the
 *          HTTP client and the HTTP server for parsing the HTTP requests.
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

#ifndef __HTTP_PARSE_H__
#define __HTTP_PARSE_H__

#include "httpd_utility.h"

/** @brief Parse the http headers into the req httpd_request_t structure.
 *
 *  @note  This function will one by one read lines terminated by CR-LF (\\r\\n) from
 * the socket and parse the headers contained in them. Some of the headers (at
 * this time only the Content-Length) will be then copied to the req
 * httpd_request_t structure.
 *
 * Note that it is mandatory to consume all the HTTP headers
 *
 * After executing this function, the socket will point to the request entity
 * body, if any.
 *
 *  @param[in] req     The incoming HTTP request \ref httpd_request_t
 *  @param[in] sock    The socket of the incoming HTTP request
 *  @param[in] scratch Buffer used to read the HTTP headers
 *  @param[in] len     Length of the scratch buffer
 *
 *  @return WM_SUCCESS     :if successful
 *  @return -WM_FAIL       :otherwise
 */
int httpd_parse_hdr_tags(httpd_request_t *req, int sock,
			 char *scratch, int len);

/** Parse tag/value form elements present in HTTP POST body
 *
 * Given a tag this function will retrieve its value from the buffer and return
 * it to the caller.
 * \param[in] inbuf Pointer to NULL-terminated buffer that holds POST data
 * \param[in] tag The tag to look for
 * \param[out] val Buffer where the value will be copied to
 * \param[in] val_len The length of the val buffer
 *
 *
 * \return WM_SUCCESS when a valid tag is found, error otherwise
 */
int httpd_get_tag_from_post_data(char *inbuf, const char *tag,
			  char *val, unsigned val_len);

/** Parse tag/value form elements present in HTTP GET URL
 *
 * Given a tag this function will retrieve its value from the HTTP URL and
 * return it to the caller.
 * \param[in] inbuf Pointer to NULL-terminated buffer that holds POST data
 * \param[in] tag The tag to look for
 * \param[out] val Buffer where the value will be copied to
 * \param[in] val_len The length of the val buffer
 *
 * \return WM_SUCCESS when a valid tag is found, error otherwise
 */
int httpd_get_tag_from_url(httpd_request_t *req_p,
			const char *tag, char *val, unsigned val_len);


void httpd_parse_useragent(char *hdrline, httpd_useragent_t *agent);
int httpd_parse_hdr_main(const char *data_p, httpd_request_t *req_p);
#endif /* __HTTP_PARSE_H__ */

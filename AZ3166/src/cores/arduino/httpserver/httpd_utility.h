/**
 ******************************************************************************
 * @file    httpd_utility.h
 * @author  QQ DING
 * @version V1.0.0
 * @date    1-September-2015
 * @brief   This contains basic definitions of httpd
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

#ifndef _HTTPD_UTILITY_H_
#define _HTTPD_UTILITY_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define httpd_d(...)
//#include <json.h>

/** Port on which the httpd listens for non secure connections. */
#define HTTP_PORT 80

/** Port on which the httpd listens for secure connections. */
#define HTTPS_PORT 443

/** Do not perform an exact match of the URI, but ensure that only the prefix is
 * correct. This is required when the registered URI is /nodes/ while the
 * handler wishes to match with /nodes/0/, /nodes/1/ etc.
 */
#define APP_HTTP_FLAGS_NO_EXACT_MATCH   1

/** This REQ_TYPE should not be used externally. */
#define HTTPD_REQ_TYPE_UNKNOWN 0

/** This request type indicates that incoming http request is of type POST. */
#define HTTPD_REQ_TYPE_POST 1

/** This request type indicates that incoming http request is of type GET.
 *
 *  \note GET does not support URL-encoded tag/val pairs.
 */
#define HTTPD_REQ_TYPE_GET 2

/** This request type indicates that incoming http request is of type PUT. */
#define HTTPD_REQ_TYPE_PUT 4

/** This request type indicates that incoming http request is of type DELETE. */
#define HTTPD_REQ_TYPE_DELETE 8

/** This request type indicates that incoming http request is of type HEAD. */
#define HTTPD_REQ_TYPE_HEAD 16

/** HTTPD Error Codes */
enum wm_httpd_errno {
	WM_E_HTTPD_ERRNO_BASE = 0,
	/** Error parsing filename in HTTP Header */
	WM_E_HTTPD_HDR_FNAME,
	/** HTTPD doesn't support the http version */
	WM_E_HTTPD_NOTSUPP,
	/** Post data too long */
	WM_E_HTTPD_TOOLONG,
	/** Data read failed */
	WM_E_HTTPD_DATA_RD,
	/** Handler generated a HTTP 404 File Not Found error */
	WM_E_HTTPD_HANDLER_404,
	/** Handler generated a HTTP 400 Bad Request error */
	WM_E_HTTPD_HANDLER_400,
	/** Handler generated a HTTP 500 Internal Server error */
	WM_E_HTTPD_HANDLER_500,
	/** No handler was found */
	WM_E_HTTPD_NO_HANDLER,
};

/** This value is returned when a WSGI handler is successful to indicate that
 * all processing has been completed.
 */
#define HTTPD_DONE 1

/** Maximum received message length
 *
 *  This value is essentially the longest single line that the HTTPD can
 *  handle.  This limits programmers and users in some interesting ways.  For
 *  example:
 *  -# This is the maximum length of any url-encoded data sent as the body of a
 *     POST request.
 *     If this limit is exceeded, the server will generate a 500 Internal
 *     Server Error response.
 *     Applications that require a higher capacity for form data must implement
 *     their own custom handler to process form data or tune this value.
 *  -# This is the maximum length of a header line that can be processed by
 *     \ref httpd_parse_hdr_tags.  Header lines that exceed this
 *     length will be truncated before being analyzed.  Applications that
 *     require more capacity must implement their own custom handler to process
 *     HTTP header data.
 *  -# This is the maximum length of a single line in a .shtml script.
 *
 *  Developers must ensure that these limits are addressed to avoid unexpected
 *  errors in the field.
 *
 *  \note This value does not include any NULL termination required internally
 *  by the httpd.
 */
#define HTTPD_MAX_MESSAGE 512

/** Maximum URI length
 *
 * This is the maximum supported URI length.  For example, if a client sends a
 * GET /foobar.html, the length of "/foobar.html" must be less than this value.
 * Programmers are obliged to ensure that none of the URIs or HTTPD file names
 * exceed this limit.  If URIs do end up exceeding this limit, a 500 Internal
 * Server Error response will be generated.
 *
 * \note This value does not include any NULL termination required internally
 * by the HTTPD.  However it does include the leading "/".
 */
#define HTTPD_MAX_URI_LENGTH 64


/** Maximum length of the value portion of a tag/value pair
 *
 *  If this length is exceeded, the value will be truncated to this length and
 *  no error will be generated.  If debugging is enabled, however, a warning
 *  will be written to the console.
 *
 *  \note Truncating the user input passed by client may result in unexpected
 *  behavior depending on how a custom handler uses that data.  Programmers
 *  should take steps to validate and/or limit user input using javascript,
 *  input tag properties, or other client-side means.
 *
 *  \note This value does not include the NULL termination of the value.
 *
 */
#define HTTPD_MAX_VAL_LENGTH 64

/** Maximum length of Content-Type
 *
 * httpd_parse_hdr_tags() will parse the headers and store the value of
 * the Content-Type. The maximum value of this field has been arrived
 * keeping in mind the need to support the following type of Content-Type
 * header for working with multipart/form-data for file uploads:
 * Content-Type: multipart/form-data; boundary=\<delimiter_max_length_70\>
 * The length of this line is 104 bytes
 */
#define HTTPD_MAX_CONTENT_TYPE_LENGTH 128

#define ISO_nl      0x0a
#define ISO_cr      0x0d
#define ISO_tab     0x09
#define ISO_space   0x20
#define ISO_bang    0x21
#define ISO_quot    0x22
#define ISO_percent 0x25
#define ISO_period  0x2e
#define ISO_slash   0x2f
#define ISO_colon   0x3a

#define HTTPD_JSON_ERR_MSG    "{\"error_msg\":\"%s\"}"
#define HTTPD_JSON_RETURN_MSG  "{\"return_msg\":\"%s\"}"
#define HTTPD_JSON_ERROR  "{\"error\": -1}"
#define HTTPD_JSON_SUCCESS "{\"success\": 0}"
#define HTTPD_TEXT_ERROR  "error -1\r\n"
#define HTTPD_TEXT_SUCCESS "success\r\n"

/** Content-Type: application/json */
#define HTTP_CONTENT_JSON_STR "application/json"
/** Content-Type: text/xml */
#define HTTP_CONTENT_XML_STR "text/xml"
/** Content-Type: text/hxml */
#define HTTP_CONTENT_HTML_STR "text/html"
/** Content-Type: text/css */
#define HTTP_CONTENT_CSS_STR "text/css"
/** Content-Type: application/x-javascript */
#define HTTP_CONTENT_JS_STR "application/javascript"
/** Content-Type: image/png */
#define HTTP_CONTENT_PNG_STR "image/png"
/** Content-Type: text/plain */
#define HTTP_CONTENT_PLAIN_TEXT_STR "text/plain"

/** HTTP Response: 200 OK */
#define HTTP_RES_200 "HTTP/1.1 200 OK\r\n"
/** HTTP Response: 301 Moved Permanently */
#define HTTP_RES_301 "HTTP/1.1 301 Moved Permanently\r\n"
/** HTTP Response: 304 Not Modified */
#define HTTP_RES_304 "HTTP/1.1 304 Not Modified\r\n"
/** HTTP Response: 400 Bad Request */
#define HTTP_RES_400 "HTTP/1.1 400 Bad Request\r\n"
/** HTTP Response: 404 Not Found */
#define HTTP_RES_404 "HTTP/1.1 404 Not Found\r\n"
/** HTTP Response: 505 HTTP Version Not Supported */
#define HTTP_RES_505 "HTTP/1.1 505 HTTP Version Not Supported\r\n"

/* Are these \r\n required in here? The send chunk will append its own set of
 * \r\n, thus making it two \r\n.
 */

#define HTTPD_XML_ERROR \
	"<?xml version=\"1.0\" encoding=\"US-ASCII\"?><error>%d</error>\r\n"
#define HTTPD_XML_SUCCESS \
	"<?xml version=\"1.0\" encoding=\"US-ASCII\"?><success>0</success>\r\n"

/** Send carriage return and new line characters on the socket \_sock\_ */
#define httpd_send_crlf(_sock_) httpd_send(_sock_, "\r\n", 2);

/** httpd_useragent_t reports the product and version of the client accessing
 * the httpd.
 */
typedef struct  {
	/** Product of the client */
	char product[HTTPD_MAX_VAL_LENGTH + 1];	/* +1 for null termination */
	/** Version of the client */
	char version[HTTPD_MAX_VAL_LENGTH + 1];	/* +1 for null termination */
} httpd_useragent_t;

struct httpd_wsgi_call;

/** Request structure representing various properties of an HTTP request
 */
typedef struct {
	/** HTTP Request type: GET, HEAD, POST, PUT, DELETE */
	int type;
	/** The incoming URI */
	/* +1 is for null termination */
	char filename[HTTPD_MAX_URI_LENGTH + 1];
	/** The socket of the incoming HTTP Request */
	int sock;
	/** Indicator notifying whether the HTTP headers are parsed or not */
	unsigned char hdr_parsed;
	/** The number of data bytes not yet parsed */
	int remaining_bytes;
	/** The size of data in the incoming HTTP Request */
	int body_nbytes;
	/** Pointer to the corresponding wsgi structure */
	const struct httpd_wsgi_call *wsgi;
	/** Set to 1 if chunked data is used */
	unsigned char chunked;
	/** User-agent: Product and Version */
	httpd_useragent_t agent;
	/** The content type of the incoming HTTP Request */
	char content_type[HTTPD_MAX_CONTENT_TYPE_LENGTH];
	/** True if "If-None-Match" header is present in the incoming
	 * HTTP Request */
	bool if_none_match;
	/** Used for storing the etag of an URI */
	unsigned etag_val;
} httpd_request_t;


struct httpd_wsgi_call {
	/** URI of the WSGI */
	const char *uri;
	/** Indicator for HTTP headers to be sent in the response*/
	int hdr_fields;
	/** Flag indicating if exact match of the URI is required or not */
	int http_flags;
	/** HTTP GET or HEAD Handler */
	int (*get_handler) (httpd_request_t *req);
	/** HTTP POST Handler */
	int (*set_handler) (httpd_request_t *req);
	/** HTTP PUT Handler */
	int (*put_handler) (httpd_request_t *req);
	/** HTTP DELETE Handler */
	int (*delete_handler) (httpd_request_t *req);
};

/** HTTP Content types
 */
enum http_content_type {
	/** Content-Type: text/plain */
	HTTP_CONTENT_PLAIN_TEXT,
	/** Content-Type: application/json */
	HTTP_CONTENT_JSON,
	/** Content-Type: text/xml */
	HTTP_CONTENT_XML,
	/** Content-Type: text/html */
	HTTP_CONTENT_HTML,
	/** Content-Type: image/jpeg */
	HTTP_CONTENT_JPEG,
};

/** HTTP Headers to be sent in the HTTP response
 */
typedef enum {
	/** "Server: Marvell-WM\r\n" */
	HTTPD_HDR_ADD_SERVER                    = 0x0001,
	/** "Connection: keep-alive\r\n" */
	HTTPD_HDR_ADD_CONN_KEEP_ALIVE           = 0x0002,
	/** "Connection: close\r\n" */
	HTTPD_HDR_ADD_CONN_CLOSE                = 0x0004,
	/** "Transfer-Encoding: chunked\r\n" */
	HTTPD_HDR_ADD_TYPE_CHUNKED              = 0x0008,
	/** "Cache-Control: no-store, no-cache, must-revalidate\r\n" */
	HTTPD_HDR_ADD_CACHE_CTRL                = 0x0010,
	/** "Cache-Control: post-check=0, pre-check=0\r\n" */
	HTTPD_HDR_ADD_CACHE_CTRL_NO_CHK         = 0x0020,
	/** "Pragma: no-cache\r\n" */
	HTTPD_HDR_ADD_PRAGMA_NO_CACHE           = 0x0040,
} httpd_hdr_field_sel_t;

/** @brief Get the incoming JSON data in case of HTTP POST request
 *
 *  @note  This function is an extension to \ref httpd_get_data. Additionally this
 *  function takes in a pointer to the JSON object and initializes the same. In
 *  this case, content is actually used as a scratch buffer. JSON object is what
 *  you actually need to use after it returns.
 *
 *  Note that this function can be called only once i.e. the size of content
 *  buffer passed to this function should be large enough to store all the
 *  incoming data.
 *
 *  @param[in] req      The incoming HTTP request \ref httpd_request_t
 *  @param[out] content The buffer in which the data is to be received
 *  @param[in] length   The length of the content buffer
 *  @param[out] obj     A pointer to JSON object structure
 *  @return   The number of bytes still remaining to be read (should be 0 if used
 *  appropriately) if successful
 *  @return   -WM_FAIL   : otherwise
 */

typedef struct {
	/* Mandatory. Will be sent to the client */
	const unsigned char *server_cert;
	/* Size of server_cert */
	int server_cert_size;
	/*
	 * Server private key. Mandatory.
	 * For the perusal of the server
	 */
	const unsigned char *server_key;
	/* Size of server_key */
	int server_key_size;
	/*
	 * Needed if the server wants to verify client
	 * certificate. Otherwise set to NULL.
	 */
	const unsigned char *client_cert;
	/* Size of client_cert */
	int client_cert_size;
} httpd_tls_certs_t;

/* Various Defines */
#ifndef NULL
#define NULL 0
#endif

enum {
	HTTP_404,
	HTTP_500,
	HTTP_505,
};
/**
  * @}
  */


/**
  * @}
  */


/*
 * Call this function to set httpd tls certificates.
 */
//int httpd_use_tls_certificates(const httpd_tls_certs_t *httpd_tls_certs);
#endif




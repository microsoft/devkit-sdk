/**
 ******************************************************************************
 * @file    http_parse.c
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

/* http_parse.c: This maintains the functions that are commonly used by both
 * the HTTP client and the HTTP server for parsing the HTTP requests.
 * A typical HTTP request looks like the following:
 *
 * -------------------------
 * GET /index.html HTTP/1.1
 * Content-Length: 35
 * User-Agent: Mozilla/5.0 (Linux)
 * Content-Type: text/html
 * ------------------------
 *
 * The functions within this file do the following job:
 * httpd_parse_hdr_main: parses the first line (starting with GET) into the
 *           httpd_request structure
 * httpd_parse_hdr_tags: parses the other tags that carry the name/value pairs.
 * httpd_parse_useragent: is internally used by httpd_parse_hdr_tags to parse
 *           the User-Agent line above.
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "http_parse.h"
#include "httpd_sys.h"
#include "httpd_wsgi.h"
#include "httpd_handle.h"
#include "http-strings.h"
#include "mico.h"

#define HTTP_CHUNKED	"chunked"

#define iswhite(c) ((((c) == ISO_nl) || ((c) == ISO_cr) || \
((c) == ISO_space) || ((c) == ISO_tab)) ? true : false)

/* Find the next user agent product, version, and comment from hdrline.  Set
* the pointers to NULL if not found.  Return a pointer to the first unparsed
* char, or NULL if the whole string was consumed.
*
* Typically the product is followed by a version, followed by a comment.
*/
static char *__httpd_parse_useragent(char *hdrline, char **product,
				     char **version, char **comment)
{
  char *token = hdrline;
  
  *product = 0;
  *version = 0;
  *comment = 0;
  
  /* User-Agent strings are composed of products and comments.  These are
  * space delimited.  Comments start with '(' and end with ')'.  Products
  * are either of the form "product/version" or just "product".
  */
  
  /* chomp any whitespace and find the beginning of the product */
  while (*token == ' ') {
    if (*token == 0)
      return 0;
    token++;
  }
  *product = token;
  
  while (1) {
    if (*token == 0) {
      /* We reached the end of the string. */
      return 0;
      
    } else if (*token == ' ') {
      /* We're done with this token.  Terminate it.  The next
      * is either an associated comment or the next token.
      */
      *token = 0;
      if (*(token + 1) != '(')
        return token + 1;
      
    } else if (*token == '/') {
      /* Found the version string. */
      *token = 0;
      *version = token + 1;
      
    } else if (*token == '(') {
      token++;
      *comment = token;
      /* comment may have embedded ' 's or '/', so find the
      * end here before continuing.
      */
      while (*token != ')') {
        if (*token == 0) {
          /* unexpected end of comment */
          return 0;
        }
        token++;
      }
      *token = 0;
      return token + 1;
      
    } else if (*token == '[') {
      /* non-standard legacy language code.  Skip it. */
      while (*token != ']') {
        if (*token == 0) {
          /* Unexpected end */
          return 0;
        }
        token++;
      }
    }
    token++;
  }
}

/* Inspect the hdrline and populate the user agent data structure. */

/* Many many many browsers do not abide by the RFC to construct their user
* agent strings.  As a result, parsing the user agent is quite tricky, and no
* authoritative algorithm exists.  For some clues about parsing user agent
* strings, see:
*
* http://www.texsoft.it/index.php?c=software&m=sw.php.useragent&l=it
*
* For an expansive test vector, see:
*
* http://www.pgts.com.au/download/data/browser_list.txt strings that
*
* Note that this test vector is not authoritative; In fact, it often reports
* the incorrect output.  However, it is highly useful as a list of real-life
* inputs.
*/
#define MOZ_COMPATIBLE "compatible;"
void httpd_parse_useragent(char *hdrline, httpd_useragent_t * agent)
{
  char *token, *product, *p, *version, *v, *comment, *dummy;
  int len;
  
  /* Inpsect the first token. */
  hdrline = __httpd_parse_useragent(hdrline, &product,
                                    &version, &comment);
  if (!product)
    goto done;
  
  if (strcmp(product, "Mozilla") == 0) {
    if (comment && strncmp(comment, MOZ_COMPATIBLE, sizeof(MOZ_COMPATIBLE) - 1) == 0) {
      /* If this is a "mozilla compatible" browser, parse it
      * and be done. */
      token = strstr(comment, " ") + 1;
      token = __httpd_parse_useragent(token, &product,
                                      &version, &comment);
      /* Sometimes when the product and version are found
      * within a comment, the version is separated by a space
      * from the product, not the '/'.  This appears as the
      * next product to our token parser.  If it is numeric,
      * assume it's the version.
      */
      if (product && !version && token) {
        token = __httpd_parse_useragent(token, &version,
                                        &dummy,
                                        &comment);
        if (*version < '0' || *version > '9')
          version = 0;
      }
      
      /* Now, some browsers report Mozilla compatible MSIE,
      * but are actually something else!  Keep parsing these
      * ones.  Otherwise, we're done.
      */
      if (strcmp(product, "MSIE") != 0)
        goto done;
    }
    
    if (!hdrline)
      goto done;
    
    /* If this is not a "mozilla compatible" browser, step through
    * the rest of the product/version (comment) tokens and try to
    * figure out what it is.
    */
    while (1) {
      
      hdrline = __httpd_parse_useragent(hdrline, &p,
                                        &v, &comment);
      if (!p ||
          strcmp(p, "Galeon") == 0 ||
            strcmp(p, "safari") == 0 ||
              strcmp(p, "Safari") == 0 ||
                strcmp(p, "Shiira") == 0 ||
                  strcmp(p, "OmniWeb") == 0 ||
                    strcmp(p, "Firefox") == 0 ||
                      strcmp(p, "Camino") == 0 ||
                        strcmp(p, "Phoenix") == 0 ||
                          strcmp(p, "Netscape6") == 0 ||
			    strcmp(p, "Netscape") == 0 ||
                              strcmp(p, "NetFront") == 0 ||
                                strcmp(p, "WebTV") == 0) {
                                  product = p;
                                  version = v;
                                  goto done;
                                } else if (strcmp(p, "Opera") == 0) {
                                  /* Many Opera browsers say "Opera VER" instead
                                  * of Opera/VER */
                                  product = p;
                                  version = v;
                                  if (v == 0 && hdrline) {
                                    hdrline =
                                      __httpd_parse_useragent(hdrline,
                                                              &version,
                                                              &dummy,
                                                              &comment);
                                    if (*version < '0' || *version > '9')
                                      version = 0;
                                  }
                                  goto done;
                                }
      
      if (!hdrline)
        goto done;
    }
  } else if ((strcmp(product, "MSIE") == 0 ||
              strcmp(product, "Netscape")	== 0) &&
             version == 0 && hdrline) {
               /* Some MSIE and Netscape browsers report Mozilla compatbile.
               * Others say PROD/VER.  Others say PROD VER.
               */
               hdrline = __httpd_parse_useragent(hdrline, &version,
                                                 &dummy, &comment);
               if (*version < '0' || *version > '9')
                 version = 0;
             }
  
done:
  if (product) {
    len = strlen(product);
    if (product[len - 1] == ';')
      product[len - 1] = 0;
    strncpy(agent->product, product, HTTPD_MAX_VAL_LENGTH);
    
    /* Sometimes if there's a comment but no version, the first
    * token in the comment is the version.
    */
    if (comment && !version && *comment >= '0' && *comment <= '9') {
      version = comment;
      while (*comment != ' ' && *comment != 0)
        comment++;
      *comment = 0;
    }
  } else
    strncpy(agent->product, "unknown", HTTPD_MAX_VAL_LENGTH);
  
  agent->product[HTTPD_MAX_VAL_LENGTH] = 0;
  
  if (version) {
    len = strlen(version);
    if (version[len - 1] == ';')
      version[len - 1] = 0;
    /* Truncate version strings with unexpected chars */
    dummy = strstr(version, ",");
    if (dummy)
      *dummy = 0;
    dummy = strstr(version, "+");
    if (dummy)
      *dummy = 0;
    strncpy(agent->version, version, HTTPD_MAX_VAL_LENGTH);
  } else
    memset(agent->version, 0, HTTPD_MAX_VAL_LENGTH + 1);
  
  agent->version[HTTPD_MAX_VAL_LENGTH] = 0;
}


/* Parse the individual components of the HTTP header and reflect it in
* httpd_request_t structure. */
static int __httpd_parse_hdr_tags(char *data_p, int len,
				  httpd_request_t *req_p, uint8_t *done)
{
  //	ASSERT((data_p != NULL) && (req_p != NULL));
  
  if ((*data_p == ISO_nl) || (*data_p == ISO_cr) || (*data_p == 0)) {
    /* Indicate that the parsing is now complete */
    *done = 1;
    return kNoErr;
  }
  
  if (strncasecmp(data_p, http_content_len, sizeof(http_content_len) - 1) == 0) {
    req_p->body_nbytes =
      atol(&data_p[sizeof(http_content_len) - 1]);
    if (req_p->body_nbytes == 0) {
      httpd_d("got 0 for body length");
      return kNoErr;
    }
    req_p->remaining_bytes = req_p->body_nbytes;
  } else if (strncasecmp(data_p, http_user_agent, sizeof(http_user_agent) - 1) == 0) {
    /* chomp the : and ' ' and pass it to the internal parser */
    httpd_parse_useragent(data_p + sizeof(http_user_agent) + 1,
                          &req_p->agent);
  } else if (strncasecmp(data_p, http_content_type, sizeof(http_content_type) - 1) == 0) {
    memcpy(req_p->content_type, data_p + strlen(http_content_type),
           strlen(data_p));
  } else if (strncasecmp(data_p, "If-None-Match", sizeof("If-None-Match") - 1) == 0) {
    /*
    * We use the FTFS CRC to generate ETag. Hence, the ETag we
    * receive is not expected to be more that 32 bits in value.
    */
    const char *first_double_quote = strchr(data_p, '"');
    if (!first_double_quote) {
      httpd_d("If_None_Match has no double quote");
      return -kInProgressErr;
    }
    
    const char *etag_start = ++first_double_quote;
    req_p->etag_val = strtol(etag_start, NULL, 16);
    req_p->if_none_match = true;
  } else if (strncasecmp(data_p, http_encoding, sizeof(http_encoding) - 1) == 0) {
    if (!strncasecmp(&data_p[sizeof(http_encoding) - 1],
                     HTTP_CHUNKED, sizeof(HTTP_CHUNKED) - 1))
      req_p->chunked = 1;
  }
  return kNoErr;
}

/* One-by-one read lines terminated by CR-LF from the socket, and
* parse the headers contained in them. */
int httpd_parse_hdr_tags(httpd_request_t *req, int sock, char *buffer, int len)
{
  int req_line_len;
  int err;
  uint8_t done = 0;
  
  while (true) {
    req_line_len = htsys_getln_soc(sock, buffer, len);
    if (req_line_len == -kInProgressErr) {
      httpd_d("Could not read line from socket");
      return -kInProgressErr;
    }
    
    err = __httpd_parse_hdr_tags(buffer, req_line_len, req, &done);
    if (err != kNoErr)
      return err;
    if (done == 1) {
      if (req->agent.product[0] != 0) {
        /* Populate the diagnostics statistics variable
        * with the user agent string.  Do not do
        * anything if the agent string is NULL
        * (http-client). This is required since this
        * code is used by http server as well as client
        * and we do not want the user agent diagnostics
        * variable to change during the client code
        * path
        */
        /* Note that, currently wm_hd_useragent is
        * populated only for file handling requests and
        * HTTP POST requests and not for HTTP GET. This
        * is because, in GET requests we directly purge
        * all the headers without reading them and
        * hence do not populate the useragent */
        //				memcpy(&g_wm_stats.wm_hd_useragent, &req->agent,
        //				       sizeof(g_wm_stats.wm_hd_useragent));
        //				g_wm_stats.wm_hd_time = wmtime_time_get_posix();
      }
      return kNoErr;
    }
  }
}


/* Get a pointer to the next token in a request. A token is
* delimited by whitespace (tab space or newline cr or '\0'. We assume that
* all databuffers are null terminated
*/
static const char *httpd_next_token(const char *data_p)
{
  //	ASSERT(data_p != NULL);
  char c;
  
  /* skip over current token, but return if we're at the end of the
  * data */
  while (true) {
    c = *data_p;
    if (c == '\0')
      return data_p;
    if (iswhite(c))
      break;
    data_p++;
  }
  
  /* skip over white spaces after last token */
  while (true) {
    c = *data_p;
    if ((c == '\0') || (!iswhite(c)))
      break;
    data_p++;
  }
  
  /* return the token */
  return (char *)data_p;
}

/* Extract a token from data_p and copy it into dest_p.
*/
static int httpd_token_cpy(const char *data_p, char *dest_p, int dest_len)
{
  int i = 0;
  
  //	ASSERT(data_p != NULL);
  
  while (!iswhite(*data_p) && i < dest_len && *data_p != '\0') {
    dest_p[i] = *data_p;
    i++;
    data_p++;
  }
  
  if (!iswhite(*data_p) && i == dest_len) {
    /* Token too long to fit in dest_len */
    return -kInProgressErr;
  }
  
  /* Add trailing \0 */
  dest_p[i] = 0;
  return kNoErr;
  
}

/* Verify that the request type is valid and then set the request in
* the request struct passed in.
*/
static int httpd_verify_req_type(const char *token_p, int *type)
{
  //	ASSERT(token_p != NULL);
  
  if (strncmp(token_p, http_post, sizeof(http_post) - 1) == 0)
    *type = HTTPD_REQ_TYPE_POST;
  else if (strncmp(token_p, http_get, sizeof(http_get) - 1) == 0)
    *type = HTTPD_REQ_TYPE_GET;
  else if (strncmp(token_p, http_put, sizeof(http_put) - 1) == 0)
    *type = HTTPD_REQ_TYPE_PUT;
  else if (strncmp(token_p, http_delete, sizeof(http_delete) - 1) == 0)
    *type = HTTPD_REQ_TYPE_DELETE;
  else if (strncmp(token_p, http_head, sizeof(http_head) - 1) == 0)
    *type = HTTPD_REQ_TYPE_HEAD;
  else {
    *type = HTTPD_REQ_TYPE_UNKNOWN;
    return -kInProgressErr;
  }
  
  return kNoErr;
}

/* Parse the main header (GET /index.html HTTP/1.1) within the HTTP header and
* update the httpd_request structure with the same.
*/
int httpd_parse_hdr_main(const char *data_p, httpd_request_t * req_p)
{
  const char *ptr, *next;
  int err;
#define TOKEN_LEN 10
  char token[TOKEN_LEN];
  
  /* First token is usually GET/POST */
  err = httpd_verify_req_type(data_p, &req_p->type);
  if (err != kNoErr) {
    httpd_d("Unknown request type %s", data_p);
    httpd_purge_headers(req_p->sock);
    return err;
  }
  
  /* Second token: Filename */
  ptr = httpd_next_token(data_p);
  next = ptr;
  
  err = httpd_token_cpy(ptr, req_p->filename, HTTPD_MAX_URI_LENGTH);
  if (err != kNoErr) {
    httpd_set_error("Error processing token: filename. "
                    "Filename missing or too long?");
    return -WM_E_HTTPD_HDR_FNAME;
  }
  
  /* Third token: HTTP version */
  ptr = httpd_next_token(next);
  next = ptr;
  
  err = httpd_token_cpy(ptr, token, TOKEN_LEN);
  if (!strncmp(token, http_10, TOKEN_LEN)) {
    httpd_set_error("HTTP/1.0 clients are not supported");
    return -WM_E_HTTPD_NOTSUPP;
  }
  
  return kNoErr;
}



/* Utility function to convert a character to the nibble. This is used for
* converting encoded characters to data. */
static int char2nibble(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  else if (c >= 'A' && c <= 'F')
    return c - 'A' + 0xA;
  else if (c >= 'a' && c <= 'f')
    return c - 'a' + 0xa;
  
  return -1;
}

/* Parse a tag value pair. tag1=value1&tag2=value2
*/
static char *httpd_parse_msgbody(char *data_p, const char *tag,
                                 char *val, unsigned val_len, short *found)
{
  int i, c1, c2;
  char *ptr;
  const char *tag_p;
  
  *found = 0;
  
  if (data_p == NULL || *data_p == '\0')
    return NULL;
  
  ptr = data_p;
  tag_p = tag;
  while (1) {
    if (*ptr == 0) {
      httpd_d("End of string reached");
      return NULL;
    }
    if (*ptr == '=') {
      /* End of tag */
      if (*tag_p != 0)
        goto next_token;
      else
        break;
    }
    /* perform url decode */
    if (*ptr == '%') {
      ptr++;
      c1 = char2nibble(*ptr++);
      c2 = char2nibble(*ptr++);
      if (c1 == -1 || c2 == -1) {
        httpd_d("Invalid URL-encoded"
                " string. Ignoring.");
        return NULL;
      }
      if (*tag_p != ((c1 << 4) | c2))
        goto next_token;
      tag_p++;
    } else if (*ptr == '+') {
      if (*tag_p != ' ')
        goto next_token;
      tag_p++;
      ptr++;
    } else {
      if (*tag_p != *ptr)
        goto next_token;
      ptr++;
      tag_p++;
    }
  }
  
  /* We come here only if tag was found */
  *found = 1;
  ptr = strchr(ptr, '=');	/* skip to next token */
  if (ptr == NULL) {
    httpd_d("No = after tag");
    return NULL;
  }
  ptr++;
  
  for (i = 0; i < (int)val_len; i++) {
    if ((*ptr == '&') || (*ptr == 0)) {
      val[i] = 0;
      break;
    }
    
    /* perform url decode */
    if (*ptr == '%') {
      ptr++;
      c1 = char2nibble(*ptr++);
      c2 = char2nibble(*ptr++);
      if (c1 == -1 || c2 == -1) {
        httpd_d("Invalid URL-encoded"
                " string. Ignoring.");
        return NULL;
      }
      val[i] = (c1 << 4) | c2;
    } else if (*ptr == '+') {
      val[i] = ' ';
      ptr++;
    } else {
      val[i] = *ptr++;
    }
  }
  
  if (i == (int)val_len) {
    val[val_len] = 0;
    if ((*ptr != '&') && (*ptr != 0)) {
      httpd_d("Max val length exceeded.  "
              "Truncating value ");
    }
  }
  
next_token:
  ptr = strchr(ptr, '&');	/* skip to next token */
  
  if (ptr == NULL) {
    return NULL;
  }
  
  return ptr + 1;
}

static int __httpd_parse_all_tags(char *inbuf, const char *tag,
				  char *val, unsigned val_len)

{
  short found = 0;
  char *ptr;
  
  ptr = inbuf;
  while (1) {
    ptr = httpd_parse_msgbody(ptr, tag, val, val_len, &found);
    if (ptr == NULL) {
      httpd_d("Failed to parse request");
      return -kInProgressErr;
    }
    if (found)
      return kNoErr;
    
    if (ptr == NULL)
      break;
  }
  
  return -kInProgressErr;
}


/* Lookup tags in HTTP POST Buffer
*/
int httpd_get_tag_from_post_data(char *inbuf, const char *tag,
				 char *val, unsigned val_len)
{
  if (val_len <= 0)
    return -kInProgressErr;
  *val = '\0';
  return __httpd_parse_all_tags(inbuf, tag, val, val_len);
}

/* Lookup tags in HTTP GET URL
*/
int httpd_get_tag_from_url(httpd_request_t *req_p, const char *tag,
                           char *val, unsigned val_len)
{
  char *p;
  
  if (val_len <= 0)
    return -kInProgressErr;
  
  *val = '\0';
  p = strchr(req_p->filename, '?');
  /* No tag=value pairs were present in the url */
  if (!p)
    return -kInProgressErr;
  
  return __httpd_parse_all_tags(p + 1, tag, val, val_len);
}

/**
 ******************************************************************************
 * @file    app_https.c
 * @author  QQ DING
 * @version V1.0.0
 * @date    1-September-2015
 * @brief   The main HTTPD server initialization and wsgi handle.
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2016 MXCHIP Inc.
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
 *
 ******************************************************************************
 */

#include <httpd.h>
#include "mico.h"
#include "app_httpd.h"

#define app_httpd_log(...) 

#define HTTPD_HDR_DEFORT (HTTPD_HDR_ADD_SERVER|HTTPD_HDR_ADD_CONN_CLOSE|HTTPD_HDR_ADD_PRAGMA_NO_CACHE)
bool is_http_init;
bool is_handlers_registered;
struct httpd_wsgi_call g_app_handlers[];

int web_send_wifisetting_page(httpd_request_t *req)
{
  int setting_page_len = 0;
  char *setting_page = NULL;
  int err = kNoErr;
  char *ssid = "";

  setting_page_len = strlen(page_head) + strlen(wifi_setting_a) + strlen(wifi_setting_b) + strlen(ssid) + 1;
  setting_page = malloc(setting_page_len);
  snprintf(setting_page, setting_page_len, "%s%s%s%s", page_head, wifi_setting_a, ssid, wifi_setting_b);

  err = httpd_send_all_header(req, HTTP_RES_200, setting_page_len, HTTP_CONTENT_HTML_STR);
  require_noerr_action( err, exit, app_httpd_log("ERROR: Unable to send http wifisetting headers.") );
  
  err = httpd_send_body(req->sock, (const unsigned char*)setting_page, setting_page_len);
  require_noerr_action( err, exit, app_httpd_log("ERROR: Unable to send http wifisetting body.") );
  
exit:
  if (setting_page) free(setting_page);
  return err; 
}

int web_send_result(httpd_request_t *req, bool is_success)
{
  int result_page_len = 0;
  char *result_page = NULL;
  OSStatus err = kNoErr;
  const char *result_body = is_success ? success_result : failed_result;

  result_page_len = strlen(page_head) + strlen(result_body) + 1;
  result_page = malloc(result_page_len);
  snprintf(result_page, result_page_len, "%s%s", page_head, result_body);

  err = httpd_send_all_header(req, HTTP_RES_200, result_page_len, HTTP_CONTENT_HTML_STR);
  require_noerr_action( err, exit, app_httpd_log("ERROR: Unable to send http result headers.") );
  
  err = httpd_send_body(req->sock, (const unsigned char*)result_page, result_page_len);
  require_noerr_action( err, exit, app_httpd_log("ERROR: Unable to send http result body.") );

exit:
  if (result_page) free(result_page);
  return err; 
}

int web_send_wifisetting_result_page(httpd_request_t *req)
{
  OSStatus err = kNoErr;
  bool para_succ = false;
  int buf_size = 512;
  char *buf;
  char value_ssid[maxSsidLen];
  char value_pass[maxKeyLen];
  char *boundary = NULL;
  // mico_Context_t* context = NULL;

  buf = malloc(buf_size);
  err = httpd_get_data(req, buf, buf_size);
  app_httpd_log("httpd_get_data return value: %d", err);
  require_noerr( err, Save_Out );
  
  if (strstr(req->content_type, "multipart/form-data") != NULL) // Post data is multipart encoded
  {
    boundary = strstr(req->content_type, "boundary=");
    boundary += 9;

    err = httpd_get_tag_from_multipart_form(buf, boundary, "SSID", value_ssid, maxSsidLen);
    require_noerr( err, Save_Out );

    if(!strncmp(value_ssid, "\0", 1)) goto Save_Out;

    err = httpd_get_tag_from_multipart_form(buf, boundary, "PASS", value_pass, maxKeyLen);
    require_noerr( err, Save_Out );
  }
  else // Post data is URL encoded
  {
    err = httpd_get_tag_from_post_data(buf, "SSID", value_ssid, maxSsidLen);
    require_noerr( err, Save_Out );

    if(!strncmp(value_ssid, "\0", 1)) goto Save_Out;

    err = httpd_get_tag_from_post_data(buf, "PASS", value_pass, maxKeyLen);
    require_noerr( err, Save_Out );
  }

  printf("<%s> <%s>\r\n", value_ssid, value_pass);
  para_succ = true;
  
Save_Out:
  
  if(para_succ == true)
  {
    err = web_send_result(req, true);
    require_noerr_action(err, exit, app_httpd_log("ERROR: Unable to send http success result"));

    // mico_system_power_perform(context, eState_Software_Reset);
  }
  else
  {
    err = web_send_result(req, false);
    require_noerr_action(err, exit, app_httpd_log("ERROR: Unable to send http failed result"));  
  }
  
exit:  
  if(buf) free(buf);
  return err; 
}

struct httpd_wsgi_call g_app_handlers[] = {
  {"/", HTTPD_HDR_DEFORT, 0, web_send_wifisetting_page, NULL, NULL, NULL},
  {"/result", HTTPD_HDR_DEFORT, 0, NULL, web_send_wifisetting_result_page, NULL, NULL},
  {"/setting", HTTPD_HDR_DEFORT, 0, web_send_wifisetting_page, NULL, NULL, NULL},
};

int g_app_handlers_no = sizeof(g_app_handlers)/sizeof(struct httpd_wsgi_call);

void app_http_register_handlers()
{
  int rc;
  rc = httpd_register_wsgi_handlers(g_app_handlers, g_app_handlers_no);
  if (rc) {
    app_httpd_log("failed to register test web handler");
  }
}

int _app_httpd_start()
{
  OSStatus err = kNoErr;
  app_httpd_log("initializing web-services");
  
  /*Initialize HTTPD*/
  if(is_http_init == false) {
    err = httpd_init();
    require_noerr_action( err, exit, app_httpd_log("failed to initialize httpd") );
    is_http_init = true;
  }
  
  /*Start http thread*/
  err = httpd_start();
  if(err != kNoErr) {
    app_httpd_log("failed to start httpd thread");
    httpd_shutdown();
  }
exit:
  return err;
}

int httpd_server_start()
{
  int err = kNoErr;
  err = _app_httpd_start();
  require_noerr( err, exit ); 
  
  if (is_handlers_registered == false) {
    app_http_register_handlers();
    is_handlers_registered = true;
  }
  
exit:
  return err;
}

int app_httpd_stop()
{
  OSStatus err = kNoErr;
  
  /* HTTPD and services */
  app_httpd_log("stopping down httpd");
  err = httpd_stop();
  require_noerr_action( err, exit, app_httpd_log("failed to halt httpd") );
  
exit:
  return err;
}

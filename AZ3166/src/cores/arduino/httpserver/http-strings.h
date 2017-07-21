/**
 ******************************************************************************
 * @file    http-string.h.c
 * @author  QQ DING
 * @version V1.0.0
 * @date    1-September-2015
 * @brief   String associated with HTTP
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

extern const char http_http[8];
extern const char http_200[5];
extern const char http_301[5];
extern const char http_302[5];
extern const char http_get[5];
extern const char http_post[6];
extern const char http_put[5];
extern const char http_delete[8];
extern const char http_head[6];
extern const char http_10[9];
extern const char http_11[9];
extern const char http_last_chunk[6];
extern const char http_content_type[15];
extern const char http_content_len[17];
extern const char http_encoding[20];
extern const char http_texthtml[10];
extern const char http_location[11];
extern const char http_host[7];
extern const char http_crnl[3];
extern const char http_index_html[12];
extern const char http_404_html[10];
extern const char http_referer[9];
extern const char http_header_server[];
extern const char http_header_conn_close[];
extern const char http_header_conn_keep_alive[];
extern const char http_header_keep_alive_ctrl[];
extern const char httpd_authorized[];
extern const char http_header_type_chunked[];
extern const char http_header_cache_ctrl[];
extern const char http_header_cache_ctrl_no_chk[];
extern const char http_header_pragma_no_cache[];
extern const char http_header_200_keepalive[66];
extern const char http_header_200[];
extern const char http_header_304_prologue[];
extern const char http_header_404[];
extern const char http_header_400[];
extern const char http_header_500[];
extern const char http_header_505[];
extern const char http_content_type_plain[27];
extern const char http_content_type_html[26];
extern const char http_content_type_charset_html[45];
extern const char http_content_type_html_nocache[157];
extern const char http_content_type_css[25];
extern const char http_content_type_text[28];
extern const char http_content_type_png[26];
extern const char http_content_type_gif[26];
extern const char http_content_type_jpg[27];
extern const char http_content_type_js[32];
extern const char http_content_type_binary[41];
extern const char http_content_type_binary_nocrlf[41];
extern const char http_content_type_json[35];
extern const char http_content_type_json_nocrlf[33];
extern const char http_content_type_json_nocache[147];
extern const char http_content_type_xml_nocache[139];
extern const char http_content_type_form[52];
extern const char http_content_type_form_nocrlf[50];
extern const char http_content_type_text_cache_manifest[36];
extern const char http_content_encoding_gz[48];
extern const char http_html[6];
extern const char http_shtml[7];
extern const char http_htm[5];
extern const char http_css[5];
extern const char http_png[5];
extern const char http_gif[5];
extern const char http_jpg[5];
extern const char http_text[5];
extern const char http_txt[5];
extern const char http_xml[5];
extern const char http_js[4];
extern const char http_gz[4];
extern const char http_manifest[10];
extern const char http_user_agent[11];
extern const char http_if_modified_since[18];
extern const char http_cache_control[28];

/* 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 
#include "http_client.h"
#include "SystemWiFi.h"

const char CERT[] = 
"-----BEGIN CERTIFICATE-----\r\nMIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\r\n"
"RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\r\nVQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\r\n"
"DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\r\nZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\r\n"
"VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\r\nmD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\r\n"
"IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\r\nmpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\r\n"
"XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\r\ndc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\r\n"
"jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\r\nBE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\r\n"
"DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\r\n9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\r\n"
"jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\r\nEpn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\r\n"
"ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\r\nR9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\r\n-----END CERTIFICATE-----\r\n";

HTTPClient::HTTPClient(http_method method, const char* url, Callback<void(const char *at, size_t length)> body_callback)
{
    init(CERT, method, url, body_callback);
}

HTTPClient::HTTPClient(const char* ssl_ca_pem, http_method method, const char* url, Callback<void(const char *at, size_t length)> body_callback)
{
    init(ssl_ca_pem, method, url, body_callback);
}

HTTPClient::~HTTPClient()
{
    if (_https_request)
    {
        delete _https_request;
    }
    if (_response)
    {
        delete _response;
    }
}

const Http_Response* HTTPClient::send(const void* body, int body_size)
{
    if (_https_request != NULL)
    {
        HttpResponse *response = _https_request->send(body, body_size);
        if (response != NULL)
        {
            _response->status_code = response->get_status_code();
            _response->status_message = response->get_status_message();
            _response->body = response->get_body();
            _response->headers = response->get_headers();
            _response->body_length = response -> get_body_length();
            return _response;
        }
    }
    
    return NULL;
}

void HTTPClient::set_header(const char* key, const char* value)
{
    if (_https_request != NULL)
    {
        _https_request->set_header(key, value);
    }
}

nsapi_error_t HTTPClient::get_error()
{
    if (_https_request != NULL)
    {
        return _https_request->get_error();
    }
    return -1;
}

void HTTPClient::init(const char* ssl_ca_pem, http_method method, const char* url, Callback<void(const char *at, size_t length)> body_callback)
{
    _https_request = NULL;
    _response = new Http_Response;
    if (strlen(url) >= 5 && (strncmp("http:", url, 5) == 0))
    {
        _https_request = new HttpsRequest(WiFiInterface(), NULL, method, url, body_callback);
    }
    else if(strlen(url) >= 6 && (strncmp("https:", url, 6) == 0))
    {
        _https_request = new HttpsRequest(WiFiInterface(), ssl_ca_pem, method, url, body_callback);
    }
}
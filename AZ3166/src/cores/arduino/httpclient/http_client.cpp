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
/* DigiCert Global Root CA */
"-----BEGIN CERTIFICATE-----\r\n""MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\r\n"
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n""d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\r\n"
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\r\n""MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\r\n"
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\r\n""9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\r\n"
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\r\n""nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\r\n"
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\r\n""T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\r\n"
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\r\n""BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\r\n"
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\r\n""DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\r\n"
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\r\n""06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\r\n"
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\r\n""YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\r\n"
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\r\n""-----END CERTIFICATE-----\r\n";

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
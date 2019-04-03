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
#include "mbed.h"
#include "mico.h"
#include "app_httpd.h"
#include "EEPROMInterface.h"
#include "EMW10xxInterface.h"
#include "httpd.h"
#include "OledDisplay.h"
#include "SystemVariables.h"
#include "SystemWeb.h"

#define HTTPD_HDR_DEFORT (HTTPD_HDR_ADD_SERVER|HTTPD_HDR_ADD_CONN_CLOSE|HTTPD_HDR_ADD_PRAGMA_NO_CACHE)

#define DEFAULT_PAGE_SIZE (10*1024)

static const char * page_head = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\"><title>AZ3166 WiFi Config</title><style>@charset \"UTF-8\";/*Flavor name:Default (mini-default)Author:Angelos Chalaris (chalarangelo@gmail.com)Maintainers:Angelos Chalarismini.css version:v2.1.5 (Fermion)*//*Browsers resets and base typography.*/html{font-size:16px;}html, *{font-family:-apple-system, BlinkMacSystemFont,\"Segoe UI\",\"Roboto\", \"Droid Sans\",\"Helvetica Neue\", Helvetica, Arial, sans-serif;line-height:1.5;-webkit-text-size-adjust:100%;}*{font-size:1rem;}body{margin:0;color:#212121;background:#f8f8f8;}section{display:block;}input{overflow:visible;}[type=\"radio\"]{position:absolute;left:-2rem;}h1, h2{line-height:1.2em;margin:0.75rem 0.5rem;font-weight:500;}h2 small{color:#424242;display:block;margin-top:-0.25rem;}h1{font-size:2rem;}h2{font-size:1.6875rem;}p{margin:0.5rem;}small{font-size:0.75em;}a{color:#0277bd;text-decoration:underline;opacity:1;transition:opacity 0.3s;}a:visited{color:#01579b;}a:hover, a:focus{opacity:0.75;}/*Definitions for the grid system.*/.container{margin:0 auto;padding:0 0.75rem;}.row{box-sizing:border-box;display:-webkit-box;-webkit-box-flex:0;-webkit-box-orient:horizontal;-webkit-box-direction:normal;display:-webkit-flex;display:flex;-webkit-flex:0 1 auto;flex:0 1 auto;-webkit-flex-flow:row wrap;flex-flow:row wrap;}[class^='col-sm-']{box-sizing:border-box;-webkit-box-flex:0;-webkit-flex:0 0 auto;flex:0 0 auto;padding:0 0.25rem;}.col-sm-10{max-width:83.33333%;-webkit-flex-basis:83.33333%;flex-basis:83.33333%;}.col-sm-offset-1{margin-left:8.33333%;}@media screen and (min-width:768px){.col-md-4{max-width:33.33333%;-webkit-flex-basis:33.33333%;flex-basis:33.33333%;}.col-md-offset-4{margin-left:33.33333%;}}/*Definitions for navigation elements.*/header{display:block;height:2.75rem;background:#1e6bb8;color:#f5f5f5;padding:0.125rem 0.5rem;white-space:nowrap;overflow-x:auto;overflow-y:hidden;}header .logo{color:#f5f5f5;font-size:1.35rem;line-height:1.8125em;margin:0.0625rem 0.375rem 0.0625rem 0.0625rem;transition:opacity 0.3s;}header .logo{text-decoration:none;}/*Definitions for forms and input elements.*/form{background:#eeeeee;border:1px solid #c9c9c9;margin:0.5rem;padding:0.75rem 0.5rem 1.125rem;}.input-group{display:inline-block;margin-left:2rem;position:relative;}.input-group.fluid{display:-webkit-box;-webkit-box-pack:justify;display:-webkit-flex;display:flex;-webkit-align-items:center;align-items:center;-webkit-justify-content:center;justify-content:center;}.input-group.fluid>input:not([type=\"radio\"]),.input-group.fluid>textarea{-webkit-box-flex:1;width:100%;-webkit-flex-grow:1;flex-grow:1;-webkit-flex-basis:0;flex-basis:0;}@media screen and (max-width:767px){.input-group.fluid{-webkit-box-orient:vertical;-webkit-align-items:stretch;align-items:stretch;-webkit-flex-direction:column;flex-direction:column;}}[type=\"password\"],[type=\"text\"],select,textarea{width:100%;box-sizing:border-box;background:#fafafa;color:#212121;border:1px solid #c9c9c9;border-radius:2px;margin:0.25rem 0;padding:0.5rem 0.75rem;}input:not([type=\"button\"]):not([type=\"submit\"]):not([type=\"reset\"]):hover, input:not([type=\"button\"]):not([type=\"submit\"]):not([type=\"reset\"]):focus, select:hover, select:focus{border-color:#0288d1;box-shadow:none;}input:not([type=\"button\"]):not([type=\"submit\"]):not([type=\"reset\"]):disabled, select:disabled{cursor:not-allowed;opacity:0.75;}::-webkit-input-placeholder{opacity:1;color:#616161;}::-moz-placeholder{opacity:1;color:#616161;}::-ms-placeholder{opacity:1;color:#616161;}::placeholder{opacity:1;color:#616161;}button::-moz-focus-inner, [type=\"submit\"]::-moz-focus-inner{border-style:none;padding:0;}button, [type=\"submit\"]{-webkit-appearance:button;}button{overflow:visible;text-transform:none;}button, [type=\"submit\"], a.button, .button{display:inline-block;background:rgba(208, 208, 208, 0.75);color:#212121;border:0;border-radius:2px;padding:0.5rem 0.75rem;margin:0.5rem;text-decoration:none;transition:background 0.3s;cursor:pointer;}button:hover, button:focus, [type=\"submit\"]:hover, [type=\"submit\"]:focus, a.button:hover, a.button:focus, .button:hover, .button:focus{background:#d0d0d0;opacity:1;}button:disabled, [type=\"submit\"]:disabled, a.button:disabled, .button:disabled{cursor:not-allowed;opacity:0.75;}/*Custom elements for forms and input elements.*/button.primary, [type=\"submit\"].primary, .button.primary{background:rgba(30, 107, 184, 0.9);color:#fafafa;}button.primary:hover, button.primary:focus, [type=\"submit\"].primary:hover, [type=\"submit\"].primary:focus, .button.primary:hover, .button.primary:focus{background:#0277bd;}#content{margin-top:2em;} table, th, td {border:1px solid #c9c9c9; border-collapse:collapse;} th, td {padding:5px;padding-left:10px} td {text-align: left;} th {background-color:#EEEEEE;color: #616161;} tr {background-color: #EEEEEE;color: #616161;}</style></head>";
static const char * wifi_setting_a = "<body><header><h1 class=\"logo\">IoT DevKit Settings</h1></header><section class=\"container\"><div id=\"content\" class=\"row\"><div class=\"col-sm-10 col-sm-offset-1 col-md-4 col-md-offset-4\" style=\"text-align:center;\"><form action=\"result\" method=\"post\" enctype=\"multipart/form-data\"><div class=\"input-group fluid\"><input type=\"radio\" name=\"input_ssid_method\" value=\"select\" onclick=\"changeSSIDInput()\" checked><select name=\"SSID\" id=\"SSID-select\"> ";
static const char * wifi_setting_b = "</select></div><div class=\"input-group fluid\"><input type=\"radio\" name=\"input_ssid_method\" value=\"text\" onclick=\"changeSSIDInput()\"><input type=\"text\" id=\"SSID-text\" placeholder=\"SSID\" disabled></div><div class=\"input-group fluid\"><input type=\"password\" value=\"\" name=\"PASS\" id=\"password\" placeholder=\"Password\"></div>";
static const char * device_conn_setting = "<div class=\"input-group fluid\"><input type=\"text\" name=\"DeviceConnectionString\" id=\"DeviceConnectionString\" placeholder=\"IoT Device Connection String\"></div>";
static const char * cert_setting = "<div class=\"input-group fluid\"><textarea name=\"certificate\" rows=\"5\" placeholder=\"X.509 Certificate\"></textarea></div>";
static const char * setting_end = "<div class=\"input-group fluid\"><button type=\"submit\" class=\"primary\">Save</button></div></form><h5 style=\"color:#616161;\">Please refresh this page to update SSID if you cannot find it from the list</h5></div></div></section><script>function changeSSIDInput(){var inputFromSelect=document.getElementsByName(\"input_ssid_method\")[0].checked;var select=document.getElementById(\"SSID-select\");var text=document.getElementById(\"SSID-text\");if(inputFromSelect){select.name=\"SSID\";select.removeAttribute(\"disabled\");text.name=\"\";text.setAttribute(\"disabled\",\"\")}else{select.name=\"\";select.setAttribute(\"disabled\",\"\");text.name=\"SSID\";text.removeAttribute(\"disabled\")}};</script></body></html>";

static const char * result_head = "<body><header> <h1 class=\"logo\">IoT DevKit Settings</h1></header><section class=\"container\"> <div id=\"content\" class=\"row\"> <div class=\"col-sm-10 col-sm-offset-1 col-md-4 col-md-offset-4\" style=\"text-align:center;\"><table align=\"center\" style=\"width:80%\"><tr><th>Settings</td></tr>";
static const char * result_wifi = "<tr><td>Wi-Fi SSID and Password - %s</th></tr>";
static const char * result_conn_string = "<tr><td>IoT Device Connection String - %s</td></tr>";
static const char * result_cert = "<tr><td>X.509 Certificate - %s</td></tr>";
static const char * result_table_end = "</table>";
static const char * result_wifi_connect_1 = "<h3 style=\"color:Tomato;\">Failed to connect： %s</h3>";
static const char * result_wifi_connect_2 = "<h3 style=\"color:DodgerBlue;\">Wi-Fi %s is connected: %s</h3>";
static const char * result_retry = "<p><a href=\"/\" class=\"button primary\" target=\"_self\">Retry</a></p>";
static const char * result_wait = "<h5 style=\"color:#616161;\">Wait a few seconds for the IoT DevKit to reboot...</h5>";
static const char * result_end = "</div></div></section></body></html>";

extern OLEDDisplay Screen;
extern NetworkInterface *_defaultSystemNetwork;

static int web_settings = 0;
static bool is_http_init;
static bool is_handlers_registered;

bool set_wifi_value(char *value_ssid, char *value_pass)
{
    EEPROMInterface eeprom;
    if (eeprom.saveWiFiSetting(value_ssid, value_pass) == 0)
    {
      return true;
    }
    else
    {
      return false;
    }
}

bool connect_wifi(char *value_ssid, char *value_pass)
{
    Screen.clean();
    Screen.print("WiFi \r\n \r\nConnecting...\r\n \r\n");

    if (_defaultSystemNetwork == NULL)
    {
        _defaultSystemNetwork = new EMW10xxInterface();
    }

    ((EMW10xxInterface*)_defaultSystemNetwork)->set_interface(Station);
    int err = ((EMW10xxInterface*)_defaultSystemNetwork)->connect((char*)value_ssid, (char*)value_pass, NSAPI_SECURITY_WPA_WPA2, 0);
    if (err != 0)
    {
        Screen.print("WiFi \r\n \r\nNo connection \r\n \r\n");
        return false;
    }
    else
    {
        char wifiBuff[128];
        sprintf(wifiBuff, "WiFi \r\n %s\r\n %s \r\n \r\n", value_ssid, _defaultSystemNetwork->get_ip_address());
        Screen.print(wifiBuff);
    }

    return true;
}

bool set_az_iothub(char *value_device_connection_string)
{
    EEPROMInterface eeprom;
    if (eeprom.saveDeviceConnectionString(value_device_connection_string) == 0)
    {
      return true;
    }
    else
    {
      return false;
    }
}

bool set_az_x509(char *value_x509)
{
    EEPROMInterface eeprom;
    if (eeprom.saveX509Cert(value_x509) == 0)
    {
      return true;
    }
    else
    {
      return false;
    }
}

static int web_system_setting_page(httpd_request_t *req)
{
    char *setting_page = NULL;
    int len = 0;
    int err = kNoErr;
    const char *ssid = "";
    int ssidLen = 0;

    // scan network
    WiFiAccessPoint wifiScanResult[50];
    int validWifiIndex[15];
    int wifiCount = ((EMW10xxInterface*)_defaultSystemNetwork)->scan(wifiScanResult, 50);
    int validWifiCount = 0;
    // Scan Wi-Fi AP
    for (int i = 0; i < wifiCount; ++i)
    {
        // too weak
        if (wifiScanResult[i].get_rssi() < -100)
        {
            continue;
        }

        bool shouldSkip = false;
        for (int j = 0; j < i; ++j)
        {
            // this ap has been skipped before
            if (wifiScanResult[j].get_rssi() < -100)
            {
                continue;
            }
            else if (strcmp(wifiScanResult[i].get_ssid(), wifiScanResult[j].get_ssid()) == 0)
            {
                // duplicate ap name
                shouldSkip = true;
                break;
            }
        }

        if (shouldSkip)
        {
            continue;
        }

        ssid = (char *)wifiScanResult[i].get_ssid();
        ssidLen = strlen(ssid);
        if (ssidLen == BOARD_AP_LENGTH && strncmp(ssid, boardAPHeader, strlen(boardAPHeader)) == 0)
        {
            shouldSkip = true;
            for (int j = strlen(boardAPHeader); j < BOARD_AP_LENGTH; ++j)
            {
                if (!isxdigit(ssid[j])) {
                    shouldSkip = false;
                }
            }
            if (shouldSkip)
            {
                continue;
            }
        }

        if (ssidLen && ssidLen <= WIFI_SSID_MAX_LEN)
        {
            validWifiIndex[validWifiCount++] = i;
        }

        if (validWifiCount >= 15)
        {
            break;
        }
    }

    // Prepare the web view
    setting_page = (char*)calloc(DEFAULT_PAGE_SIZE, 1);
    if (setting_page == NULL)
    {
        err = kGeneralErr;
        goto exit;
    }
    strcpy(setting_page, page_head);
    len += strlen(page_head);
    strcpy(&setting_page[len], wifi_setting_a);
    len += strlen(wifi_setting_a);
    for (int i = 0; i < validWifiCount; ++i)
    {
        ssid = (char *)wifiScanResult[validWifiIndex[i]].get_ssid();
        ssidLen = strlen((char *)wifiScanResult[validWifiIndex[i]].get_ssid());
        if (ssidLen && ssidLen <= WIFI_SSID_MAX_LEN)
        {
            int ret = snprintf(&setting_page[len], DEFAULT_PAGE_SIZE - len, "<option value=\"%s\">%s</option>", ssid, ssid);
            len += (ret > 0 ? ret : 0);
        }
    }
    strcpy(&setting_page[len], wifi_setting_b);
    len += strlen(wifi_setting_b);
    if (web_settings & WEB_SETTING_IOT_DEVICE_CONN_STRING)
    {
        // Enable IoT Device Connection string
        strcpy(&setting_page[len], device_conn_setting);
        len += strlen(device_conn_setting);
    }
    if (web_settings & WEB_SETTING_IOT_CERT)
    {
        // Enable X.509 cert
        strcpy(&setting_page[len], cert_setting);
        len += strlen(cert_setting);
    }
    strcat(&setting_page[len], setting_end);
    len += strlen(setting_end) + 1;
    
    err = httpd_send_all_header(req, HTTP_RES_200, len, HTTP_CONTENT_HTML_STR);
    require_noerr(err, exit);

    err = httpd_send_body(req->sock, (const unsigned char*)setting_page, len);
    require_noerr(err, exit);

exit:
    if (setting_page)
    {
        free(setting_page);
    }
    return err;
}

int web_system_setting_result_page(httpd_request_t *req)
{
    OSStatus err = kNoErr;
    bool retry = false;
    int buf_size;
    char *buf = NULL;
    char value_ssid[WIFI_SSID_MAX_LEN + 1];
    char value_pass[WIFI_PWD_MAX_LEN + 1];
    char *value_device_connection_string = NULL;
    char *value_x509 = NULL;
    char *boundary = NULL;
    char *result_page = NULL;
    int len = 0;
    int ret;

    memset(value_ssid, 0, sizeof(value_ssid));
    memset(value_pass, 0, sizeof(value_pass));

    buf_size = 512 + WIFI_SSID_MAX_LEN + WIFI_PWD_MAX_LEN;
    if (web_settings & WEB_SETTING_IOT_DEVICE_CONN_STRING)
    {
        value_device_connection_string = (char*)calloc(AZ_IOT_HUB_MAX_LEN + 1, 1);
        buf_size += AZ_IOT_HUB_MAX_LEN;
    }
    if (web_settings & WEB_SETTING_IOT_CERT)
    {
        value_x509 = (char*)calloc(AZ_IOT_X509_MAX_LEN + 1, 1);
        buf_size += AZ_IOT_X509_MAX_LEN;
    }
    buf = (char *)calloc(buf_size, 1);
    if (buf == NULL)
    {
        err = kGeneralErr;
        goto _exit;
    }

    // Get data
    err = httpd_get_data(req, buf, buf_size - 1);
    require_noerr(err, _exit);
    // Extract settings
    if (strstr(req->content_type, "multipart/form-data") != NULL) // Post data is multipart encoded
    {
        boundary = strstr(req->content_type, "boundary=");
        boundary += 9;

        err = httpd_get_tag_from_multipart_form(buf, boundary, "SSID", value_ssid, WIFI_SSID_MAX_LEN);
        require_noerr(err, _exit);

        err = httpd_get_tag_from_multipart_form(buf, boundary, "PASS", value_pass, WIFI_PWD_MAX_LEN);
        require_noerr(err, _exit);

        if (value_device_connection_string)
        {
            err = httpd_get_tag_from_multipart_form(buf, boundary, "DeviceConnectionString", value_device_connection_string, AZ_IOT_HUB_MAX_LEN);
            require_noerr(err, _exit);
        }

        if (value_x509)
        {
            err = httpd_get_tag_from_multipart_form(buf, boundary, "certificate", value_x509, AZ_IOT_X509_MAX_LEN);
            require_noerr(err, _exit);
        }
    }
    else // Post data is URL encoded
    {
        err = httpd_get_tag_from_post_data(buf, "SSID", value_ssid, WIFI_SSID_MAX_LEN);
        require_noerr(err, _exit);

        err = httpd_get_tag_from_post_data(buf, "PASS", value_pass, WIFI_PWD_MAX_LEN);
        require_noerr(err, _exit);

        if (value_device_connection_string)
        {
            err = httpd_get_tag_from_post_data(buf, "DeviceConnectionString", value_device_connection_string, AZ_IOT_HUB_MAX_LEN);
            require_noerr(err, _exit);
        }

        if (value_x509)
        {
            err = httpd_get_tag_from_post_data(buf, "certificate", value_x509, AZ_IOT_X509_MAX_LEN);
            require_noerr(err, _exit);
        }
    }
    // Free the data buffer
    free(buf);
    buf = NULL;

    // Prepare the result page
    result_page = (char*)calloc(DEFAULT_PAGE_SIZE, 1);
    if (result_page == NULL)
    {
        err = kGeneralErr;
        goto _exit;
    }
    // Head
    strcpy(result_page, page_head);
    len = strlen(page_head);
    // Result body head
    strcpy(&result_page[len], result_head);
    len += strlen(result_head);
    // Wi-Fi setting
    if (value_ssid[0] == 0)
    {
        ret = snprintf(&result_page[len], DEFAULT_PAGE_SIZE - len, result_wifi, "no change");
    }
    else
    {
        if (set_wifi_value(value_ssid, value_pass))
        {
            ret = snprintf(&result_page[len], DEFAULT_PAGE_SIZE - len, result_wifi, "saved");
        }
        else
        {
            ret = snprintf(&result_page[len], DEFAULT_PAGE_SIZE - len, result_wifi, "save failed");
        }
    }
    len += (ret > 0 ? ret : 0);
    
    // IoT Device Connection string
    if (value_device_connection_string)
    {
        if (value_device_connection_string[0] == 0)
        {
            ret = snprintf(&result_page[len], DEFAULT_PAGE_SIZE - len, result_conn_string, "no change");
        }
        else
        {
            if (set_az_iothub(value_device_connection_string))
            {
                ret = snprintf(&result_page[len], DEFAULT_PAGE_SIZE - len, result_conn_string, "saved");
            }
            else
            {
                ret = snprintf(&result_page[len], DEFAULT_PAGE_SIZE - len, result_conn_string, "save failed");
            }
        }
        len += (ret > 0 ? ret : 0);
    }
    // X.509 cert
    if (value_x509)
    {
        if (value_x509[0] == 0)
        {
            ret = snprintf(&result_page[len], DEFAULT_PAGE_SIZE - len, result_cert, "no change");
        }
        else
        {
            if (set_az_x509(value_x509))
            {
                ret = snprintf(&result_page[len], DEFAULT_PAGE_SIZE - len, result_cert, "saved");
            }
            else
            {
                ret = snprintf(&result_page[len], DEFAULT_PAGE_SIZE - len, result_cert, "save failed");
            }
        }
        len += (ret > 0 ? ret : 0);
    }
    strcpy(&result_page[len], result_table_end);
    len += strlen(result_table_end);
    
    // Connect Wi-Fi
    retry = false;
    if (value_ssid[0])
    {
        if (connect_wifi(value_ssid, value_pass))
        {
            ret = snprintf(&result_page[len], DEFAULT_PAGE_SIZE - len, result_wifi_connect_2, value_ssid, _defaultSystemNetwork->get_ip_address());
        }
        else
        {
            ret = snprintf(&result_page[len], DEFAULT_PAGE_SIZE - len, result_wifi_connect_1, value_ssid);
            retry = true;
        }
        len += (ret > 0 ? ret : 0);
    }
    if (retry)
    {
        strcpy(&result_page[len], result_retry);
        len += strlen(result_retry);
    }
    else
    {
        strcpy(&result_page[len], result_wait);
        len += strlen(result_wait);
    }
    strcpy(&result_page[len], result_end);
    len += strlen(result_end) + 1;

    // Show the result page
    err = httpd_send_all_header(req, HTTP_RES_200, len, HTTP_CONTENT_HTML_STR);
    require_noerr(err, _exit);

    err = httpd_send_body(req->sock, (const unsigned char*)result_page, len);
    require_noerr(err, _exit);

_exit:
    if (buf)
    {
        free(buf);
    }
    if (value_device_connection_string)
    {
        free(value_device_connection_string);
    }
    if (result_page)
    {
        free(result_page);
    }

    if (err == 0 && !retry)
    {
        wait_ms(5000);
        mico_system_reboot();
    }
    return err;
}

struct httpd_wsgi_call g_app_handlers[] = {
  {"/", HTTPD_HDR_DEFORT, 0, web_system_setting_page, NULL, NULL, NULL},
  {"/result", HTTPD_HDR_DEFORT, 0, NULL, web_system_setting_result_page, NULL, NULL},
  {"/setting", HTTPD_HDR_DEFORT, 0, web_system_setting_page, NULL, NULL, NULL},
};

int g_app_handlers_no = sizeof(g_app_handlers) / sizeof(struct httpd_wsgi_call);

void app_http_register_handlers()
{
    httpd_register_wsgi_handlers(g_app_handlers, g_app_handlers_no);
}

int _app_httpd_start()
{
    OSStatus err = kNoErr;

    /*Initialize HTTPD*/
    if (is_http_init == false)
    {
        err = httpd_init();
        require_noerr(err, exit);
        is_http_init = true;
    }

    /*Start http thread*/
    err = httpd_start();
    if (err != kNoErr)
    {
        httpd_shutdown();
    }
exit:
    return err;
}

int httpd_server_start(int settings)
{
    int err = kNoErr;
    web_settings = settings;
    err = _app_httpd_start();
    require_noerr(err, exit);

    if (is_handlers_registered == false)
    {
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
    err = httpd_stop();
    require_noerr(err, exit);

exit:
    return err;
}
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __SYSTEM_WEB_H__
#define __SYSTEM_WEB_H__

#include "mbed.h"

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

#define WEB_SETTING_IOT_DEVICE_CONN_STRING  0x00000001
#define WEB_SETTING_IOT_CERT                0x00000002
#define WEB_SETTING_IOT_DPS_SYMMETRIC_KEY   0x00000004

[[deprecated("Replaced by SystemWebAddSettings, which can add new settings in the system setting web page")]]
void EnableSystemWeb(int extFunctions);

void StartupSystemWeb(void);

#define ITEM_INPUT_TEXT         0
#define ITEM_INPUT_PASSWORD     1
#define ITEM_INPUT_RICH_TEXT    2

#define SETTING_RESULT_NO_CHANGE        0
#define SETTING_RESULT_SAVED            1
#define SETTING_RESULT_SAVE_FAILED      2
#define SETTING_RESULT_INVALID_VALUE    3

typedef struct web_page_setting_item_t
{
    const char *name;
    const char *display_name;
    int type;
    const char *placeholder_text;
    const char *default_text;
    char *value_text;
    int value_len;
    int result;
}WEB_PAGE_SETTING_ITEM;

typedef struct web_page_settings_t
{
    const char *name;
    int item_number;
    WEB_PAGE_SETTING_ITEM *items;
    int (*result_fun)(struct web_page_settings_t *settings);
}WEB_PAGE_SETTINGS;

extern WEB_PAGE_SETTINGS az_iot_default_settings;
extern WEB_PAGE_SETTINGS az_iot_x509_settings;
extern WEB_PAGE_SETTINGS az_iot_symmetric_key_settings;

int SystemWebAddSettings(WEB_PAGE_SETTINGS *settings);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __SYSTEM_WEB_H__

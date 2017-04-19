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

#ifndef __SYSTME_WIFI_H__
#define __SYSTME_WIFI_H__

#include "mbed.h"

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

bool InitSystemWiFi(void);
bool SystemWiFiConnect(void);
const char* SystemWiFiSSID(void);
NetworkInterface* WiFiInterface(void);
void SyncTime(void);

bool InitSystemWiFiAP(void);
bool SystemWiFiAPStart(const char *ssid, const char *passphrase);
NetworkInterface* WiFiAPInterface(void);

int WiFiScan(WiFiAccessPoint *res, unsigned count);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __SYSTME_WIFI_H__

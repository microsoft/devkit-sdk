/*(Copyright)

Microsoft Copyright 2017
Confidential Information

*/
#ifndef __IOTHUB_CLIENT_DPS_H
#define __IOTHUB_CLIENT_DPS_H

#include "iothub_client.h"
#include "iothub_client_ll.h"
#include "azure_hub_modules/dps_client.h"

static IOTHUBMESSAGE_DISPOSITION_RESULT receive_msg_callback(IOTHUB_MESSAGE_HANDLE message, void* user_context);
static void dps_registation_status(DPS_REGISTRATION_STATUS reg_status, void* user_context);
static void iothub_connection_status(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context);
static void iothub_dps_register_device(DPS_RESULT register_result, const char* iothub_uri, const char* device_id, void* user_context);
static void on_dps_error_callback(DPS_ERROR error_type, void* user_context);
int IoTHubClientStart();

#endif

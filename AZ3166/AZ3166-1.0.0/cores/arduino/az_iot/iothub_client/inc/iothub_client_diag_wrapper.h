// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/** @file iothub_client_diag_wrapper.h
*	@brief Extends the IoTHubCLient module with additional features.
*
*	@details IoTHubClientDiagWrapper is a module that extends the IoTHubCLient module with diagnostics support:
*				It will send diagnostics properties along with messages per diagnostics sampling configuration.
*/

#ifndef IOTHUB_CLIENT_DIAG_WRAPPER_H_
#define IOTHUB_CLIENT_DIAG_WRAPPER_H_

#include "iothub_client.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum {
		/*diagnostics sampling is off, i.e. do not send diagnostics info at all*/
		SAMPLING_NONE,

		/*diagnostics sampling is on, the sampling percentage is provided by client*/
		SAMPLING_CLIENT,

		/*diagnostics sampling switch and percentage are provided by device's configuration at server end*/
		SAMPLING_SERVER
	} SAMPLING_RATE_SOURCE;
	
	/**
    * @brief	Wrapper of IoTHubClient_LL_CreateFromConnectionString with diagnostics support.
    *
    * @param	samplingSource				Diagnostics sampling source
    * @param	clientSamplingPercentage	Specifying sampling percentage when samplingSource is SAMPLING_CLIENT, value range [0,100].
											The value is meaningless when samplingSource is NOT SAMPLING_CLIENT
    * @param	connectionString	Pointer to a character string
    * @param	protocol			Function pointer for protocol implementation

	* @return	A non-NULL @c IOTHUB_CLIENT_HANDLE value that is used when
    * 			invoking other functions for IoT Hub client and @c NULL on failure.
    */
	IOTHUB_CLIENT_LL_HANDLE IoTHubClient_LL_CreateFromConnectionString_WithDiagnostics(SAMPLING_RATE_SOURCE samplingSource, int clientSamplingPercentage, const char* deviceConnectionString, IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol);
	
	/**
	* @brief	Wrapper of IoTHubClient_LL_SetDeviceTwinCallback with diagnostics support.
	*
	* @param	iotHubClientHandle		The handle created by a call to the create function.
	* @param	deviceTwinCallback	    The callback specified by the device client to be used for updating
	*									the desired state. The callback will be called in response to patch
	*									request send by the IoTHub services. The payload will be passed to the
	*									callback, along with two version numbers:
	*										- Desired:
	*										- LastSeenReported:
	* @param	userContextCallback		User specified context that will be provided to the
	* 									callback. This can be @c NULL.
	*
	*			@b NOTE: The application behavior is undefined if the user calls
	*			the ::IoTHubClient_LL_Destroy function from within any callback.
	*
	* @return	IOTHUB_CLIENT_OK upon success or an error code upon failure.
	*/
	IOTHUB_CLIENT_RESULT IoTHubClient_LL_SetDeviceTwinCallback_WithDiagnostics(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, IOTHUB_CLIENT_DEVICE_TWIN_CALLBACK iotHubUserdeviceTwinCallback, void* userContextCallback);


	/**
	* @brief	Wrapper of IoTHubClient_LL_SendEventAsync with diagnostics support.
	*
	* @param	iotHubClientHandle		   	The handle created by a call to the create function.
	* @param	eventMessageHandle		   	The handle to an IoT Hub message.
	* @param	eventConfirmationCallback  	The callback specified by the device for receiving
	* 										confirmation of the delivery of the IoT Hub message.
	* 										This callback can be expected to invoke the
	* 										::IoTHubClient_SendEventAsync function for the
	* 										same message in an attempt to retry sending a failing
	* 										message. The user can specify a @c NULL value here to
	* 										indicate that no callback is required.
	* @param	userContextCallback			User specified context that will be provided to the
	* 										callback. This can be @c NULL.
	*
	*
	* @return	IOTHUB_CLIENT_OK upon success or an error code upon failure.
	*/
	IOTHUB_CLIENT_RESULT IoTHubClient_LL_SendEventAsync_WithDiagnostics(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, IOTHUB_MESSAGE_HANDLE eventMessageHandle, IOTHUB_CLIENT_EVENT_CONFIRMATION_CALLBACK eventConfirmationCallback, void* userContextCallback);

#ifdef __cplusplus
}
#endif

#endif /*IOTHUB_CLIENT_DIAG_WRAPPER_H_*/
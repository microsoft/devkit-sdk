// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "iothub_client_ll.h"
#include "iothubtransportmqtt.h"
#include "iothub_client_diag_wrapper.h"
#include "json.h"
#include "StringUtils.h"
#include "azure_c_shared_utility/uniqueid.h"

#define MAX_TOKENS_LEN 100
#define BASE_36 36

static SAMPLING_RATE_SOURCE samplingRateSource;
static bool isServerDiagnosticEnabled;
static int diagSamplingRate;
static long sequenceNumberOfMessage;
static char base36Map[BASE_36]; //0..10 a..z
static bool isDiagnosticInitialized;
static IOTHUB_CLIENT_DEVICE_TWIN_CALLBACK userDeviceTwinCallback;
static const char* diagEnableProperty = "diag_enable";
static const char* diagSampleRateProperty = "diag_sample_rate";

static char* GenerateRandomEightCharacters(char *randomString)
{
	char* randomStringPos = randomString;
	for (int i = 0; i < 4; ++i)
	{
		int rawRandom = rand();
		int first = rawRandom % BASE_36;
		int second = rawRandom / BASE_36 % BASE_36;
		*randomStringPos++ = base36Map[first];
		*randomStringPos++ = base36Map[second];
	}
	*randomStringPos = 0;

	return randomString;
}

static char* GetCurrentTimeUtc(char* timeBuffer, int bufferLen, const char* formatString)
{
	time_t epochTime;
	struct tm* utcTime;
	epochTime = time(NULL);
	utcTime = gmtime(&epochTime);
	strftime(timeBuffer, bufferLen, formatString, utcTime);
	return timeBuffer;
}

static void AddDiagnosticsPropertiesIfNecessary(IOTHUB_MESSAGE_HANDLE eventMessageHandle)
{
	if (samplingRateSource == SAMPLING_NONE || diagSamplingRate == 0 || 
		(samplingRateSource == SAMPLING_SERVER && !isServerDiagnosticEnabled))
		return;

	if (floor((sequenceNumberOfMessage - 2) * diagSamplingRate / 100.0) < floor((sequenceNumberOfMessage - 1) * diagSamplingRate / 100.0))
	{
		char randomId[37];
		char timeBuffer[21];
		MAP_HANDLE propMap = IoTHubMessage_Properties(eventMessageHandle);

		if(!(UniqueId_Generate(randomId, 37) == UNIQUEID_OK &&
			Map_Add(propMap, "x-correlation-id", randomId) == MAP_OK))
		{
			LogInfo("Fail to add diagnostic property: x-correlation-id");
		}
		if (Map_Add(propMap, "x-before-send-request", GetCurrentTimeUtc(timeBuffer, 21, "%Y-%m-%dT%H:%M:%SZ")) != MAP_OK)
		{
			LogInfo("Fail to add diagnostic property: x-before-send-request");
		}
	}
	++sequenceNumberOfMessage;
}

static void DiagnosticsDeviceTwinCallBack(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* deviceTwinJson, size_t size, void* userContextCallback)
{
	if(samplingRateSource == SAMPLING_SERVER)
	{
		const char *payLoad, *enableString, *rateString;
		json_object *twinObject, *desiredObject, *diagEnableObject, *rateObject;

		payLoad = (const char*)deviceTwinJson;
		if(payLoad != NULL && (twinObject = json_tokener_parse(payLoad)) != NULL)
		{
			if((desiredObject = json_object_object_get(twinObject, "desired")) != NULL)
			{
				if((diagEnableObject = json_object_object_get(desiredObject, diagEnableProperty)) != NULL)
				{
					enableString = json_object_get_string(diagEnableObject);
					if(enableString != NULL && strnicmp(enableString, "true", 4) == 0)
					{
						isServerDiagnosticEnabled = true;
					}
				}

				if((rateObject = json_object_object_get(desiredObject, diagSampleRateProperty)) != NULL)
				{
					if((rateString = json_object_get_string(rateObject)) != NULL)
					{
						int sampleRate = atoi(rateString);
						if (sampleRate < 0 || sampleRate > 100)
						{
							sampleRate = 0;
						}
						diagSamplingRate = sampleRate;
					}
				}
			}
		}
	}
	
	if (userDeviceTwinCallback != NULL)
	{
		userDeviceTwinCallback(update_state, deviceTwinJson, size, userContextCallback);
	}
}

IOTHUB_CLIENT_RESULT IoTHubClient_LL_SetDeviceTwinCallback_WithDiagnostics(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, IOTHUB_CLIENT_DEVICE_TWIN_CALLBACK iotHubUserdeviceTwinCallback, void* userContextCallback)
{
	IOTHUB_CLIENT_RESULT result;

	if (!isDiagnosticInitialized)
	{
		LogError("You must call IoTHubClient_LL_CreateFromConnectionString_WithDiagnostics first.");
		result = IOTHUB_CLIENT_ERROR;
	}
	else if (samplingRateSource == SAMPLING_SERVER)
	{
		/* Codes_SRS_IOTHUBCLIENT_LL_10_001: [ IoTHubClient_LL_SetDeviceTwinCallback shall fail and return IOTHUB_CLIENT_INVALID_ARG if parameter iotHubClientHandle is NULL.] */
		if (iotHubClientHandle == NULL)
		{
			result = IOTHUB_CLIENT_INVALID_ARG;
			LogError("Invalid argument specified iothubClientHandle=%p", iotHubClientHandle);
		}
		userDeviceTwinCallback = iotHubUserdeviceTwinCallback;
		result = IOTHUB_CLIENT_OK;
	}
	else
	{
		result = IoTHubClient_LL_SetDeviceTwinCallback(iotHubClientHandle, iotHubUserdeviceTwinCallback, userContextCallback);
	}
	return result;
}

IOTHUB_CLIENT_LL_HANDLE IoTHubClient_LL_CreateFromConnectionString_WithDiagnostics(SAMPLING_RATE_SOURCE samplingSource, int clientSamplingPercentage, const char* deviceConnectionString, IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol)
{
	if (protocol != MQTT_Protocol && samplingSource == SAMPLING_SERVER)
	{
		LogError("The protocol must be MQTT when using SAMPLING_SERVER");
		return NULL;
	}
	if (samplingSource == SAMPLING_CLIENT && (clientSamplingPercentage < 0 || clientSamplingPercentage > 100))
	{
		LogError("The value of clientSamplingPercentage must be between [0-100].");
		return NULL;
	}

	//Initialize for diagnostics
	srand((int)time(NULL));
	for (int i = 0; i < BASE_36; ++i)
	{
		base36Map[i] = i < 10 ? (char)('0' + i) : (char)('a' + (i - 10));
	}
	samplingRateSource = samplingSource;
	diagSamplingRate = samplingSource == SAMPLING_SERVER ? 0 : clientSamplingPercentage;
	isDiagnosticInitialized = true;
	userDeviceTwinCallback = NULL;

	IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(deviceConnectionString, protocol);
	if (iotHubClientHandle != NULL)
	{
		IoTHubClient_LL_SetDeviceTwinCallback(iotHubClientHandle, DiagnosticsDeviceTwinCallBack, NULL);
	}
	return iotHubClientHandle;
}

IOTHUB_CLIENT_RESULT IoTHubClient_LL_SendEventAsync_WithDiagnostics(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, IOTHUB_MESSAGE_HANDLE eventMessageHandle, IOTHUB_CLIENT_EVENT_CONFIRMATION_CALLBACK eventConfirmationCallback, void* userContextCallback)
{
	if (!isDiagnosticInitialized)
	{
		LogError("You must call IoTHubClient_LL_CreateFromConnectionString_WithDiagnostics first.");
		return IOTHUB_CLIENT_ERROR;
	}
	AddDiagnosticsPropertiesIfNecessary(eventMessageHandle);
	return IoTHubClient_LL_SendEventAsync(iotHubClientHandle, eventMessageHandle, eventConfirmationCallback, userContextCallback);
}

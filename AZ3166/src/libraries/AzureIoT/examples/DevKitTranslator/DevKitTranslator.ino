// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
// To get started please visit https://microsoft.github.io/azure-iot-developer-kit/docs/projects/devkit-translator/?utm_source=ArduinoExtension&utm_medium=ReleaseNote&utm_campaign=VSCode
#include "AudioClassV2.h"
#include "AZ3166WiFi.h"
#include "AzureIotHub.h"
#include "DevKitMQTTClient.h"
#include "OLEDDisplay.h"
#include "SystemTickCounter.h"

#include "azure_config.h"
#include "http_client.h"

#define LANGUAGES_COUNT     9
#define MAX_RECORD_DURATION 2
#define MAX_UPLOAD_SIZE     (64 * 1024)
#define LOOP_DELAY          100
#define PULL_TIMEOUT        10000
#define AUDIO_BUFFER_SIZE   ((32000 * MAX_RECORD_DURATION) - 16000 + 44 + 1)

#define APP_VERSION         "ver=1.0"
#define ERROR_INFO          "Sorry, I can't \r\nhear you."

enum STATUS
{
  Idle,
  Recording,
  Recorded,
  WavReady,
  Uploaded,
  SelectLanguage
};

static int wavFileSize;
static char *waveFile = NULL;
static char azureFunctionUri[128];

// The timeout for retrieving the result
static uint64_t result_timeout_ms;

// Audio instance
static AudioClass& Audio = AudioClass::getInstance();

// Language
static int currentLanguage = 1; // Default is Chinese
static const char* allLanguages[LANGUAGES_COUNT] = {"Arabic", "Chinese", "French", "German", "Italian", "Japanese", "Portuguese", "Russian", "Spanish"};

// Indicate the processing status
static STATUS status = Idle;

// Indicate whether WiFi is ready
static bool hasWifi = false;

// Indicate whether IoT Hub is ready
static bool hasIoTHub = false;


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities
static void InitWiFi()
{
  Screen.print(2, "Connecting...");
  
  if (WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    Screen.print(1, ip.get_address());
    hasWifi = true;
    Screen.print(2, "Running... \r\n");
  }
  else
  {
    hasWifi = false;
    Screen.print(1, "No Wi-Fi\r\n ");
  }
}

static void EnterIdleState(bool clean = true)
{
  status = Idle;
  if (clean)
  {
    Screen.clean();
  }
  Screen.print(0, "Hold B to talk");
}

static int HttpTriggerTranslator(const char *content, int length)
{
  if (content == NULL || length <= 0 || length > MAX_UPLOAD_SIZE)
  {
    Serial.println("Content not valid");
    return -1;
  }
  HTTPClient client = HTTPClient(HTTP_POST, azureFunctionUri);
  client.set_header("source", allLanguages[currentLanguage]);
  const Http_Response *response = client.send(content, length);
  
  if (response != NULL && response->status_code == 200)
  {
    return 0;
  }
  return -1;
}

static void ShowLanguages()
{
  char temp[20];

  int idx = (currentLanguage + LANGUAGES_COUNT  -  1) % LANGUAGES_COUNT;
  sprintf(temp, "  %s", allLanguages[idx]);
  Screen.print(1, temp);
  
  sprintf(temp, "> %s", allLanguages[(idx + 1) % LANGUAGES_COUNT]);
  Screen.print(2, temp);
  
  sprintf(temp, "  %s", allLanguages[(idx + 2) % LANGUAGES_COUNT]);
  Screen.print(3, temp);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callback functions
static void ResultMessageCallback(const char *text, int length)
{
  if (status != Uploaded)
  {
      return;
  }

  EnterIdleState();
  if (text == NULL)
  {
    Screen.print(1, ERROR_INFO);
    return;
  }
  
  char temp[33];
  int end = min(length, sizeof(temp) - 1);
  memcpy(temp, text, end);
  temp[end] = '\0';
  Screen.print(1, "Translation: ");
  Screen.print(2, temp, true);
  LogTrace("DevKitTranslatorSucceed", APP_VERSION);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Actions
static void DoIdle()
{
  if (digitalRead(USER_BUTTON_A) == LOW)
  {
    // Enter Select Language mode
    status = SelectLanguage;
    Screen.clean();
    Screen.print(0, "Press B Scroll");
    ShowLanguages();
  }
  else if (digitalRead(USER_BUTTON_B) == LOW)
  {
    // Enter the Recording mode
    Screen.clean();
    Screen.print(0, "Recording...");
    Screen.print(1, "Release to send\r\nMax duraion: \r\n1.5 sec");
    memset(waveFile, 0, AUDIO_BUFFER_SIZE);
    Audio.format(8000, 16);
    Audio.startRecord(waveFile, AUDIO_BUFFER_SIZE);
    status = Recording;
  }

  DevKitMQTTClient_Check();
}

static void DoRecording()
{
    if (digitalRead(USER_BUTTON_B) == HIGH)
    {
      Audio.stop();
      status = Recorded;
    }
}

static void DoRecorded()
{
  wavFileSize = Audio.getCurrentSize();
  if (wavFileSize > 0)
  {
    wavFileSize = Audio.convertToMono(waveFile, wavFileSize, 16);
    if (wavFileSize <= 0)
    {
      Serial.println("ConvertToMono failed! ");
      EnterIdleState();
    }
    else
    {
      status = WavReady;
      Screen.clean();
      Screen.print(0, "Processing...");
      Screen.print(1, "Uploading...");
    }
  }
  else
  {
    Serial.println("No Data Recorded! ");
    EnterIdleState();
  }
}

static void DoWavReady()
{
  if (wavFileSize <= 0)
  {
    EnterIdleState();
    return;
  }
  
  if (HttpTriggerTranslator(waveFile, wavFileSize) == 0)
  {
    status = Uploaded;
    Screen.print(1, "Receiving...");
    // Start retrieving result timeout clock
    result_timeout_ms = SystemTickCounterRead();
  }
  else
  {
    Serial.println("Error happened when translating: Azure Function failed");
    EnterIdleState();
    Screen.print(1, "Sorry, I can't \r\nhear you.");
  }
}

static void DoUploader()
{
  DevKitMQTTClient_ReceiveEvent();
  
  if ((int)(SystemTickCounterRead() - result_timeout_ms) >= PULL_TIMEOUT)
  {
    // Timeout
    EnterIdleState();
    Screen.print(1, ERROR_INFO);
  }  
}

static void DoSelectLanguage()
{
  if (digitalRead(USER_BUTTON_B) == LOW)
  {
    currentLanguage = (currentLanguage + 1) % LANGUAGES_COUNT;
    ShowLanguages();
  }
  else if (digitalRead(USER_BUTTON_A) == LOW)
  {
    EnterIdleState();
  }
  DevKitMQTTClient_Check();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino sketch
void setup()
{
  Screen.init();
  Screen.print(0, "DevKitTranslator");

  if (strlen(AZURE_FUNCTION_APP_NAME) == 0)
  {
    Screen.print(2, "No Azure Func");
    return;
  }

  Screen.print(2, "Initializing...");
  pinMode(USER_BUTTON_A, INPUT);
  pinMode(USER_BUTTON_B, INPUT);

  Screen.print(3, " > Serial");
  Serial.begin(115200);
  
  // Initialize the WiFi module
  Screen.print(3, " > WiFi");
  hasWifi = false;
  InitWiFi();
  if (!hasWifi)
  {
    return;
  }
  LogTrace("DevKitTranslatorSetup", APP_VERSION);

  // IoT hub
  Screen.print(3, " > IoT Hub");
  DevKitMQTTClient_SetOption(OPTION_MINI_SOLUTION_NAME, "DevKitTranslator");
  if (!DevKitMQTTClient_Init())
  {
    Screen.clean();
    Screen.print(0, "DevKitTranslator");
    Screen.print(2, "No IoT Hub");
    hasIoTHub = false;
    return;
  }
  hasIoTHub = true;
  DevKitMQTTClient_SetMessageCallback(ResultMessageCallback);

  // Audio
  Screen.print(3, " > Audio");
  waveFile = (char *)malloc(AUDIO_BUFFER_SIZE);
  if (waveFile == NULL)
  {
    Screen.print(3, "Audio init fails");
    Serial.println("No enough Memory!");
    return;
  }
    
  sprintf(azureFunctionUri, "http://%s.azurewebsites.net/api/devkit-translator", (char *)AZURE_FUNCTION_APP_NAME);
  Screen.print(1, "Talk:   Hold  B\r\n \r\nSetting:Press A");
}

void loop()
{
  if (hasWifi && hasIoTHub)
  {
    switch (status)
    {
      case Idle:
        DoIdle();
        break;

      case Recording:
        DoRecording();
        break;

      case Recorded:
        DoRecorded();
        break;

      case WavReady:
        DoWavReady();
        break;

      case Uploaded:
        DoUploader();
        break;

      case SelectLanguage:
        DoSelectLanguage();
        break;
    }
  }
  delay(LOOP_DELAY);
}
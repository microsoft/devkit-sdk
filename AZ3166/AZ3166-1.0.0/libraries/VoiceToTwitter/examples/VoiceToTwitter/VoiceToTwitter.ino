#include "Arduino.h"
#include "RGB_LED.h"
#include "iot_client.h"
#include "AZ3166WiFi.h"
#include "EEPROMInterface.h"
#include "OLEDDisplay.h"
#include "http_client.h"
#include "mbed_memory_status.h"
#include "json.h"
#include "AudioClass.h"

#define RGB_LED_BRIGHTNESS  16
static boolean hasWifi;
static int status = 0; // idle
static const int recordedDuration = 3;
static const int AUDIO_SIZE = 32044 * recordedDuration;
static char *waveFile = NULL;
static int wavFileSize;
static int timeout = 0;
static int step2Result = -1;
static RGB_LED rgbLed;
const char *_json_object_get_string(json_object *obj, const char *name);
static AudioClass Audio;

static void InitWiFi()
{
    if (WiFi.begin() == WL_CONNECTED)
    {
        hasWifi = true;
    }
    else
    {
        Screen.print(1, "No Wi-Fi           ");
    }
}

static void InitBoard(void)
{
    Screen.clean();
    Screen.print(0, "Azure IoT DevKit     ");
    Screen.print(2, "Initializing...      ");

    // Initialize the WiFi module
    Screen.print(3, " > WiFi              ");
    hasWifi = false;
    rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
    InitWiFi();
    rgbLed.setColor(0, 0, 0);
    enterIdleState();
}

static void enterIdleState()
{
    status = 0;
    Screen.clean();
    Screen.print(0, "Hold B to talk   ");
    Serial.println("Hold B to talk   ");
    rgbLed.setColor(0, 0, 0);
}

static void enterRecordState()
{
    status = 1;
    Screen.clean();
    Screen.print(0, "Release B to send\r\nMax duraion: 3 sec");
    Serial.println("Release B to send    ");
    rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
}

static void enterUploading1State()
{
    status = 2;
    Screen.clean();
    Screen.print(0, "Processing...          ");
    Screen.print(1, "Uploading #1", true);
    rgbLed.setColor(0, RGB_LED_BRIGHTNESS, 0);
}

static void enterUploading2State(int size)
{
    status = 3;
    Screen.clean();
    Screen.print(0, "Processing...          ");
    char buf[30];
    sprintf(buf, "Uploading #2: size %d", size);
    Screen.print(1, buf, true);
    rgbLed.setColor(0, 0, RGB_LED_BRIGHTNESS);
}

static void enterUploading3State()
{
    status = 4;
    Screen.clean();
    Screen.print(0, "Processing...          ");
    Screen.print(1, "Uploading #3", true);

    rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, RGB_LED_BRIGHTNESS);
}

static void enterReceivingState()
{
    status = 5;
    Screen.clean();
    Screen.print(0, "Processing...          ");

    Screen.print(1, "Receiving...", true);
    rgbLed.setColor(RGB_LED_BRIGHTNESS, RGB_LED_BRIGHTNESS, RGB_LED_BRIGHTNESS);
}

static void enterShowMessageState(const char *message)
{
    Screen.clean();
    Screen.print(0, "Message about to send:           ");
    Screen.print(1, message, true);

    rgbLed.setColor(0, 0, 0);

}

static void enterShowErrorMessageState(const char *message)
{
    Screen.clean();
    Screen.print(0, "Failed to process the voice!          ");
    Screen.print(1, message, true);
    rgbLed.setColor(0, 0, 0);
}

void setup()
{
    Serial.begin(115200);
    InitBoard();
    EEPROMInterface eeprom;
    uint8_t connString[AZ_IOT_HUB_MAX_LEN + 1] = {'\0'};
    int ret = eeprom.read(connString, AZ_IOT_HUB_MAX_LEN, 0x00, AZ_IOT_HUB_ZONE_IDX);
    if (ret < 0)
    {
        (void)Serial.println("ERROR: Unable to get the azure iot connection string from EEPROM. Please set the value in configuration mode.\r\n");
        return;
    }
    iot_client_set_connection_string((const char *)connString);
}

void freeWavFile()
{
    if (waveFile != NULL)
    {
        free(waveFile);
        waveFile = NULL;
    }
}

void loop()
{
    Serial.println("Enter loop.");

    uint32_t delayTimes = 600;
    uint32_t curr = millis();
    if (status == 0)
    {
        int buttonB = digitalRead(USER_BUTTON_B);
        if (buttonB == LOW)
        {
            waveFile = (char *)malloc(AUDIO_SIZE + 1);
            if (waveFile == NULL)
            {
                Serial.println("Not enough Memory! ");
                enterIdleState();
                return;
            }
            memset(waveFile, 0, AUDIO_SIZE + 1);
            Audio.format(8000, 16);
            Audio.startRecord(waveFile, AUDIO_SIZE, recordedDuration);
            enterRecordState();
        }
    }
    else if (status == 1)
    {
        int buttonB = digitalRead(USER_BUTTON_B);
        if (buttonB == HIGH)
        {
            Audio.getWav(&wavFileSize);
            if (wavFileSize > 0)
            {
                wavFileSize = Audio.convertToMono(waveFile, wavFileSize, 16);
                if (wavFileSize <= 0)
                {
                    Serial.println("ConvertToMono failed! ");
                    enterIdleState();
                    freeWavFile();
                    return;
                }
                else
                {
                    enterUploading1State();
                }
            }
            else
            {
                Serial.println("No Data Recorded! ");
                freeWavFile();
                enterIdleState();
                return;
            }
        }
    }
    else if (status == 2)
    {
        if (wavFileSize > 0 && waveFile != NULL)
        {
            if (0 == iot_client_blob_upload_step1("test.wav"))
            {
                enterUploading2State(wavFileSize);
            }
            else
            {
                Serial.println("Upload(1) Failure");
                freeWavFile();
                enterIdleState();
            }
        }
        else
        {
            freeWavFile();
            enterIdleState();
        }
    }
    else if (status == 3)
    {
        char buf[30];
        sprintf(buf, "Uploading size %d          ", wavFileSize);
        Serial.println(buf);
        step2Result = iot_client_blob_upload_step2(waveFile, wavFileSize);

        enterUploading3State();
    }
    else if (status == 4)
    {
        // Screen.print(3, "Send notification...             ");
        if (iot_client_blob_upload_step3(step2Result == 0) == 0)
        {
            enterReceivingState();
        }
        else
        {
            freeWavFile();
            enterIdleState();
        }
    }
    else if (status == 5)
    {
        const char *p = iot_client_get_c2d_message();
        while (p != NULL)
        {
            if (strlen(p) == 0)
            {
                free((void *)p);
                break;
            }
            Serial.println(p);
            if (strlen(p) > 0 && p[0] == '{')
            {
                json_object *jsonObject = json_tokener_parse(p);
                if (jsonObject != NULL)
                {
                    const char *jsonText = _json_object_get_string(jsonObject, "text");
                    if (jsonText != NULL && strlen(jsonText) > 0)
                    {
                        Serial.println("Got messsage");
                        Serial.println(jsonText);
                        enterShowMessageState(jsonText);
                    }
                    else
                    {
                        const char *jsonError = _json_object_get_string(jsonObject, "error");
                        Serial.print("Got error ");
                        Serial.println(jsonError);
                        enterShowErrorMessageState(jsonError);
                    }
                    // delay to let user read this message
                    delay(5000);
                    json_object_put(jsonObject);
                }
            }
            free((void *)p);
            p = iot_client_get_c2d_message();
        }
        freeWavFile();
        enterIdleState();
    }

    curr = millis() - curr;
    if (curr < delayTimes)
    {
        delay(delayTimes - curr);
    }
}
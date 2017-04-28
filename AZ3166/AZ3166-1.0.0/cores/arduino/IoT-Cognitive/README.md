# Speech Recognition Library

Speech recognition Library provides voice interaction to let you control your device with your voice.

Currently our speech library includes:
- *Microphone APIs*: provide API to get WAVE file of the input audio.
- *Speech recognition APIs*: leverage [Microsoft Cognitive Service](https://www.microsoft.com/cognitive-services/en-us/Speech-api/documentation/overview) to convert words spoken by the user into text for form input, for text dictation, to specify an action or command, and to accomplish tasks. 

This library is still being developed, and more cognitive APIs will be supported in the near future:
- *Text-To-Speech API*: uses a speech synthesis engine (voice) to convert a text string into spoken words. The input string can be either basic, unadorned text or more complex Speech Synthesis Markup Language (SSML).
- *Speaker Identification API*: automatically identify the person speaking in an audio file. The audio of the unknown speaker, together with the prospective group of speakers, is provided during recognition. The input voice is compared against all speakers in order to determine whose voice it is, and if there is a match found, the identity of the speaker is returned.

## Prerequisite
As we leverage the Microsoft Cognitive Service to perform the speech recognition. In order to use this library, you need to register for [Microsoft Cognitive Service](https://www.microsoft.com/cognitive-services/) and get an subscription key for the Bing Speech API and Speaker Recognition API.

![Cognitive Service Subscription Key](https://doliotstorage.blob.core.windows.net/public-resources/cognitiveSubKey.JPG)

## API Definition
- **recognizeSpeech** API
```cpp
SpeechResponse* recognizeSpeech(char * audioFileBinary, int length); 

typedef struct
{
    char * status;  //success or error
    char * text;
    double confidence;
}SpeechResponse;
```

## Sample Usage:
This Sample code can be run on STM32 Nucleo_F412ZG board with SPWF01SA wifi module.

``` cpp
Microphone microphone(A0, 2, 8000);
Serial pc(USBTX, USBRX, 115200);
SPWFSAInterface5 spwf(D1, D0, false);

const char* ssid = "[your wifi ssid]";
const char* pwd = "[your wifi password]";
const char* subscriptionKey = "[your subscripption key to use Microsoft Cognitive Service]";
const char* deviceId = "[guid to use as your device identification]";

int main(void)
{
    // set uo network connection
    printf("Start...\n");
    while(true) {
        printf("Try to connect %s ...\n", ssid);
        if(spwf.connect(ssid, pwd, NSAPI_SECURITY_WPA2) == NSAPI_ERROR_OK) {
            printf("connected %s\r\n", spwf.get_mac_address());
            break;
        }
        else {
            printf("failed.\r\n");
        }
    }

    SpeechInterface * speechInterface = new SpeechInterface(&spwf, subscriptionKey, deviceId, true);
    
    int audio_size;
    char * audio_file;
    DigitalIn button(USER_BUTTON);
    int old_pb = 0, new_pb;
    while(1) {
        new_pb = button.read();
        if(new_pb == 0 && old_pb == 1) {
            printf("start record...\r\n");
            audio_file = microphone.getWav(&audio_size);

            SpeechResponse* speechResponse = speechInterface->recognizeSpeech(audio_file, audio_size);
        }
        old_pb = new_pb;
    }

    free(audio_file);
    return 0;
}
```
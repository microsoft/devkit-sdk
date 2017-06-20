#include "Arduino.h"
#include "OLEDDisplay.h"
#include "AudioClass.h"

AudioClass Audio;
const int AUDIO_SIZE = 32000*3 + 45;

int lastButtonState;
int buttonState;
char * waveFile;

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  Serial.println("Helloworld in Azure IoT DevKits!");

  // initialize the button pin as a input
  pinMode(USER_BUTTON_A, INPUT);
  lastButtonState = digitalRead(USER_BUTTON_A);

  // Setup your local audio buffer
  waveFile = (char *)malloc(AUDIO_SIZE + 1);
  memset(waveFile, 0x0, AUDIO_SIZE);
}

void loop(void) {
  Serial.println("Press button A to start record 3 secs audio");
  Screen.clean();
  Screen.print(0, "AZ3166 Audio:  ");
  Screen.print(1, "Press button A  to start record 3 secs audio", true);

  while (1) {
    buttonState = digitalRead(USER_BUTTON_A);
    if(buttonState == LOW && lastButtonState == HIGH)
    {
      record();
    }
  
    lastButtonState = buttonState;
  }
  
  delay(100);
}

void record() {
  // Re-config the audio data format
  Audio.format(8000, 16);

  Serial.println("start recording");
  
  Screen.clean();
  Screen.print(0, "AZ3166 Audio:  ");
  Screen.print(1, "Start recording");

  // Start to record audio data
  Audio.startRecord(waveFile, AUDIO_SIZE, 3);
  
  while (1) {
    // Check whether the audio record is completed.
    if (Audio.recordComplete() == true) {
      Screen.clean();
      Screen.print(0, "AZ3166 Audio:  ");
      Screen.print(1, "Finish recording");
      int totalSize;
      Audio.getWav(&totalSize);
      Serial.print("Recorded size: ");
      Serial.println(totalSize);

      int monoSize = Audio.convertToMono(waveFile, totalSize, 16);
      Serial.print("To Mono size: " );
      Serial.println(monoSize);
      delay(500);

      Screen.clean();
      Screen.print(0, "AZ3166 Audio:  ");
      Screen.print(1, "Press button A  to start record 3 secs audio", true);
      break;
    }
  }
}


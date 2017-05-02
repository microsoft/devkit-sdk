#include "Arduino.h"
#include "OLEDDisplay.h"

AudioClass Audio;
const int AUDIO_SIZE = 32000*2 + 45;

int lastButtonState;
int buttonState;
char * waveFile;

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
 
  Serial.println("Helloworld in AzureDevKits!");
  
  // initialize the button pin as a input
  pinMode(USER_BUTTON_A, INPUT);
  lastButtonState = digitalRead(USER_BUTTON_A);

  // Setup your local audio buffer
  waveFile = (char *)malloc(AUDIO_SIZE + 1);
  memset(waveFile, 0x0, AUDIO_SIZE);
}

void loop(void) {
  Serial.println("Press user button A to start recording.");

  while (1) {
    buttonState = digitalRead(USER_BUTTON_A);
    if(buttonState == LOW && lastButtonState == HIGH)
    {
      record();
    }
  
    lastButtonState = buttonState;
  }
  
  delay(1000);
}

void record() {
  // Re-config the audio data format
  Audio.format(8000, 16);
  Serial.println("start recording.");
  Screen.print(0, "Start recording   ");

  // Start to record audio data
  Audio.startRecord(waveFile, AUDIO_SIZE, 2);
  
  while (1) {
    // Check whether the audio record is completed.
    if (Audio.recordComplete() == true) {
      Screen.print(0, "Finish recording   ");
      int totalSize;
      Audio.getWav(&totalSize);
      Serial.print("Total size: ");
      Serial.println(totalSize);

      int monoSize = Audio.convertToMono(waveFile, totalSize, 16);
      Serial.print("Mono size:" );
      Serial.println(monoSize);
      delay(100);

      /*
      // Print the binary of the wave data
      int16_t *p = (int16_t *)waveFile;

      for (int i = 0; i < monoSize / 2; i++) {
        if (i % 40 == 0) printf("\n");
        printf("%d,", *p);
        //Serial.print(*p, DEC);
        p++;
      }
      */
      
      break;
    }
  }
}


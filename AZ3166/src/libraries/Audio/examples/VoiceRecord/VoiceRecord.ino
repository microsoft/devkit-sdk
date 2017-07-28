#include "Arduino.h"
#include "OledDisplay.h"
#include "AudioClass.h"

AudioClass& Audio = AudioClass::getInstance();
const int AUDIO_SIZE = 32000 * 3 + 45;

int lastButtonAState;
int buttonAState;
int lastButtonBState;
int buttonBState;
char *waveFile;
int totalSize;
int monoSize;

void setup(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  Serial.println("Helloworld in Azure IoT DevKits!");

  // initialize the button pin as a input
  pinMode(USER_BUTTON_A, INPUT);
  lastButtonAState = digitalRead(USER_BUTTON_A);
  pinMode(USER_BUTTON_B, INPUT);
  lastButtonBState = digitalRead(USER_BUTTON_B);

  // Setup your local audio buffer
  waveFile = (char *)malloc(AUDIO_SIZE + 1);
  memset(waveFile, 0x0, AUDIO_SIZE);
}

void loop(void)
{
  printIdleMessage();

  while (1)
  {
    buttonAState = digitalRead(USER_BUTTON_A);
    buttonBState = digitalRead(USER_BUTTON_B);
    
    if (buttonAState == LOW && lastButtonAState == HIGH)
    {
      record();
    }

    if (buttonBState == LOW && lastButtonBState == HIGH)
    {
      play();
    }
    
    lastButtonAState = buttonAState;
    lastButtonBState = buttonBState;
  }

  delay(100);
}

void printIdleMessage()
{
  Screen.clean();
  Screen.print(0, "AZ3166 Audio:  ");
  Screen.print(1, "Press A to record", true);
  Screen.print(2, "Press B to play", true);
}

void record()
{
  // Re-config the audio data format
  Audio.format(8000, 16);

  Serial.println("start recording");
  Screen.clean();
  Screen.print(0, "Start recording");

  // Start to record audio data
  Audio.startRecord(waveFile, AUDIO_SIZE, 3);

  // Check whether the audio record is completed.
  while (Audio.getAudioState() == AUDIO_STATE_RECORDING)
  {
    delay(100);
  }
  
  Screen.clean();
  Screen.print(0, "Finish recording");
  Audio.getWav(&totalSize);
  Serial.print("Recorded size: ");
  Serial.println(totalSize);

  printIdleMessage();
}

void play()
{
  Screen.clean();
  Screen.print(0, "Start playing");
  Audio.startPlay(waveFile, totalSize);
  
  while (Audio.getAudioState() == AUDIO_STATE_PLAYING)
  {
    delay(100);
  }
  
  Screen.print(0, "Stop playing");
  printIdleMessage();
}

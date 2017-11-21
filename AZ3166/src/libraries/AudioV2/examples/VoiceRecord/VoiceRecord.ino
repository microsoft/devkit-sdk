#include "Arduino.h"
#include "OledDisplay.h"
#include "RingBuffer.h"
#include "AudioClassV2.h"

static AudioClass& Audio = AudioClass::getInstance();
static int AUDIO_SIZE = 32000 * 3 + 45;
static char emptyAudio[AUDIO_CHUNK_SIZE];

RingBuffer ringBuffer(AUDIO_SIZE);
char readBuffer[AUDIO_CHUNK_SIZE];
bool startPlay = false;
int lastButtonAState;
int buttonAState;
int lastButtonBState;
int buttonBState;

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

  memset(emptyAudio, 0x0, AUDIO_CHUNK_SIZE);
  printIdleMessage();
}

void loop(void)
{
  buttonAState = digitalRead(USER_BUTTON_A);
  buttonBState = digitalRead(USER_BUTTON_B);
    
  if (buttonAState == LOW && lastButtonAState == HIGH)
  {
    Screen.clean();
    Screen.print(0, "Start recording:");
    ringBuffer.clear();
    record();
    while(digitalRead(USER_BUTTON_A) == LOW && ringBuffer.available() > 0)
    {
      delay(10);
    }
    if (Audio.getAudioState() == AUDIO_STATE_RECORDING)
    {
      Audio.stop();
    }
    startPlay = true;
    printIdleMessage();
  }

  if (buttonBState == LOW && lastButtonBState == HIGH)
  {
    if (startPlay == true)
    {
      Screen.clean();
      Screen.print(0, "start playing");
      play();
      while(ringBuffer.use() >= AUDIO_CHUNK_SIZE)
      {
        delay(10);
      }
      Audio.stop();
      startPlay = false;
      printIdleMessage();
    }
    else
    {
      Screen.clean();
      Screen.print(0, "Nothing to play");
      Screen.print(1, "Hold A to Record", true);
    }
  }
  lastButtonAState = buttonAState;
  lastButtonBState = buttonBState;
}

void printIdleMessage()
{
  Screen.clean();
  Screen.print(0, "AZ3166 AudioV2:  ");
  Screen.print(1, "Hold A to Record", true);
  Screen.print(2, "Press B to play", true);
}

void record()
{
  Serial.println("start recording");
  ringBuffer.clear();
  Audio.format(8000, 16);
  Audio.startRecord(recordCallback);
}

void play()
{
  Serial.println("start playing");
  Audio.format(8000, 16);
  Audio.startPlay(playCallback);
}

void playCallback(void)
{
  if (ringBuffer.use() < AUDIO_CHUNK_SIZE)
  {
    Audio.writeToPlayBuffer(emptyAudio, AUDIO_CHUNK_SIZE);
    return;
  }
  int length = ringBuffer.get((uint8_t*)readBuffer, AUDIO_CHUNK_SIZE);
  Audio.writeToPlayBuffer(readBuffer, length);
}

void recordCallback(void)
{
  int length = Audio.readFromRecordBuffer(readBuffer, AUDIO_CHUNK_SIZE);
  ringBuffer.put((uint8_t*)readBuffer, length);
}

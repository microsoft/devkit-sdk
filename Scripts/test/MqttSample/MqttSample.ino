#include "_iothub_client_sample_mqtt.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include "azure_c_shared_utility/crt_abstractions.h"
NetworkInterface *network;
static char msgText[1024];
static char temp[100];
static double PRECISION = 0.00000000000001;
/**
 * Float to ASCII
 */
char *ftoa(float n, char *s)
{
  // handle special cases
  if (isnan(n))
  {
    strcpy(s, "nan");
  }
  else if (isinf(n))
  {
    strcpy(s, "inf");
  }
  else if (n == 0.0)
  {
    strcpy(s, "0");
  }
  else
  {
    int digit, m, m1;
    char *c = s;
    int neg = (n < 0);
    if (neg)
      n = -n;
    // calculate magnitude
    m = log10(n);
    int useExp = (m >= 14 || (neg && m >= 9) || m <= -9);
    if (neg)
      *(c++) = '-';
    // set up for scientific notation
    if (useExp)
    {
      if (m < 0)
        m -= 1.0;
      n = n / pow(10.0, m);
      m1 = m;
      m = 0;
    }
    if (m < 1.0)
    {
      m = 0;
    }
    // convert the number
    while (n > PRECISION || m >= 0)
    {
      float weight = pow(10.0, m);
      if (weight > 0 && !isinf(weight))
      {
        digit = floor(n / weight);
        n -= (digit * weight);
        *(c++) = '0' + digit;
      }
      if (m == 0 && n > 0)
        *(c++) = '.';
      m--;
    }
    if (useExp)
    {
      // convert the exponent
      int i, j;
      *(c++) = 'e';
      if (m1 > 0)
      {
        *(c++) = '+';
      }
      else
      {
        *(c++) = '-';
        m1 = -m1;
      }
      m = 0;
      while (m1 > 0)
      {
        *(c++) = '0' + m1 % 10;
        m1 /= 10;
        m++;
      }
      c -= m;
      for (i = 0, j = m - 1; i < j; i++, j--)
      {
        // swap without temporary
        c[i] ^= c[j];
        c[j] ^= c[i];
        c[i] ^= c[j];
      }
      c += m;
    }
    *(c) = '\0';
  }
  return s;
}

#define pulsePin D1
#define pulseLedPin LED_BUILTIN
#define lightPin PB_12
bool buttonStateChanged = false;
volatile byte state = HIGH;
void turnLightOnOrOff(bool status) {
   Serial.println("-----------------------------------------------");
  digitalWrite(lightPin, status ? HIGH : LOW);
}
void pulseStateHook()
{
  auto prev = state;
  state = digitalRead(pulsePin);
  buttonStateChanged = prev != state;
}
void setup()
{
  pinMode(pulseLedPin, OUTPUT);
  pinMode(lightPin, OUTPUT);
  pinMode(pulsePin, INPUT);
  // pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("start");
  digitalWrite(lightPin, HIGH);

  iothub_client_sample_mqtt_init();
}
void reportPulse()
{
  if (state == LOW)
  {
    Serial.println("*****************************SendEvent********************************");
    sprintf(msgText, "{\"deviceId\":\"%s\",\"windSpeed\":%s}", "andy", ftoa((rand() % 1001) / 100.0f, temp));
    iothub_client_sample_send_event((const unsigned char *)msgText);
  }
}
void loop()
{
  pulseStateHook();
  if (buttonStateChanged)
  {
    buttonStateChanged = false;
    reportPulse();
  }
  digitalWrite(pulseLedPin, state);
  iothub_client_sample_mqtt_loop();
}

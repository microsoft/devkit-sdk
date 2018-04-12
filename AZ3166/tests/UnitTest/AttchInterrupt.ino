volatile byte state = LOW;
RGB_LED rgb;

void pinInit()
{
  pinMode(LED_USER, OUTPUT);
  pinMode(PB_2, OUTPUT);

  digitalWrite(PB_2, state);
  rgb.setColor(0, 0, 0);

  attachInterrupt(PA_5, blink, FALLING);
  attachInterrupt(PA_10, blink, FALLING);
  attachInterrupt(PB_2, blink, CHANGE);
  attachInterrupt(PB_3, blink, CHANGE);
}

test(PB_2)
{
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(PB_2, !digitalRead(PB_2));
    delay(LOOP_DELAY);
  }
}

test(PB_3)
{
  for (int i = 0; i < 5; i++)
  {
    rgb.setColor(0, 255, 0);
    delay(LOOP_DELAY);

    rgb.setColor(0, 0, 0);
    delay(LOOP_DELAY);
  }
}

void blink()
{
  state = !state;

  digitalWrite(LED_USER, state);
}
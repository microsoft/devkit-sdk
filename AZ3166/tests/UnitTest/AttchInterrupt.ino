volatile byte state = LOW;
byte state_bk = LOW;
RGB_LED rgb;

void pinInit()
{
  pinMode(PA_5, OUTPUT);
  pinMode(PB_0, OUTPUT);
  pinMode(PB_2, OUTPUT);
  pinMode(PB_6, OUTPUT);
  pinMode(PB_7, OUTPUT);
  pinMode(PB_13, OUTPUT);
  pinMode(PB_15, OUTPUT);

  digitalWrite(PB_2, state);
  digitalWrite(LED_USER, state);
  rgb.setColor(0, 0, 0);

  attachInterrupt(PA_4, blink, RISING);
  attachInterrupt(PA_5, blink, CHANGE);
  attachInterrupt(PA_10, blink, FALLING);
  attachInterrupt(PB_0, blink, CHANGE);
  attachInterrupt(PB_2, blink, CHANGE);
  attachInterrupt(PB_3, blink, CHANGE);
  attachInterrupt(PB_6, blink, CHANGE);
  attachInterrupt(PB_7, blink, CHANGE);
  attachInterrupt(PB_13, blink, CHANGE);
  attachInterrupt(PB_15, blink, CHANGE);
}

test(PA_5)
{
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(PA_5, !digitalRead(PA_5));
    assertEqual(digitalRead(LED_USER), state_bk);
    delay(LOOP_DELAY);
  }
}

test(PB_0)
{
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(LED_USER, state);
    digitalWrite(PB_0, !digitalRead(PB_0));
    assertEqual(digitalRead(LED_USER), state_bk);
    delay(LOOP_DELAY);
  }
}

test(PB_2)
{
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(PB_2, !digitalRead(PB_2));
    assertEqual(digitalRead(LED_USER), state_bk);
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

test(PB_6)
{
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(PB_6, !digitalRead(PB_6));
    assertEqual(digitalRead(LED_USER), state_bk);
    delay(LOOP_DELAY);
  }
}

test(PB_7)
{
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(PB_7, !digitalRead(PB_7));
    assertEqual(digitalRead(LED_USER), state_bk);
    delay(LOOP_DELAY);
  }
}

test(PB_13)
{
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(PB_13, !digitalRead(PB_13));
    assertEqual(digitalRead(LED_USER), state_bk);
    delay(LOOP_DELAY);
  }
}

test(PB_15)
{
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(PB_15, !digitalRead(PB_15));
    assertEqual(digitalRead(LED_USER), state_bk);
    delay(LOOP_DELAY);
  }
}

void blink()
{
  state = !state;
  state_bk = state;
  digitalWrite(LED_USER, state);
}
volatile int counter = 0;

test(PA_5)
{
  check_rising(PA_5);
}

test(PB_2)
{
  check_change(PB_2);
}

test(PB_3)
{
  check_rising(PB_3);
}

test(PB_6)
{
  check_falling(PB_6);
}

test(PB_7)
{
  check_change(PB_7);
}

test(PB_13)
{
  check_rising(PB_13);
}

test(PB_14)
{
  check_falling(PB_14);
}

test(PB_15)
{
  check_change(PB_15);
}

void check_rising(PinName pin)
{
  digitalWrite(pin, LOW);
  counter = 0;

  attachInterrupt(pin, change, RISING);

  digitalWrite(pin, LOW);
  assertEqual(counter, 0);
  delay(LOOP_DELAY);

  digitalWrite(pin, HIGH);
  assertEqual(counter, 1);
  delay(LOOP_DELAY);

  digitalWrite(pin, LOW);
  assertEqual(counter, 1);
  delay(LOOP_DELAY);

  digitalWrite(pin, HIGH);
  assertEqual(counter, 2);
  delay(LOOP_DELAY);
}

void check_falling(PinName pin)
{
  digitalWrite(pin, HIGH);
  counter = 0;

  attachInterrupt(pin, change, FALLING);

  digitalWrite(pin, LOW);
  assertEqual(counter, 1);
  delay(LOOP_DELAY);

  digitalWrite(pin, HIGH);
  assertEqual(counter, 1);
  delay(LOOP_DELAY);

  digitalWrite(pin, LOW);
  assertEqual(counter, 2);
  delay(LOOP_DELAY);

  digitalWrite(pin, HIGH);
  assertEqual(counter, 2);
  delay(LOOP_DELAY);
}

void check_change(PinName pin)
{
  digitalWrite(pin, HIGH);
  counter = 0;

  attachInterrupt(pin, change, CHANGE);

  digitalWrite(pin, LOW);
  assertEqual(counter, 1);
  delay(LOOP_DELAY);

  digitalWrite(pin, HIGH);
  assertEqual(counter, 2);
  delay(LOOP_DELAY);

  digitalWrite(pin, LOW);
  assertEqual(counter, 3);
  delay(LOOP_DELAY);

  digitalWrite(pin, HIGH);
  assertEqual(counter, 4);
  delay(LOOP_DELAY);
}

void change()
{
  counter++;
}
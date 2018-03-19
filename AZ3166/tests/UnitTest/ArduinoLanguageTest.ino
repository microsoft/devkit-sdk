test(analog_read)
{
    analogReadResolution(10);
    int result  = analogRead(ARDUINO_PIN_A0);
    assertMoreOrEqual(result, 0);
    assertLessOrEqual(result, 1023);

    analogReadResolution(12);
    result  = analogRead(ARDUINO_PIN_A0);
    assertMoreOrEqual(result, 0);
    assertLessOrEqual(result, 4095);

    delay(LOOP_DELAY);
}

test(analog_write)
{
    int val = 0;

    analogReadResolution(10);
    val= analogRead(ARDUINO_PIN_A0);
    Serial.print("Analog Read:");
    Serial.println(val);

    analogWriteResolution(10);
    analogWrite(ARDUINO_PIN_A5, val);
    assertEqual(analogRead(ARDUINO_PIN_A5), val);

    delay(LOOP_DELAY);
}

test(digital_io)
{
    int val = 0;
    Serial.println("You can press button A to check the LED status");
    val = digitalRead(USER_BUTTON_A);
    assertEqual(val, HIGH);

    digitalWrite(LED_BUILTIN, val);
    assertEqual(digitalRead(LED_BUILTIN), val);
    delay(LOOP_DELAY);

    digitalWrite(LED_BUILTIN, HIGH);
    assertEqual(digitalRead(LED_BUILTIN), HIGH);
    delay(LOOP_DELAY);

    digitalWrite(LED_BUILTIN, LOW);
    assertEqual(digitalRead(LED_BUILTIN), LOW);

    delay(LOOP_DELAY);
}

test(serial_print)
{
    int x =0;

    Serial.print("No Format");
    Serial.print("\t");

    Serial.print("DEC");
    Serial.print("\t");

    Serial.print("HEX");
    Serial.print("\t");

    Serial.print("OCT");
    Serial.print("\t");

    Serial.print("BIN");
    Serial.print("\n");

    for(x=0; x<20; x++)
    {
        Serial.print(x);
        Serial.print("\t\t");

        Serial.print(x, DEC);
        Serial.print("\t");

        Serial.print(x, HEX);
        Serial.print("\t");

        Serial.print(x, OCT);
        Serial.print("\t");

        Serial.print(x, BIN);
        Serial.println();

        delay(LOOP_DELAY);
    }

    Serial.println(x, 1);
    Serial.println(12.3456, 2);

    int analogValue = analogRead(ARDUINO_PIN_A0);
    Serial.println(analogValue);
    Serial.println(analogValue, DEC);
    Serial.println(analogValue, HEX);
    Serial.println(analogValue, OCT);
    Serial.println(analogValue, BIN);

    // test printf() and printf_P()
    Serial.printf("testing printf() - String: %s; the length is %d\n", "hello", 5);
    Serial.printf_P("testing printf_P() - String: %s; the length is %d", "hello", 5);
    Serial.println();

    // test print(double n, int digits)
    Serial.print(11.2233,2);
    Serial.println();

    delay(LOOP_DELAY);
}

/***************************************************************************************
 *                            Arduino Data Type Testing                                *
 ***************************************************************************************
 */
test(bits)
{
    int value = 22;

    int x = bitRead(value,3);
    assertEqual(x,0);

    x=bitWrite(value,3,1);
    assertEqual(x,30);

    x = bitSet(value,3);
    assertEqual(x,30);

    x = bitClear(value, 2);
    assertEqual(x,26);

    x= bit(2);
    assertEqual(x,4);

    delay(LOOP_DELAY);
}

test(bytes)
{
    BytesData bytes[3]= {
      {1022, 254, 3},
      {1, 1, 0},
      {2048,0,8}
    };
    byte lb, hb;

    for(int i=0; i<3; ++i)
    {
      //LowByte returns the low Byte(that is righmost) of a variable(e.g.a word)
      lb= lowByte(bytes[i].test_data);
      assertEqual(lb, bytes[i].lb);

      //HighByte returns the higher Byte(that is leftmost) of a variable to 2byte(16bit) or the second lowest byte of a larger data type.
      hb= highByte(bytes[i].test_data);
      assertEqual(hb, bytes[i].hb);
    }

    delay(LOOP_DELAY);
}

test(chars)
{
  for(int i=0; i<128; ++i)
  {
    assertTrue(isAscii(i));

    // 0-31, 127: non-printable char
    if((i>=0 && i<=31) || i ==127)
    {
      assertFalse(isPrintable(i));
      assertFalse(isGraph(i));
      assertTrue(isControl(i));

      continue;
    }

    // 32: space
    if(i==32)
    {
      assertTrue(isSpace(i));
      assertTrue(isWhitespace(i));
      continue;
    }

    // 32-126: printable char
    if(i>=33 && i<= 126)
    {
      assertTrue(isPrintable(i));
      assertTrue(isGraph(i));
      assertFalse(isControl(i));
      assertFalse(isSpace(i));
      assertFalse(isWhitespace(i));

      //33-47, 58-64, 91-96, 123-126: is Punct
      if((i>=33 && i<=47) ||
        (i>=58 && i<=64) ||
        (i>=91 && i<=96) ||
        (i>=123 && i<=126))
      {
        assertTrue(isPunct(i));
      }
      else
      {
        assertFalse(isPunct(i));
      }

      //48-57: digit
      if((i>=48 && i<=57))
      {
        assertTrue(isAlphaNumeric(i));
        assertTrue(isDigit(i));
      }
      else
      {
        assertFalse(isDigit(i));
      }

      // 65 - 90: upper char
      if((i>=65 && i<=90))
      {
        assertTrue(isAlphaNumeric(i));
        assertTrue(isAlpha(i));
        assertTrue(isUpperCase(i));

        if(i>=65 && i<=70)
        {
          assertTrue(isHexadecimalDigit(i));
        }
        else
        {
          assertFalse(isHexadecimalDigit(i));
        }
      }
      else
      {
        assertFalse(isUpperCase(i));
      }

      //97 - 122: lower char
      if(i>=97 && i<=122)
      {
        assertTrue(isAlphaNumeric(i));
        assertTrue(isAlpha(i));
        assertTrue(isLowerCase(i));

        if(i>=97 && i<=102)
        {
          assertTrue(isHexadecimalDigit(i));
        }
        else
        {
          assertFalse(isHexadecimalDigit(i));
        }
      }
      else
      {
        assertFalse(isLowerCase(i));
      }
    }
  }

  assertFalse(isAscii(128));

  delay(LOOP_DELAY);
}

test(random)
{
    randomSeed(analogRead(ARDUINO_PIN_A0));
    long randNum = random(300);
    assertLess(randNum, 300);
    assertMoreOrEqual(randNum, 0);

    randNum = random(10,20);
    assertLess(randNum, 20);
    assertMoreOrEqual(randNum, 10);

    delay(LOOP_DELAY);
}

/***************************************************************************************
 *                            Arduino Data Type Testing                                *
 ***************************************************************************************
 */
test(math_abs)
{
    assertEqual(abs(Math_PositiveNum), Math_PositiveNum);
    assertEqual(abs(Math_Zero), Math_Zero);
    assertEqual(abs(Math_NegativeNum), Math_NegativeNum * (-1));

    delay(LOOP_DELAY);
}

test(math_constrain)
{
    // If testVal is less than oneNum, oneNum should be return
    assertEqual(constrain(Math_NegativeNum, Math_Zero, Math_PositiveNum), Math_Zero);

    // If testVal is between oneNum and secondNum, testVal should be return
    assertEqual(constrain(Math_Zero, Math_NegativeNum, Math_PositiveNum), Math_Zero);

    //If testVal is greater than secondNum, secondNum should be return
    assertEqual(constrain(Math_PositiveNum, Math_NegativeNum, Math_Zero), Math_Zero);

    delay(LOOP_DELAY);
}

test(math_map)
{
    int val = analogRead(ARDUINO_PIN_A0);
    val = map(val,0,1023,0,255);

    // map(val,0,1023,0,255): The value shoud be between 0 and 255
    assertMoreOrEqual(val,0);
    assertLessOrEqual(val,255);

    delay(LOOP_DELAY);
}

test(math_max)
{
    assertEqual(max(Math_NegativeNum,Math_PositiveNum), Math_PositiveNum);

    delay(LOOP_DELAY);
}

test(math_min)
{
    assertEqual(min(Math_NegativeNum,Math_PositiveNum), Math_NegativeNum);

    delay(LOOP_DELAY);
}

test(math_pow)
{
    assertEqual(pow(2,3), 8);
    assertEqual(pow(9,0.5), 3);

    delay(LOOP_DELAY);
}

test(math_sqrt)
{
    assertEqual(sqrt(9), 3);
    assertEqual(sqrt(1.0), 1);

    delay(LOOP_DELAY);
}

test(trigonometry)
{
    double data[9]={PI, PI/2,PI/3, PI/4, 0, -PI/2,-PI/3, -PI/4, -PI};

    for(int i=0; i<9; ++i)
    {
        if(data[i] ==0)
        {
            // for sin(0) should be 0
            assertEqual(sin(0), 0);

            // for cos(0) should be 1
            assertEqual(cos(0), 1);

            // for tan(0) should be 0
            assertEqual(tan(0), 0);
        }
        else // except 0, others value are not precise
        {
            int result = sin(data[i]);
            assertLessOrEqual(result,1);
            assertMoreOrEqual(result, -1);

            result = cos(data[i]);
            assertLessOrEqual(result,1);
            assertMoreOrEqual(result, -1);

            //not check the tan() here as it caused out of memory
        }
    }

    delay(LOOP_DELAY);
}

/***************************************************************************************
 *                            Arduino Date Time Testing                                *
 ***************************************************************************************
 */
test(time_delay)
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(LOOP_DELAY);
    digitalWrite(LED_BUILTIN, LOW);
    delay(LOOP_DELAY);

    digitalWrite(LED_BUILTIN, HIGH);
    delayMicroseconds(LOOP_DELAY);
    digitalWrite(LED_BUILTIN, LOW);
    delayMicroseconds(LOOP_DELAY);
}

test(time_micros_millis)
{
    unsigned long T_time;
    Serial.print("Time:");
    T_time = micros();

    //print time
    Serial.println(T_time);
    delay(LOOP_DELAY);

    T_time = millis();

    //print time
    Serial.println(T_time);
    delay(LOOP_DELAY);
}

test(interrupt)
{
    noInterrupts();

    interrupts();

    delay(LOOP_DELAY);
}
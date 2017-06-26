#include <ArduinoUnit.h>
#define LOOP_DELAY          500

void setup(){
  Serial.println(__FILE__);
}

void loop() {
  Test::run();
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
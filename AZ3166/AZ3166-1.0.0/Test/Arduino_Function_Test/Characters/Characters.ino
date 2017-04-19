#include "../Test/TestData.h"

void setup(){
    Serial.begin(115200);    
}

void loop(){
  test();
}

void test()
{
  Serial.println("[Characters]: Test characters related functions");

  for(int i=0; i<128; ++i)
  {
    TestIsAscii(i, true);

    // 0-31, 127: non-printable char
    if((i>=0 && i<=31) || i ==127)
    {
      TestIsPrintable(i, false); 
      TestIsGraph(i, false);  
      TestIsControl(i, true); 
      continue;   
    }

    // 32: space
    if(i==32)
    {
      TestIsSpace(i,true);
      TestIsWhiteSpace(i, true);
      continue;
    }

    // 32-126: printable char
    if(i>=33 && i<= 126)
    {
      TestIsPrintable(i, true); 
      TestIsGraph(i, true); 
      TestIsControl(i, false);
      TestIsSpace(i,false);
      TestIsWhiteSpace(i, false);      

      //33-47, 58-64, 91-96, 123-126: is Punct
      if((i>=33 && i<=47) || 
      (i>=58 && i<=64) ||
      (i>=91 && i<=96) ||
      (i>=123 && i<=126))
      {
        TestIsPunct(i, true);
      }
      else
      {
        TestIsPunct(i, false);
      }

      //48-57: digit
      if((i>=48 && i<=57))
      {
        TestIsAlphaNumeric(i,true);
        TestIsDigit(i, true);
      }
      else
      {
        TestIsDigit(i, false);
      }

      // 65 - 90: upper char
      if((i>=65 && i<=90))
      {
        TestIsAlphaNumeric(i,true);
        TestIsAlpha(i, true);
        TestIsUpperCase(i,true);

        if(i>=65 && i<=70)
        {
          TestIsHexadecimalDigit(i, true);
        }
        else
        {
          TestIsHexadecimalDigit(i, false);
        }
      }
      else
      {
        TestIsUpperCase(i, false);
      }

      //97 - 122: lower char
      if((i>=97 && i<=122))
      {
        TestIsAlphaNumeric(i,true);
        TestIsAlpha(i, true);
        TestIsLowerCase(i,true);

        if(i>=97 && i<=102)
        {
          TestIsHexadecimalDigit(i, true);
        }
        else
        {
          TestIsHexadecimalDigit(i, false);
        }
      }
      else
      {
        TestIsLowerCase(i, false);
      }
    }
  }

  TestIsAscii(128, false);
  
  Serial.println("[Characters]: Done");
  delay(1000);    
}

void TestIsAlphaNumeric(int c, bool expected)
{
  bool actual= isAlphaNumeric(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isAlphaNumeric(): for char - %d, expected: %d, actual: %d", c ,expected ,actual);   
      Serial.println();   
  }
}

void TestIsAlpha(int c, bool expected)
{
  bool actual= isAlpha(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isAlpha(): for char - %d, expected: %d, actual: %d", c ,expected ,actual);     
      Serial.println();  
  }
}

void TestIsAscii(int c, bool expected)
{
  bool actual= isAscii(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isAscii(): for char - %d, expected: %d, actual: %d", c ,expected ,actual); 
      Serial.println();      
  }
}

void TestIsWhiteSpace(int c, bool expected)
{
  bool actual= isWhitespace(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isWhitespace(): for char - %d, expected: %d, actual: %d", c ,expected ,actual);  
      Serial.println();     
  }
}

void TestIsControl(int c, bool expected)
{
  bool actual= isControl(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isControl(): for char - %d, expected: %d, actual: %d", c ,expected ,actual); 
      Serial.println();      
  }
}

void TestIsDigit(int c, bool expected)
{
  bool actual= isDigit(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isDigit(): for char - %d, expected: %d, actual: %d", c ,expected ,actual); 
      Serial.println();      
  }
}

void TestIsGraph(int c, bool expected)
{
  bool actual= isGraph(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isGraph(): for char - %d, expected: %d, actual: %d", c ,expected ,actual);     
      Serial.println();  
  }
}

void TestIsLowerCase(int c, bool expected)
{
  bool actual= isLowerCase(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isLowerCase(): for char - %d, expected: %d, actual: %d", c ,expected ,actual);   
      Serial.println();    
  }
}

void TestIsPrintable(int c, bool expected)
{
  bool actual= isPrintable(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isPrintable(): for char - %d, expected: %d, actual: %d", c ,expected ,actual);      
      Serial.println(); 
  }
}

void TestIsPunct(int c, bool expected)
{
  bool actual= isPunct(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isPunct(): for char - %d, expected: %d, actual: %d", c ,expected ,actual); 
      Serial.println();      
  }
}

void TestIsSpace(int c, bool expected)
{
  bool actual= isSpace(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isSpace(): for char - %d, expected: %d, actual: %d", c ,expected ,actual);  
      Serial.println();     
  }
}

void TestIsUpperCase(int c, bool expected)
{
  bool actual= isUpperCase(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isUpperCase(): for char - %d, expected: %d, actual: %d", c ,expected ,actual);    
      Serial.println();   
  }
}

void TestIsHexadecimalDigit(int c, bool expected)
{
  bool actual= isHexadecimalDigit(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isHexadecimalDigit(): for char - %d, expected: %d, actual: %d", c ,expected ,actual);     
      Serial.println();  
  }
}


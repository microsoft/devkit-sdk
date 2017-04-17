void setup(){
    Serial.begin(9600);
}

void loop(){
  Serial.println("[Characters]: Test characters related functions");
  // isAlphaNumeric
  TestIsAlphaNumeric('-', false);
  TestIsAlphaNumeric('a',true);

  // isAlpha
  TestIsAlpha('2',false);
  TestIsAlpha('s',true);

  //isAscii
  TestIsAscii(128, false);
  TestIsAscii(127, true);
  
  //isWhiteSpace
  TestIsWhiteSpace('a', false);
  TestIsWhiteSpace(' ', true);

  //isControl
  TestIsControl('a', false);
  TestIsControl('\t', true);

  //isDigit
  TestIsDigit('a', false);
  TestIsDigit('1', true);

  //isGraph
  TestIsGraph(7, false);
  TestIsGraph(40, true);

  //isLowerCase
  TestIsLowerCase('A', false);
  TestIsLowerCase('a', true);

  //isPrintable
  TestIsPrintable(7, false);
  TestIsPrintable(40,true);

  //isPunct
  TestIsPunct('a', false);
  TestIsPunct(',', true);

  //isSpace
  TestIsSpace(32, true);
  
  //isUpperCase
  TestIsUpperCase('a', false);
  TestIsUpperCase('A', true);

  //isHexademicalDigit
  TestIsHexadecimalDigit('h',false);
  TestIsHexadecimalDigit('f',true);
  
  Serial.println("[Characters]: Done");
  delay(1000);    
}

void TestIsAlphaNumeric(int c, bool expected)
{
  bool actual= isAlphaNumeric(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isAlphaNumeric(): for char - %s, expected: %s, actual: %s", c ,expected ,actual);      
  }
}

void TestIsAlpha(int c, bool expected)
{
  bool actual= isAlpha(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isAlpha(): for char - %s, expected: %s, actual: %s", c ,expected ,actual);      
  }
}

void TestIsAscii(int c, bool expected)
{
  bool actual= isAscii(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isAscii(): for char - %s, expected: %s, actual: %s", c ,expected ,actual);      
  }
}

void TestIsWhiteSpace(int c, bool expected)
{
  bool actual= isWhitespace(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isWhitespace(): for char - %s, expected: %s, actual: %s", c ,expected ,actual);      
  }
}

void TestIsControl(int c, bool expected)
{
  bool actual= isControl(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isControl(): for char - %s, expected: %s, actual: %s", c ,expected ,actual);      
  }
}

void TestIsDigit(int c, bool expected)
{
  bool actual= isDigit(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isDigit(): for char - %s, expected: %s, actual: %s", c ,expected ,actual);      
  }
}

void TestIsGraph(int c, bool expected)
{
  bool actual= isGraph(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isGraph(): for char - %s, expected: %s, actual: %s", c ,expected ,actual);      
  }
}

void TestIsLowerCase(int c, bool expected)
{
  bool actual= isLowerCase(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isLowerCase(): for char - %s, expected: %s, actual: %s", c ,expected ,actual);      
  }
}

void TestIsPrintable(int c, bool expected)
{
  bool actual= isPrintable(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isPrintable(): for char - %s, expected: %s, actual: %s", c ,expected ,actual);      
  }
}

void TestIsPunct(int c, bool expected)
{
  bool actual= isPunct(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isPunct(): for char - %s, expected: %s, actual: %s", c ,expected ,actual);      
  }
}

void TestIsSpace(int c, bool expected)
{
  bool actual= isSpace(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isSpace(): for char - %s, expected: %s, actual: %s", c ,expected ,actual);      
  }
}

void TestIsUpperCase(int c, bool expected)
{
  bool actual= isUpperCase(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isUpperCase(): for char - %s, expected: %s, actual: %s", c ,expected ,actual);      
  }
}

void TestIsHexadecimalDigit(int c, bool expected)
{
  bool actual= isHexadecimalDigit(c);
  if(actual != expected)
  {
      Serial.printf("[Characters]:Error: characters.isHexadecimalDigit(): for char - %s, expected: %s, actual: %s", c ,expected ,actual);      
  }
}


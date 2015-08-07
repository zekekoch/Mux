class CMux 
{
  private:
  const byte pinA = 17;
  const byte pinB = 18;
  const byte pinC = 19;
  const byte pinZ = 16; 
  
  const byte dialCont = 0;
  const byte dialEmission = 1;
  const byte dialLow = 2;
  const byte dialMedium = 3;
  const byte dialHigh = 4;
  
  byte dialState = 0;
  int cutOffState = 0;
  
  public:
  CMux()
  {
    pinMode(pinZ, INPUT);
    pinMode(pinA, OUTPUT);     
    pinMode(pinB, OUTPUT);     
    pinMode(pinC, OUTPUT); 
  }
  
  byte getDial()
  {
    static unsigned long lastCheck = millis();

    if (millis() > lastCheck + 200)
    {
      lastCheck = millis();
      return dialState;
    }

//            1  3  5  7
// cont-short 0  30 0  30
// emission   0  30 30 30
// dyn-lo     0  30 30 0
// dyn med    30 0  30 0 
// dyn-hi     30 30 30 30

    // cont 110
    // emis 100
    // dlow 101
    // dmed 001
    // dhi  001
    boolean d1 = (read(1) < 10);
    boolean d3 = (read(3) < 10);
    boolean d5 = (read(5) < 10);
    boolean d7 = (read(7) < 10);

    if(d5)
    {
      return dialCont;
    }
    else if (d1)
    {
      if (d7)
      {
        return dialLow;
      }  
      else
      {
        return dialEmission;
      }
    } 
    else if (d3)
    {
      return dialMedium;
    }
    else
    {
      return dialHigh;
    }
  }
  
  bool getDynamicIntensifier()
  {
    return (read(0) < 10);
  }

  int getCutOff()
  {
    static unsigned long lastCheck = millis();

    if (millis() > lastCheck + 1000)
    {
      lastCheck = millis();
    } 
    else
    {
      
    }

    cutOffState = (1023 - read(2));
    return cutOffState;
  }

  
  bool getLifeTest()
  {
    return (read(6) > 0);
  }

  bool getEmission()
  {
    return (read(4) == 0);
  }
  
  void printPad(byte num)
  {
    if (num < 100)
      Serial.print("0");
    if (num < 10)
      Serial.print("0");
    Serial.print(num);
  }
  
  void print()
  {

    Serial.print(" E");
      printPad(read(4));
    Serial.print(" I");
      printPad(read(0));
    Serial.print(" L");
      printPad(read(6));
    Serial.print(" C");
      printPad(read(2));

    Serial.print(" D");
      printPad(getDial());

    Serial.print("  ");
      printPad(read(1));
    Serial.print(" ");
      printPad(read(3));
    Serial.print("  ");
      printPad(read(5));
    Serial.print("  ");
      printPad(read(7));
    Serial.println();
  }
  
  int read(byte index)
  {
    digitalWrite(pinC, bitRead(index,0)); 
    digitalWrite(pinB, bitRead(index,1)); 
    digitalWrite(pinA, bitRead(index,2)); 

    return analogRead(pinZ); 
  }
};
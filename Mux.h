class CMux 
{
  private:

    // this represents the pins on the mux going into the arduino
    const byte pinA = 17;
    const byte pinB = 18;
    const byte pinC = 19;
    const byte pinZ = 16; 
    
    int muxState[8] = {0};

    byte dialState = 0;
    int cutOffState = 0;
  
  // this is the pins going to the buttons
  enum pins
  {
    pinEmission = 0,
    pinLifeTest = 2,
    pinIntensifier = 4,
    pinCutoff = 6,

    pinDialA = 7,
    pinDialB = 5,
    pinDialC = 3,
    pinDialD = 1
  } pins;

  boolean emissionOn = false;
  boolean intensifierOn = false;
  boolean lifeTestOn = false;

  public:

  enum dialState
  {
    dialCont,
    dialEmission,
    dialLow,
    dialMedium,
    dialHigh
  };

  CMux()
  {
    pinMode(pinZ, INPUT);
    pinMode(pinA, OUTPUT);     
    pinMode(pinB, OUTPUT);     
    pinMode(pinC, OUTPUT); 
  }
  
  byte getDial()
  {
    //            1  3  5  7
    // cont-short 0  30 0  30
    // emission   0  30 30 30
    // dyn-lo     0  30 30 0
    // dyn med    30 0  30 0 
    // dyn-hi     30 30 30 30

    static unsigned long lastCheck = millis();

    if (millis() > lastCheck + 200)
    {
      lastCheck = millis();
      return dialState;
    }
    
    if(read(pinDialC) < 5)
    {
      dialState = dialCont;
    }
    else if (read(pinDialA) < 5)
    {
      if (read(pinDialD) < 5)
      {
        dialState =  dialLow;
      }  
      else
      {
        dialState = dialEmission;
      }
    } 
    else if (read(pinDialB) < 5)
    {
      dialState = dialMedium;
    }
    else
    {
      dialState = dialHigh;
    }

    return dialState;
  }
  
  bool getDynamicIntensifier()
  {
    return (read(pinIntensifier) < 10);
  }

  bool isIntensifierOn()
  {    
    if (getDynamicIntensifier());
  }

  byte getCutOff()
  {
    static int lastCutOff = 1023 - read(pinCutoff);
    int cutOff = 1023 - read(pinCutoff);
    int diff = lastCutOff - cutOff;

    if ((diff > 16) || (diff < -16))
      lastCutOff = cutOff;

    return lastCutOff/4;

  }  

  int oldgetCutOff()
  {
    static unsigned long lastCheck = millis();

    cutOffState = (byte)((1023 - read(pinCutoff))/4);
    return cutOffState;
  }

  
  bool getLifeTest()
  {
    return (read(pinLifeTest) > 0);
  }

  bool getEmission()
  {
    return (read(pinEmission) == 0);
  }
  
  void printPad(int num)
  {
    if (num < 1000)
      Serial.print("0");
    if (num < 100)
      Serial.print("0");
    if (num < 10)
      Serial.print("0");
    Serial.print(num);
  }
  
  void print()
  {

    Serial.print(" E");
      printPad(read(pinEmission));
    Serial.print(" I");
      printPad(read(pinIntensifier));
    Serial.print(" L");
      printPad(read(pinLifeTest));
    Serial.print(" C");
      printPad(read(pinCutoff));

    Serial.print(" D");
      printPad(getDial());

    Serial.print("  ");
      printPad(read(pinDialA));
    Serial.print(" ");
      printPad(read(pinDialB));
    Serial.print("  ");
      printPad(read(pinDialC));
    Serial.print("  ");
      printPad(read(pinDialD));
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
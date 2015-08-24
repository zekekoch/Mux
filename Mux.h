class CMux 
{
  private:

    // this represents the pins on the mux going into the arduino
    static const byte pinA = 17;
    static const byte pinB = 18;
    static const byte pinC = 19;
    static const byte pinZ = 16; 
    
    byte dialState = 0;
    int cutOffState = 0;

    typedef bool (*checkButtonState)();
  
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
  } Dial;

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
  
  static bool getDynamicIntensifier()
  {
    return (read(pinIntensifier) < 10);
  }

  bool getButtonState(checkButtonState checkState)
  {
    static bool currentState = false;
    static bool lastState = false;
    static bool isOn = false;

    static unsigned long lastTime = millis();
    if (millis() < lastTime + 25)
    {
      //Serial.println("not long enough");
      return isOn;
    } 
    else
    {
      lastTime = millis();      
    }

    currentState = checkState();


    if (currentState == true && lastState == false)
    {
      if (isOn == true)
        isOn = false;
      else
        isOn = true;
    }
    lastState = currentState;
    return isOn;
  }

  bool isIntensifierOn()
  {
    return getButtonState(getDynamicIntensifier);
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
  
  static bool getLifeTest()
  {
    return (read(pinLifeTest) > 5);
  }

  bool isLifeTestOn()
  {
    return getButtonState(getLifeTest);
  }


  static bool getEmission()
  {
    return (read(pinEmission) == 0);
  }

  bool isEmissionOn()
  {
    return getButtonState(getEmission);
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
  
  static int read(byte index)
  {
    digitalWrite(pinC, bitRead(index,0)); 
    digitalWrite(pinB, bitRead(index,1)); 
    digitalWrite(pinA, bitRead(index,2)); 

    return analogRead(pinZ); 
  }
};
class CMux 
{
  private:

    // this represents the pins on the mux going into the arduino
    static const byte pinA = 16;
    static const byte pinB = 17;
    static const byte pinC = 18;
    static const byte pinD = 19;
    static const byte pinZ = A11; 
    
    byte dialState = 0;
    int cutOffState = 0;

    typedef bool (*checkButtonState)();
  
  // this is the pins going to the buttons
  enum pins
  {
    pinLifeTest = 0,
    pinIntensifier = 1,
    pinEmission = 2,
    pinCutoff = 3,

    pinDialA = 4,
    pinDialB = 5,
    pinDialC = 6,
    pinDialD = 7
  } pins;

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
    pinMode(pinD, OUTPUT); 
  }
  
  byte getDial()
  {
    //            A B C D
    // cont-short 1 1 0 0
    // emission   1 0 0 0
    // dyn-lo     1 0 1 1
    // dyn med    0 0 1 1 
    // dyn-hi     0 0 1 1

    static unsigned long lastCheck = millis();

    if (millis() > lastCheck + 200)
    {
      lastCheck = millis();
      return dialState;
    }


    if(read(pinDialB) > 512)
    {
      dialState = dialCont;
    }
    else if (read(pinDialA) > 512)
    {
      if (read(pinDialC) > 512)
      {
        dialState =  dialLow;
      }  
      else
      {
        dialState = dialEmission;
      }
    } 
    // i can't distinguish between med and hi right now
    else
    {
      dialState = dialMedium;
    }

    return dialState;
  }
  
  // pass in the address of lastState and isOn so that I can store them in the calling function
  bool getButtonState(bool currentState, bool &lastState, bool &isOn, unsigned long &lastTime)
  {
    if (millis() < lastTime + 25)
    {
      //Serial.println("not long enough");
      return isOn;
    } 
    else
    {
      lastTime = millis();      
    }

    if (currentState == true && lastState == false)
    {
      if (isOn == true)
      {
        isOn = false;
      }
      else
      {
        isOn = true;
      }
    }
    lastState = currentState;

    return isOn;
  }

  bool isEmissionOn()
  {
    static bool lastState = 0;
    static bool isOn = 0;    
    static unsigned long lastTime = millis();

    return getButtonState(read(pinEmission) > 512, lastState, isOn, lastTime);
  }

  // for some reason this doesn't work, but Emission does...
  bool isIntensifierOn()
  {
    static bool lastState = 0;
    static bool isOn = 0;  
    static unsigned long lastTime = millis();

    return getButtonState(read(pinIntensifier) > 512, lastState, isOn, lastTime);
  }

  bool isLifeTestOn()
  {
    static bool lastState = 0;
    static bool isOn = 0;
    static unsigned long lastTime = millis();

    return getButtonState(read(pinLifeTest) < 512, lastState, isOn, lastTime);
  }

  byte getCutOff()
  {
    static int lastCutOff = read(pinCutoff);
    int cutOff = read(pinCutoff);
    int diff = lastCutOff - cutOff;

    if ((diff > 16) || (diff < -16))
      lastCutOff = cutOff;

    return lastCutOff/4;
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
    for(int i = 0;i<16;i++)
    {
      Serial.print(i);
      Serial.print(":");
      printPad(read(i));
      Serial.print(" ");
    }
    Serial.println();
    prettyPrint();

  }

  void prettyPrint()
  {

    Serial.print(" I");
    Serial.print(isIntensifierOn());
    //Serial.print(" ");
    //printPad(read(pinIntensifier));

    Serial.print(" E");
    Serial.print(isEmissionOn());
    //Serial.print(" ");
    //printPad(read(pinEmission));

    Serial.print(" L");
    Serial.print(isLifeTestOn());
    //Serial.print(" ");
    //printPad(read(pinLifeTest));

    Serial.print(" C");
    Serial.print(getCutOff());
    Serial.print(" ");
    printPad(read(pinCutoff));

    Serial.print(" D");
      Serial.print(getDial());

/*
    Serial.print("  ");
      printPad(read(pinDialA));
    Serial.print(" ");
      printPad(read(pinDialB));
    Serial.print("  ");
      printPad(read(pinDialC));
    Serial.print("  ");
      printPad(read(pinDialD));
*/
    Serial.println();
  }
  
  static int read(byte index)
  {
    digitalWrite(pinA, bitRead(index,0)); 
    digitalWrite(pinB, bitRead(index,1)); 
    digitalWrite(pinC, bitRead(index,2)); 
    digitalWrite(pinD, bitRead(index,3)); 

    return analogRead(pinZ); 
  }
};
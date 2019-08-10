// I have a 4067 MUX that is connected to a bunch of knobs and dials
// on the Queen's control panel

class CMux 
{
  private:

    // this represents the pins on the mux going into the arduino
    static const byte pinD = 16;
    static const byte pinC = 17;
    static const byte pinA = 18;
    static const byte pinB = 19;
    static const byte pinZ = A11; 
    
    byte dialState = 0;
    int cutOffState = 0;

    // is the virtual button on or off
    int virtualButtonState[16] = {false};

    typedef bool (*checkButtonState)();
  
    // this is the pins going to the buttons
    enum pins
    {
      pinLifeTest = 0,
      pinIntensifier = 1,
      // pinEmission = 2, // I don't think this one works
      pinCutoff = 3,

      pinDialA = 4, // anything except dyn-medium
      pinDialB = 5, // dyn low med or high
      pinDialC = 6, // anything except dyn high
      pinDialD = 7  // same as DialC
    } pins;

    static int getPinState(byte index)
    {
      digitalWrite(pinA, bitRead(index,0)); 
      digitalWrite(pinB, bitRead(index,1)); 
      digitalWrite(pinC, bitRead(index,2)); 
      digitalWrite(pinD, bitRead(index,3)); 

      return analogRead(pinZ); 
    }


  public:

  enum dialState
  {
    dialCont = 0,
    dialEmission = 1,
    dialLow = 2,
    dialMedium = 3,
    dialHigh = 4
  } Dial;

  // setup for mux, make sure the arduino knows which pins are read and write
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
    //            4 5 6
    // cont-short 0 0 0
    // emission   0 0 0
    // dyn-lo     0 1 0
    // dyn med    1 1 0 
    // dyn-hi     0 1 1 

    // only update the dialstate every 200ms to prevent
    // flooding of the system

    static unsigned long lastCheck = millis();
    if (millis() > lastCheck + 200)
    {
      lastCheck = millis();
      return dialState;
    }

    // my cheap analog to digital conversion
    // if it's over 50% of 1024 then it's true otherwise false
    bool a = getPinState(4) > 512;
    bool b = getPinState(5) > 512;
    bool c = getPinState(6) > 512;

    // if the first pin is low then it's definitely dyn-medium 
    if(a)
    { 
      dialState = dialMedium;
    }
    // if the last pin is high then it's definitely dyn-high 
    else if (c)
    {
      dialState = dialHigh;
    }
    // if the middle pin is low then it's dyn-low 
    else if (b)
    {
      dialState = dialLow;
    } 
    // sadly I can't tell the different between cont-short and emission
    else 
    {
      dialState = dialEmission;
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

  void clear()
  {
    for (int i = 0;i<16;i++)
      virtualButtonState[i] = 0;
  }

  void setButtonState(byte button, bool state)
  {
      virtualButtonState[i] = state;
  }

  void getButtonState(byte button)
  {
    return ButtonOn(button);
  }

  bool ButtonDown(byte button)
  {
    return  getPinState(button) < 5 ? true : false;
  }

  // debounce 16 buttons
  //
  // the basic idea is that when they physical button transitions from an up to a down checkButtonState
  // I toggle the state of the virtual button.
  bool ButtonOn(byte button)
  {

    // if the physical button depressed or not
    static int isButtonDown[16] = {false};         // the current isButtonDown from the input pin
    static int wasButtonDown[16] = {false};    // the wasButtonDown isButtonDown from the input pin

    // todo: am I wasting too much time in here, it would be nice to return cached values if it hasn't 
    //       been very long, but the code below only allows the first button to have it's state
    //       updated.
    // perhaps premature optimization, but if I get called too quickly then just return the last value 
    //static long lastButtonCheck = millis();
    //if (millis() < lastButtonCheck + 50)
    //{
    //  return virtualButtonState[button];
    //}
   
    static long time[16] = {0};         // the last time the output pin was toggled
    const long debounce = 200;   // the debounce time, increase if the output flickers

    isButtonDown[button] = ButtonDown(button);
    //Serial.print("b");Serial.print(button);Serial.print(" ");
    //Serial.print(isButtonDown[button]);Serial.print(" ");Serial.print(getPinState(button));Serial.println();

    // if the input just went from LOW and HIGH and we've waited long enough
    // to ignore any noise on the circuit, toggle the output pin and remember
    // the time
    if (isButtonDown[button] == true && wasButtonDown[button] == false && millis() - time[button] > debounce) {
      if (virtualButtonState[button] == true)
        virtualButtonState[button] = false;
      else
        virtualButtonState[button] = true;

      time[button] = millis();    
    }

    //lastButtonCheck = millis();
    wasButtonDown[button] = isButtonDown[button];
    return virtualButtonState[button];
  }

  bool isEmissionOn()
  {
    static bool lastState = 0;
    static bool isOn = 0;    
    static unsigned long lastTime = millis();

    return getButtonState(getDial() == dialEmission, lastState, isOn, lastTime);
  }

  // for some reason this doesn't work, but Emission does...
  bool isIntensifierOn()
  {
    static bool lastState = 0;
    static bool isOn = 0;  
    static unsigned long lastTime = millis();

    return getButtonState(getPinState(pinIntensifier) > 512, lastState, isOn, lastTime);
  }

  bool isLifeTestOn()
  {
    static bool lastState = 0;
    static bool isOn = 0;
    static unsigned long lastTime = millis();

    return getButtonState(getPinState(pinLifeTest) < 512, lastState, isOn, lastTime);
  }

  byte getCutOff()
  {
    static int lastCutOff = getPinState(pinCutoff);
    int cutOff = getPinState(pinCutoff);
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
  
  void printBare()
  {
    for(int i = 0;i<16;i++)
    {
      Serial.print(i);
      Serial.print(":");
      printPad(getPinState(i));
      Serial.print(" ");
    }
    Serial.println();

  }

  void printCarMux()
  {
    for(int i = 0;i<16;i++)
    {
      Serial.print(i);
      Serial.print(":");
      Serial.print(getPinState(i) < 512 ? 1 : 0);
      Serial.print(" ");
    }
    Serial.println();

  }

  void print()
  {
    printBinary();
  }

  void printBinary()
  {
    for(int i = 0;i<16;i++)
    {
      Serial.print(i);
      Serial.print(":");
      Serial.print(getPinState(i) < 5 ? 1 : 0);
      Serial.print(" ");
    }
    Serial.println();

  }



  void prettyPrint()
  {

    Serial.print(" I");
    Serial.print(isIntensifierOn());
    //Serial.print(" ");
    //printPad(getPinState(pinIntensifier));

    Serial.print(" E");
    Serial.print(isEmissionOn());
    //Serial.print(" ");
    //printPad(getPinState(pinEmission));

    Serial.print(" L");
    Serial.print(isLifeTestOn());
    //Serial.print(" ");
    //printPad(getPinState(pinLifeTest));

    Serial.print(" C");
    Serial.print(getCutOff());
    Serial.print(" ");
    printPad(getPinState(pinCutoff));

    Serial.print(" D");
      Serial.print(getDial());

/*
    Serial.print("  ");
      printPad(getPinState(pinDialA));
    Serial.print(" ");
      printPad(getPinState(pinDialB));
    Serial.print("  ");
      printPad(getPinState(pinDialC));
    Serial.print("  ");
      printPad(getPinState(pinDialD));
*/
    Serial.println();
  }
  
};
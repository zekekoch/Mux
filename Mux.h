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

    static bool readPin(byte index)
    {
      digitalWrite(pinA, bitRead(index,0)); 
      digitalWrite(pinB, bitRead(index,1)); 
      digitalWrite(pinC, bitRead(index,2)); 
      digitalWrite(pinD, bitRead(index,3)); 

      return digitalRead(pinZ); 
    }


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
    pinMode(pinZ, INPUT_PULLUP);
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

  void printVBS()
  {
    Serial.println("VirtualButtonState");
    for (int i = 0;i<16;i++)
    {
      Serial.print(virtualButtonState[i]);
      if ((i+1)%4==0)
        Serial.println();
    } 
  }

  bool ButtonDown(byte button)
  {
    // when the button is down sometimes is hovers right above zero
    // do I have a ground issue?
    return  getPinState(button) == 1023 ? true : false;
  }


  // debounce 16 buttons
  //
  // the basic idea is that when they physical button transitions from an up to a down checkButtonState
  // I toggle the state of the virtual button.
  bool ButtonOn(byte button)
  {
    //Serial.print("ButtonOn ");Serial.println(button);
    // if the physical button depressed or not
    static int isButtonDown[16] = {false};         // the current isButtonDown from the input pin
    static int wasButtonDown[16] = {false};    // the wasButtonDown isButtonDown from the input pin
    static long time[16] = {0};         // the last time the output pin was toggled
    const long debounce = 50;   // the debounce time, increase if the output flickers

    // todo: am I wasting too much time in here, it would be nice to return cached values if it hasn't 
    //       been very long, but the code below only allows the first button to have it's state
    //       updated.
    // perhaps premature optimization, but if I get called too quickly then just return the last value 
    //static long lastButtonCheck = millis();
    //if (millis() < lastButtonCheck + 50)
    //{
    //  return virtualButtonState[button];
    //}
   
    isButtonDown[button] = ButtonDown(button);
    //Serial.print("b");Serial.print(button);Serial.print(" ");
    //Serial.print(isButtonDown[button]);Serial.print(" ");Serial.print(getPinState(button));Serial.println();

    // if the input just went from LOW and HIGH and we've waited long enough
    // to ignore any noise on the circuit, toggle the output pin and remember
    // the time
    if (isButtonDown[button] == true && wasButtonDown[button] == false && (millis() - time[button]) > debounce) {
      if (virtualButtonState[button] == true)
      {
        virtualButtonState[button] = false;
      }
      else
      {
        virtualButtonState[button] = true;
        Serial.print("enable ");Serial.print(button);
      }

      time[button] = millis();    
    }

    //lastButtonCheck = millis();
    wasButtonDown[button] = isButtonDown[button];

    //Serial.print("ButtonDown return ");Serial.print(virtualButtonState[button]);Serial.println();
    return virtualButtonState[button];
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
      Serial.print(readPin(i));
      Serial.print(" ");
    }
    Serial.println();

  }
};
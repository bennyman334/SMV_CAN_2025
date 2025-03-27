#include "SMVcanbus.h"
#include <string.h>


//Const declerations
CANBUS can(UI);
double OFF = 0;
double ON = 1;
const int numInputs = 8;
const unsigned long inputDelay = 10;
enum Type {SWITCH, BUTTON};

//H&S Input Class
class Input {
  public:
    //Constructor and destructor
    Input(int pin, Type type, UIMessage message):
      m_inputPin(pin), m_inputType(type), m_message(message), m_inputState(OFF), m_lastCheck(0) {pinMode(pin, INPUT);};
    ~Input() = default;
   
   //Main function - detect the current input state and send messages
    void detectState();
    void initialState();

    //Accessors
    double getState() {return m_inputState;};

    int getPin() {return m_inputPin;};
    Type getType() {return m_inputType;};
    UIMessage getMessageType() {return m_message;};

  private:
    int m_inputPin;
    Type m_inputType;
    double m_inputState;
    UIMessage m_message;
    unsigned long m_lastCheck;
};

void Input::initialState() {
  int pinState = digitalRead(m_inputPin);
  double m_inputState = (pinState == HIGH) ? ON : OFF;
  if(pinState == HIGH) {
        Serial.println("ON!");
  } else {
        Serial.println("OFF!");
  }
  //Don't send anything as of yet
  Serial.println("Sent packet");
  can.send(m_inputState, m_message);
}

//State detection function implementation
void Input::detectState() {
  //Read pin via digitalRead and convert
  int pinState = digitalRead(m_inputPin);
  double newState = (pinState == HIGH) ? ON : OFF;
  //Serial.println("Hi");

  ///ALSO send INITIAL STATES!!! OR ELSE THAT SHIT WON'T WORK!!!
  //Check if state is different
  if (newState != m_inputState) {
    //Check if outside of input delay range (crucial for misinputs)
    if ((millis()-m_lastCheck) > inputDelay) {
      //Update state and check time, send can message
      m_inputState = newState;
      m_lastCheck = millis();

      if(pinState == HIGH) {
        Serial.println("ON!");
      } else {
        Serial.println("OFF!");
      }
      //Don't send anything as of yet
      Serial.println("Sent packet");
      can.send(m_inputState, m_message);
    }
  }
  else {
    //Update check time, valid as no state change
    m_lastCheck = millis();
  }
}

//Input object array
Input* inputObjects[numInputs];

//Arduino setup
void setup() {
  
  //Create input objects and populate
  // inputObjects[0] = new Input(26, SWITCH, Reverse);
  // inputObjects[1] = new Input(27, SWITCH, Headlights);
  // inputObjects[2] = new Input(28, SWITCH, Wipers);
  // inputObjects[3] = new Input(29, SWITCH, Hazard);
  // inputObjects[4] = new Input(9, SWITCH, Blink_Left);
  // inputObjects[5] = new Input(6, SWITCH, Blink_Right);
  // inputObjects[6] = new Input(10, BUTTON, Horn); //NOTE: LIBRARY SHENANIGANS, THIS ACTUALLY SENDS DAQ BUT REFERS TO HORN
  //inputObjects[7] = new Input(11, BUTTON, DAQ);
  //Input spareButton = Input(12, BUTTON);
  //Input spareSwitch = Input(24, SWITCH);


//testing
  inputObjects[0] = new Input(27, SWITCH, Headlights);
  inputObjects[1] = new Input(9, SWITCH, Blink_Left);
  inputObjects[2] = new Input(6, SWITCH, Blink_Right);
  inputObjects[3] = new Input(29, SWITCH, Hazard);
  inputObjects[4] = new Input(13, SWITCH, Motor);
  
  //Set CAN serialization
  Serial.begin(115200);
  can.begin();
  delay(500);

  for (int i=0; i<5; i++) {
    inputObjects[i]->initialState();
  }
}

//Arduino loop
void loop() {
  //Go through each button, check state and send messages
  // for (int i=0; i<numInputs; i++) {
  //   inputObjects[i]->detectState();
  // }
  for (int i=0; i<5; i++) {
    inputObjects[i]->detectState();
  }
  delay(30);
}
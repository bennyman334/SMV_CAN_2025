#include "SMVcanbus.h"
#include <string.h>


//Const declerations
CANBUS can(UI);
double OFF = 0;
double ON = 1;
const int numInputs = 10;
const unsigned long inputDelay = 50;
enum Type {SWITCH, BUTTON}; 

const int BCD_A_pin = 21;
const int BCD_B_pin = 18; 
const int BCD_C_pin = 19; 
const int BCD_D_pin = 20; 
const int digit_1_pin = 15; 
const int digit_2_pin = 16; 
const int digit_3_pin = 17; 
const int digit_4_pin = 14; 
const int dp_pin = 12; 

//H&S Input Class
class Input {
  public:
    //Constructor and destructor
    Input(int pin, Type type, UIMessage message):
      m_inputPin(pin), m_inputType(type), m_message(message), m_inputState(OFF), m_lastCheck(0) {pinMode(pin, INPUT);};
    ~Input() = default;
   
   //Main function - detect the current input state and send messages
    void detectState();
    void sendState(); 

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

//State detection function implementation
void Input::detectState() {
  //Read pin via digitalRead and convert
  int pinState = digitalRead(m_inputPin);
  double newState = (pinState == HIGH) ? ON : OFF;

  //Check if state is different
  if (newState != m_inputState) {
    //Check if outside of input delay range (crucial for misinputs)
    if ((millis()-m_lastCheck) > inputDelay) {
      //Update state and check time, send can message
      m_inputState = newState;
      m_lastCheck = millis();
    }
  }
  else {
    //Update check time, valid as no state change
    m_lastCheck = millis();
  }

  Serial.println("Sent packet");
  can.send(m_inputState, m_message);
}

string intToBinary(int inputNum){
  switch(inputNum){
    case 9: 
      return "1001"; 
    case 8: 
      return "1000"; 
    case 7: 
      return "0111"; 
    case 6: 
      return "0110"; 
    case 5: 
      return "0101"; 
    case 4: 
      return "0100";  
    case 3: 
      return "0011";  
    case 2: 
      return "0010"; 
    case 1: 
      return "0001"; 
    default: 
      return "0000"; 
  }
}

void cd4511_output(string binary_input){
  if(binary_input[0] == '1'){
    digitalWrite(BCD_A_pin, HIGH); 
  } else {
    digitalWrite(BCD_A_pin, LOW); 
  }
  if(binary_input[1] == '1'){
    digitalWrite(BCD_B_pin, HIGH); 
  } else {
    digitalWrite(BCD_B_pin, LOW); 
  }
  if(binary_input[2] == '1'){
    digitalWrite(BCD_C_pin, HIGH); 
  } else {
    digitalWrite(BCD_C_pin, LOW); 
  }
  if(binary_input[3] == '1'){
    digitalWrite(BCD_D_pin, HIGH); 
  } else {
    digitalWrite(BCD_D_pin, LOW); 
  }
}

//Input object array
Input* inputObjects[numInputs];

//Arduino setup
void setup() {
  //Create input objects and populate
  inputObjects[0] = new Input(9, SWITCH, Reverse);
  inputObjects[1] = new Input(10, SWITCH, Headlights);
  inputObjects[2] = new Input(11, SWITCH, Wipers);
  inputObjects[3] = new Input(7, SWITCH, Hazard);
  inputObjects[4] = new Input(22, SWITCH, Blink_Left);
  inputObjects[5] = new Input(23, SWITCH, Blink_Right);
  inputObjects[6] = new Input(24, BUTTON, Horn); 
  inputObjects[7] = new Input(5, BUTTON, DAQ);
  inputObjects[8] = new Input(4, BUTTON, Spare_Button); 
  inputObjects[9] = new Input(24, SWITCH, Ignition); 

  // For Seven-Segment Display
  pinMode(BCD_A_pin, OUTPUT); 
  pinMode(BCD_B_pin, OUTPUT); 
  pinMode(BCD_C_pin, OUTPUT);
  pinMode(BCD_D_pin, OUTPUT);
  pinMode(digit_1_pin, OUTPUT);
  pinMode(digit_2_pin, OUTPUT);
  pinMode(digit_3_pin, OUTPUT);
  pinMode(digit_4_pin, OUTPUT);
  pinMode(dp_pin, OUTPUT); 

  bool buttonCheck = false; 
  
  //Set CAN serialization
  Serial.begin(115200);
  can.begin();
  delay(500);
  
}

//Arduino loop
void loop() {
  //Go through each button, check state and send messages
  if(buttonCheck){
     for (int i=0; i<numInputs; i++) {
        inputObjects[i]->detectState();
     }
     buttonCheck = false; 
  } else {
    buttonCheck = true; 
  }
  
  int displayNumber = can.receive(); // Send a value in form of "xxxx", no decimals, x can be 0
  
  int digit4_num = displayNumber % 10; 
  string digit4_binary = intToBinary(digit4_num); 
  digitalWrite(digit_4_pin, HIGH); 
  cd4511_output(digit4_binary); 
  delay(50); 
  digitalWrite(digit_4_pin, LOW); 
  
  displayNumber = displayNumber/10; 
  int digit3_num = displayNumber % 10; 
  string digit3_binary = intToBinary(digit3_num); 
  digitalWrite(digit_3_pin, HIGH); 
  cd4511_output(digit3_binary); 
  delay(50); 
  digitalWrite(digit_3_pin, LOW); 

  displayNumber = displayNumber/10; 
  int digit2_num = displayNumber % 10; 
  string digit2_binary = intToBinary(digit2_num); 
  digitalWrite(digit_2_pin, HIGH); 
  cd4511_output(digit2_binary); 
  delay(50); 
  digitalWrite(digit_2_pin, LOW); 
  
  displayNumber = displayNumber/10; 
  int digit1_num = displayNumber % 10; 
  string digit1_binary = intToBinary(digit1_num); 
  digitalWrite(digit_1_pin, HIGH); 
  cd4511_output(digit1_binary); 
  delay(50); 
  digitalWrite(digit_1_pin, LOW); 
}

#include "SMVcanbus.h"

CANBUS can(HS3);

double datarec = 0; //for headlights
double datarec1 = 0; //for blinker
double datarec2 = 0; //for hazard;

const int LED = 12;
const int headlight = 5; //bottom connector
const int runninglight = 11;
const int blinker = 6; //middle connector

bool isHazard = false;
int hazardState = 0;
int blinkerState = 0;

void setup(void){ //do something to detect initial state?
  Serial.begin(115200);
  can.begin();
  delay(400); //for printing
  pinMode(LED, OUTPUT);
  pinMode(headlight, OUTPUT);
  pinMode(blinker, OUTPUT);
  pinMode(runninglight, OUTPUT);

}

int blinkLight(int currentState) {
  if(currentState == 0) {
    return 1;
  }
  return 0;
}



void loop(){
  can.looper();
  if(can.isThere() && strcmp(can.getHardware(), "UI") == 0)
  {
    // Serial.print("The data is: ");
    // Serial.println(can.getData());
    // Serial.print("The Hardware Type is: ");
    // Serial.println(can.getHardware());
    // Serial.print("The Data Type is: ");
    // Serial.println(can.getDataType());

    if(strcmp(can.getDataType(), "Headlights") == 0){
      datarec = can.getData();
    }

    if(strcmp(can.getDataType(), "Blink_Left") == 0) {
      datarec1 = can.getData();
    }

    if(strcmp(can.getDataType(), "Hazard") == 0){ //override the headlights ()
      datarec2 = can.getData();
      if(datarec2 > 0) {
        isHazard = true;
      } else {
        isHazard = false;
      }
    }
    
  }

  if(isHazard) { //Hazard lights will override the headlights
    hazardState = blinkLight(hazardState); 
    digitalWrite(headlight, hazardState);
  }

//will follow headlights only if hazard is not on
  if(datarec == 0 && !isHazard) {
    //digitalWrite(LED, LOW);
    digitalWrite(headlight, LOW);
  } else if (datarec > 0 && !isHazard) {
    //digitalWrite(LED, HIGH);
    digitalWrite(headlight, HIGH);
  }

  if(datarec1 == 0) {
    digitalWrite(LED, LOW);
    digitalWrite(blinker, LOW);
  } else if (datarec1 > 0) {
    digitalWrite(LED, HIGH);
    //digitalWrite(blinker, HIGH);

    if(blinkerState == 0) {
      blinkerState = 1;
    } else {
      blinkerState = 0;
    }
    //if(blinkerState )
    if(blinkerState == 0) {
      digitalWrite(blinker, LOW);
    } else {
      digitalWrite(blinker, HIGH);
    }
  }
  delay(250);
}
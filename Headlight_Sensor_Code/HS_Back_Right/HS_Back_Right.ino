//#include <SMVcanbus.h>
#include "SMVcanbus.h"

CANBUS can(HS3);

double datarec = 0; //for brakelight
double datarec1 = 0; //for blinker
double datarec2 = 0; //for hazard;
double datarec3 = 0; //for runninglights (determined by motor on or off)

const int LED = 12;
const int brakelight = 5; //bottom connector
const int runninglight = 11; //top connector
const int blinker = 6; //middle connector

bool isHazard = false; //whether hazard light is on or not
int hazardState = 0; 
int blinkerState = 0;

int blinkCycle = 0;
int hazardCycle = 0;

void setup(void){ //do something to detect initial state?
  Serial.begin(115200);
  can.begin();
  delay(400); //for printing
  pinMode(LED, OUTPUT);
  pinMode(brakelight, OUTPUT);
  pinMode(blinker, OUTPUT);
  pinMode(runninglight, OUTPUT);

}

int blinkLight(int currentState) { //input 1, will output 0 and vice versa (change states)
  if(currentState == 0) {
    return 1;
  }
  return 0;
}


void loop(){
  can.looper();
  if(can.isThere() && (strcmp(can.getHardware(), "UI") == 0 || strcmp(can.getHardware(), "FC") == 0))
  {
    // Serial.print("The data is: ");
    // Serial.println(can.getData());
    // Serial.print("The Hardware Type is: ");
    // Serial.println(can.getHardware());
    // Serial.print("The Data Type is: ");
    // Serial.println(can.getDataType());

    if(strcmp(can.getDataType(), "Brake") == 0){
      datarec = can.getData();
    }

    if(strcmp(can.getDataType(), "Blink_Right") == 0) {
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

    if(strcmp(can.getDataType(), "Motor") == 0){
      datarec3 = can.getData();
    }
    
  }

  if(isHazard) { //Hazard lights will override the running lights
    if(hazardCycle%10 == 0) {
      hazardState = blinkLight(hazardState); 
    }
    digitalWrite(runninglight, hazardState);
    hazardCycle = (hazardCycle + 1)%10; //keeps hazardCycle between 0 to 9 (we don't want int to get too big i think)
  } else {
    if(datarec3 > 0) { //motor on
      digitalWrite(runninglight, HIGH); //runninglight is always on when motor is on (aside from when hazard light activated)
    } else if (datarec3 == 0) {
      digitalWrite(runninglight, LOW); //motor off so runninglight is off
    }
  }

//brake lights will be based on the hall sensor
  if(datarec <= 0.25) {
    //digitalWrite(LED, LOW);
    digitalWrite(brakelight, LOW); //no brake detected
  } else if (datarec > 0.25) {
    //digitalWrite(LED, HIGH);
    digitalWrite(brakelight, HIGH);
  }

  if(datarec1 == 0) {
    digitalWrite(LED, LOW);
    digitalWrite(blinker, LOW);
  } else if (datarec1 > 0) { //every 10 iterations will change its blinkState
    digitalWrite(LED, HIGH);
    if(blinkCycle%10 == 0) {
      blinkerState = blinkLight(blinkerState);
    }
    digitalWrite(blinker, blinkerState);
    blinkCycle = (blinkCycle + 1)%10;
  }

  delay(25);
}
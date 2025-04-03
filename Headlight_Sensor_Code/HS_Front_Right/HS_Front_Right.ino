#include <Adafruit_MCP2515.h>

//#include <SMVcanbus.h>
#include "smv_accel.h"
#include "SMVcanbus.h"

CANBUS can(HS1);

double datarec = 0; //for headlights
double datarec1 = 0; //for blinker
double datarec2 = 0; //for hazard;

const int LED = 12;
const int headlight = 5; //bottom connector
const int runninglight = 11;
const int blinker = 6; //middle connector

bool isHazard = false; //whether hazard light is on or not
int hazardState = 0; 
int blinkerState = 0;

int blinkCycle = 0;
int hazardCycle = 0;

int sendBuffer = 0;
int accelSendState = 0;

//accelerometer definitions
const int CS_PIN = 13;
ASM330LHH sensor(CS_PIN);

void setup(void){ //do something to detect initial state?
  Serial.begin(115200);
  can.begin();
  delay(400); //for printing
  pinMode(LED, OUTPUT);
  pinMode(headlight, OUTPUT);
  pinMode(blinker, OUTPUT);
  pinMode(runninglight, OUTPUT);

  //---Accelerometer---
  sensor.begin();
}

int blinkLight(int currentState) { //input 1, will output 0 and vice versa (change states)
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
    
  }

  if(isHazard) { //Hazard lights will override the headlights
    if(hazardCycle%10 == 0) {
      hazardState = blinkLight(hazardState); 
    }
    digitalWrite(headlight, hazardState);
    hazardCycle = (hazardCycle + 1)%10; //keeps hazardCycle between 0 to 9 (we don't want int to get too big i think)
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
  } else if (datarec1 > 0) { //every 10 iterations will change its blinkState
    digitalWrite(LED, HIGH);
    if(blinkCycle%10 == 0) {
      blinkerState = blinkLight(blinkerState);
    }
    digitalWrite(blinker, blinkerState);
    blinkCycle = (blinkCycle + 1)%10;
  }

//    //------Accelerometer CAN stuff-------
//  int32_t accelerometer[3] = {};
//  int32_t gyroscope[3] = {};
//  if (sendBuffer%20 == 0){
//    sensor.readAccelerometer(accelerometer);
//    // sensor.readGyroscope(gyroscope);
//    
//    if(accelSendState == 0){
//      can.send(accelerometer[0], Accel_x);
//      accelSendState = 1;
//    }
//    else if(accelSendState == 1){
//      can.send(accelerometer[1], Accel_y);
//      accelSendState = 2;
//    }
//    else if(accelSendState == 2){
//      can.send(accelerometer[2], Accel_z);
//      accelSendState = 0;
//    }
//    
//    // can.send(gyroscope[0], Gyro_x);
//    // can.send(gyroscope[1], Gyro_y);
//    // can.send(gyroscope[2], Gyro_z);
//  }
//  sendBuffer += 1;
  
  delay(25);
}

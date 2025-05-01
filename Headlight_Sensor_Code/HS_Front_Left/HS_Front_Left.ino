//#include <SMVcanbus.h>
#include <Adafruit_MCP2515.h>
#include "SMVcanbus.h"
#include "smv_accel.h"

CANBUS can(HS2);

#define ROLL 0
#define PITCH 0

//---------Data Recordings from CAN Receives--------------
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
    if(hazardCycle%10 == 0) {
      hazardState = blinkLight(hazardState); 
      digitalWrite(headlight, hazardState);
    }
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
      digitalWrite(blinker, blinkerState);
    }
    blinkCycle = (blinkCycle + 1)%10;
  }

// ------Accelerometer CAN stuff-------
 int32_t accelerometer[3] = {};
 int32_t gyroscope[3] = {};
 sensor.readAccelerometer(accelerometer);
  sensor.readGyroscope(gyroscope);

  int32_t x_acc = accelerometer[0];
  int32_t y_acc = accelerometer[1];
  int32_t z_acc = accelerometer[2];

  int32_t x_gyro = gyroscope[0];
  int32_t y_gyro = gyroscope[1];
  int32_t z_gyro = gyroscope[2];

  int32_t front_acc = -x_acc;
  int32_t right_acc = y_acc;
  int32_t up_acc = z_acc;

  int32_t front_gyro = -x_gyro;
  int32_t right_gyro = y_gyro;
  int32_t up_gyro = z_gyro;

  double global_front_acc = toGlobalFront(front_acc, right_acc, up_acc);
  double global_front_gyro = toGlobalFront(front_gyro, right_gyro, up_gyro);
  //global_front_mag = toGlobalFront(front_mag, right_mag, up_mag);

  double global_right_acc = toGlobalRight(front_acc, right_acc, up_acc);
  double global_right_gyro = toGlobalRight(front_gyro, right_gyro, up_gyro);
  //global_right_mag = toGlobalRight(front_mag, right_mag, up_mag);

  double global_up_acc = toGlobalUp(front_acc, right_acc, up_acc);
  double global_up_gyro = toGlobalUp(front_gyro, right_gyro, up_gyro);
  //global_up_mag = toGlobalUp(front_mag, right_mag, up_mag);

//  ------Accelerometer CAN stuff-------
//  int32_t accelerometer[3] = {};
//  int32_t gyroscope[3] = {};
//
//  if (sendBuffer%20 == 0){
//    sensor.readAccelerometer(accelerometer);
//    sensor.readGyroscope(gyroscope);
//    
//    // can.send(accelerometer[0], Accel_x);
//    // can.send(accelerometer[1], Accel_y);
//    // can.send(accelerometer[2], Accel_z);
//    // can.send(gyroscope[0], Gyro_x);
//    // can.send(gyroscope[1], Gyro_y);
//    // can.send(gyroscope[2], Gyro_z);
//  }
//  sendBuffer += 1;

  delay(25);
}


double toGlobalFront(double front, double right, double up) {
  return front*cos(PITCH) - up*sin(PITCH);
}
double toGlobalRight(double front, double right, double up) {
  return front*sin(ROLL)*sin(PITCH) + right*cos(ROLL) + up*sin(ROLL)*cos(PITCH);
}
double toGlobalUp(double front, double right, double up) {
  return front*cos(ROLL)*sin(PITCH) - right*sin(ROLL) + up*cos(PITCH)*cos(ROLL);
}

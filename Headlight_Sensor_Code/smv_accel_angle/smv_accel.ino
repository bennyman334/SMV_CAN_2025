#include "smv_accel.h"

// -----------------------------------------------------------------------------------------------
// THIS IS A BASIC EXAMPLE .INO
// MODIFY THIS FILE TO CODE YOUR DEVICE
// -----------------------------------------------------------------------------------------------

// Define SPI Chip Select pin for RP2040
const int CS_PIN = 13;

int front_sum = 0;
int right_sum = 0;
int up_sum = 0;

int i = 0;

ASM330LHH sensor(CS_PIN);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Test!");
  sensor.begin();
}

void loop() {
    if(i%5 == 0){
      sensor.printSensorData();
    }

    int32_t accelerometer[3] = {};
    int32_t gyroscope[3] = {};

    sensor.readAccelerometer(accelerometer);
    sensor.readGyroscope(gyroscope);

    int32_t x_acc = accelerometer[0];
    int32_t y_acc = accelerometer[1];
    int32_t z_acc = accelerometer[2];

    int32_t front_acc = -x_acc;
    int32_t right_acc = y_acc;
    int32_t up_acc = z_acc;

    front_sum += front_acc;
    right_sum += right_acc;
    up_sum += up_acc;

    double front_avg = front_sum/i;
    double right_avg = right_sum/i;
    double up_avg = up_sum/i;

    double roll = -atan(right_avg/up_avg);
    double pitch = atan(front_avg/sqrt(pow(right_avg,2) + pow(up_avg,2)));

  //IN DEGREES
    double roll_degree = roll * 180/3.1415926;
    double pitch_degree = pitch * 180/3.1415926;

    if(i%5 == 0) {
      Serial.print("Roll Degree: "); Serial.println(roll);
      Serial.print("Pitch Degree: "); Serial.println(pitch);
    }

    i ++;
    delay(50);
}

// double toGlobalFront(double front, double right, double up) {
//   return front*cos(PITCH) - up*sin(PITCH);
// }
// double toGlobalRight(double front, double right, double up) {
//   return front*sin(ROLL)*sin(PITCH) + right*cos(ROLL) + up*sin(ROLL)*cos(PITCH);
// }
// double toGlobalUp(double front, double right, double up) {
//   return front*cos(ROLL)*sin(PITCH) - right*sin(ROLL) + up*cos(PITCH)*cos(ROLL);
// }


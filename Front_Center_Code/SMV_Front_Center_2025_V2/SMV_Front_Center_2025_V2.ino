#include <Servo.h>
#include "ADS131M04.h"
#include "SMVcanbus.h"
#include <SPI.h>
#include "RP2040_PWM.h"


//ADC Stuff
ADS131M04 adc;
adcOutput res;
RP2040_PWM* PWM_Instance;
const int CLOCK_PIN = 26;

//Pin init
const int wiper_switch = 12;
const int adc_cs_pin = 25;
const int wiper_pwm = 13;
const int horn_switch = 11;
const int led_1 =27;
const int led_2 = 28;

//Pulse Widths
const int minPulseWidth = 500; // microseconds
const int maxPulseWidth = 2500; // microseconds
const int neutralPulseWidth = 1500; // microseconds

//Servo Vars
unsigned long currentMillis;
const int servoMinAngle = 125;    // Minimum angle
const int servoMaxAngle = 160;  // Maximum angle
int servoPos = servoMinAngle;   // Current position of servo
int servoStep = 5;              // Degrees to move per update (speed control)

//Servo Times
unsigned long previousMillis = 0;
const long interval = 20; // Servo update interval in milliseconds (adjust for speed)

//CAN rec data vars
double wiper_data = 0;
double horn_data = 0;
char* data_type_rec;

//CAN send data
double gas_data = 0;
double brake_data = 0;
double hall1 = 0;
double hall2 = 0;

Servo myServo;
CANBUS can (FC);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  can.begin();
  // //ADC start
  // PWM_Instance = new RP2040_PWM(CLOCK_PIN, 8192000, 50);
  // PWM_Instance->setPWM(CLOCK_PIN, 8192000, 50);
  
  
  // delay(100); // Give the ADC time to recognize the clock
  

  // // -----------------------------------------------------------------------------------------------------------
  // // SHOULD MODIFY PINS TO MATCH YOUR BOARD - if you do not have DRDY, ignore it. DO NOT IGNORE RESET_PIN
  // // -----------------------------------------------------------------------------------------------------------
  // // ORDER: clk_pin, miso_pin, mosi_pin, cs_pin, drdy_pin, reset_pin
  // adc.begin(14, 28, 27, 25, 20, 15);

  
  // // Channel setup
  // adc.setInputChannelSelection(0, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  // adc.setInputChannelSelection(1, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  // adc.setInputChannelSelection(2, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  // adc.setInputChannelSelection(3, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  // adc.setOsr(OSR_1024);      // 32KSPS only with 8MHz clock
  // //ADC end

  // Serial.println("ADC initialized");
  // delay(100);
  pinMode(wiper_switch, OUTPUT);
  digitalWrite(wiper_switch, LOW); //LOW = Off, HIGH = Drive
  pinMode(wiper_pwm, OUTPUT);
  pinMode(led_1, OUTPUT);
  pinMode(led_2, OUTPUT);
  myServo.attach(wiper_pwm, minPulseWidth, maxPulseWidth); // Adjust pulse widths as needed , [500, 2500]
  delay(400);  
}

void loop() {
  can.looper();
  /*if(can.isThere())
  {
    Serial.print("The data is: ");
    Serial.println(can.getData());
    Serial.print("The Hardware Type is: ");
    Serial.println(can.getHardware());
    Serial.print("The Data Type is: ");
    Serial.println(can.getDataType());
    data_rec = can.getData();
  }*/

  data_type_rec = can.getDataType();

  if (strcmp(data_type_rec, "Wipers") == 0){
    wiper_data = can.getData();
  }else if (strcmp(data_type_rec, "Horn") == 0){
    horn_data = can.getData();
  }

  currentMillis = millis();

  if(wiper_data == 1){
    digitalWrite(wiper_switch, HIGH);
    // Check if it's time to update servo position
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      // Move servo incrementally
      myServo.write(servoPos);
      servoPos += servoStep;

      // Reverse direction at limits
      if (servoPos >= servoMaxAngle || servoPos <= servoMinAngle) {
        servoStep = -servoStep; // Reverse direction
      }
    }
  }else if(horn_data == 1){
    digitalWrite(horn_switch, HIGH);
  }else if (wiper_data == 0){
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      // Move servo incrementally
      myServo.write(servoPos);
      servoPos += servoStep;

      // Reverse direction at limits
      if (servoPos > servoMaxAngle || servoPos < servoMinAngle) {
        servoStep = -servoStep; // Reverse direction
      } else if(servoPos == servoMinAngle){
        digitalWrite(wiper_switch, LOW);
      }
    }
  }else if (horn_data == 0){
     digitalWrite(horn_switch, LOW);
  }

 // res = adc.readADC();
  //hall2 = res.ch1;

  // if (hall2 > 0.25){ //hall2: [0.2, 1.09]
  //   brake_data = 1;
  // }else{
  //   brake_data = 0;
  // }

  can.send(gas_data, Gas);
  can.send(brake_data, Brake);

//  //SERVO
//   myServo.write(0); // Minimum position
//  delay(200);
//  myServo.write(90); // Middle position
//  delay(200);
  delay(50);
}
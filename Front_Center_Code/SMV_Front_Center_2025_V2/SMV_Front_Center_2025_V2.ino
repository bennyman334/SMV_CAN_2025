#include "SMVcanbus.h"
#include <string.h>

#include <Arduino.h>
#include "ADS131M04.h"
#include <SPI.h>
#include "RP2040_PWM.h"

#include <Servo.h>

ADS131M04 adc;
adcOutput res;
RP2040_PWM* PWM_Instance;
RP2040_PWM* motor_PWM_1;
// RP2040_PWM* motor_PWM_2;
CANBUS can(FC);
Servo myServo;


//change through testing
const int FORWARD_FASTEST = 3.2*(100/3.3); // percent duty cycle = voltage threshold * max duty cycle/max voltage
const int FORWARD_SLOWEST = 1.7*(100/3.3);
const int REVERSE_SLOWEST = 1.6*(100/3.3);
const int REVERSE_FASTEST = 0.1*(100/3.3);
bool current_reverse = 0; // 0 = forward; 1 = reverse
float duty_cycle_out = 50; // 50% duty cycle = nothing
float pwm_freq = 500000;

double wiper_data = 0;
double horn_data = 0;
double reverse_data = 0;
char* data_type_rec;

const int wiper_switch = 12; //D12
const int horn_switch = 11; //D11
const int wiper_pwm = 13; //D13

//Pulse Widths
const int minPulseWidth = 500; // microseconds
const int maxPulseWidth = 2500; // microseconds
const int neutralPulseWidth = 1500; // microseconds

const int CLOCK_PIN = 26; //CLKIN
const int motor_pwm_pin_1 = 4; //motor pwm output
// const int motor_pwm_pin_2 = 2; //motor pwm output

int servo_buffer = 0;
int servo_buffer_1 = 0;
bool servo_dir = 1;
// ----------------------------------------------------------------------------------
// SETUP CODE
// ----------------------------------------------------------------------------------
void setup()
{
  //Serial.begin(115200);
  // can.begin() MUST be in this location -----------------------------------------------------------------
  can.begin();
  // can.begin() MUST be in this location -----------------------------------------------------------------
  //while (!Serial) delay(10);
  
  // Note: previous version used analogWriteFreq(8192000), analogWrite(CLOCK_PIN, 128) and it seemed to work
  // However, according to the Earle Philhower library, it shouldn't. The following code is its replacement.
  PWM_Instance = new RP2040_PWM(CLOCK_PIN, 8192000, 50);
  PWM_Instance->setPWM(CLOCK_PIN, 8192000, 50);

  motor_PWM_1 = new RP2040_PWM(motor_pwm_pin_1, pwm_freq, duty_cycle_out); //output pin, frequency, duty cycle
  motor_PWM_1->setPWM(motor_pwm_pin_1, pwm_freq, duty_cycle_out);
  // motor_PWM_2 = new RP2040_PWM(motor_pwm_pin_2, pwm_freq, duty_cycle_out);
  // motor_PWM_2->setPWM(motor_pwm_pin_2, pwm_freq, duty_cycle_out);
  
  delay(100); // Give the ADC time to recognize the clock
  
  // -----------------------------------------------------------------------------------------------------------
  // SHOULD MODIFY PINS TO MATCH YOUR BOARD - if you do not have DRDY, ignore it. DO NOT IGNORE RESET_PIN
  // -----------------------------------------------------------------------------------------------------------
  // ORDER: sck_pin, miso_pin, mosi_pin, cs_pin, drdy_pin, reset_pin
  adc.begin(14, 28, 27, 25, 20, 24);

  
  // Channel setup
  adc.setInputChannelSelection(0, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  adc.setInputChannelSelection(1, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  adc.setInputChannelSelection(2, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  adc.setInputChannelSelection(3, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  adc.setOsr(OSR_1024);      // 32KSPS only with 8MHz clock

  Serial.println("ADC initialized");
  pinMode(wiper_switch, OUTPUT);
  pinMode(horn_switch, OUTPUT);
  digitalWrite(wiper_switch, HIGH); //LOW = Off, HIGH = Drive
  pinMode(wiper_pwm, OUTPUT);
  myServo.attach(wiper_pwm, minPulseWidth, maxPulseWidth); // Adjust pulse widths as needed , [500, 2500]
  myServo.write(0);
  digitalWrite(wiper_switch, LOW);
  delay(400);
}

// ----------------------------------------------------------------------------------
// LOOP
// ----------------------------------------------------------------------------------

void loop()
{
  adcOutput res;
  delay(100);

  // -----------------------------------------------------------
  // ADC DATA ACQUISITION AND MANIPULATION
  // -----------------------------------------------------------
  while (1)
  {
    // res.ch0 contains data in channel 0, res.ch1 contains data in channel 1, and so on until res.ch3
    res = adc.readADC();

    // ----------------------------------------------------------------------------------
    // MODIFY BELOW CODE TO MANIPULATE THE ADC'S READINGS AS YOU DESIRE
    // ----------------------------------------------------------------------------------

    // // Example: printing out ADC channel values for channels 0, 1, 2, and 3
//     Serial.print("Status = ");
//     Serial.println(res.status, BIN);
//     Serial.print("CH0 = ");
//     // adc.convert automatically converts the output into floating point voltage values
//     Serial.println(adc.convert(res.ch0));
//     Serial.print("CH1 = ");
//     Serial.println(adc.convert(res.ch1)); //gas pedal 
//     Serial.print("CH2 = ");
//     Serial.println(adc.convert(res.ch2));
//     Serial.print("CH3 = ");
//     Serial.println(adc.convert(res.ch3));
//     Serial.println("");
//     delay(500);

    double brake = adc.convert(res.ch0);
    double gas = adc.convert(res.ch1);

    if(gas == 0) {
      duty_cycle_out = 50;
    } else if(current_reverse == 0) {
      duty_cycle_out = FORWARD_SLOWEST + gas*(FORWARD_FASTEST-FORWARD_SLOWEST)/1.1;
    } else if (current_reverse == 1){
      duty_cycle_out = REVERSE_SLOWEST + -1*gas*(REVERSE_SLOWEST-REVERSE_FASTEST)/1.1;
    }

    motor_PWM_1->setPWM(motor_pwm_pin_1, pwm_freq, duty_cycle_out);
    // motor_PWM_2->setPWM(motor_pwm_pin_2, pwm_freq, duty_cycle_out);

    can.send(brake, Brake);
    can.send(gas, Gas);
    can.looper();
    data_type_rec = can.getDataType();

    if (strcmp(data_type_rec, "Wipers") == 0){
      wiper_data = can.getData();
    } else if (strcmp(data_type_rec, "Horn") == 0){
      horn_data = can.getData();
    } else if (strcmp(data_type_rec, "Reverse") == 0){
      reverse_data = can.getData();
    }

    if (wiper_data == 1){
      digitalWrite(wiper_switch, HIGH);
      if (servo_buffer == 0){
        myServo.write(90*servo_dir);
        servo_dir = !servo_dir;
      }
      servo_buffer += 1;
      servo_buffer = servo_buffer%10;
    } else if (wiper_data == 0){
      if (servo_buffer_1 < 10){
        digitalWrite(wiper_switch, HIGH);
        myServo.write(0);
        servo_buffer_1 += 1;
      } else {
        servo_buffer_1 = 0;
        digitalWrite(wiper_switch, LOW);
      }

    }

    if (horn_data == 1){
      digitalWrite(horn_switch, HIGH);
    } else{
      digitalWrite(horn_switch, LOW);
    }

    if (reverse_data == 0){ // UI board reverse switch unflipped
      current_reverse = 0;
    } else if (reverse_data == 1){ // UI board reverse switch flipped
      current_reverse = 1;
    }

    // Call adc.begin() at the end of the loop to counteract CAN-ADC bug
    // DO NOT REMOVE
    delay(50);
    adc.begin(14, 28, 27, 25, 20, 24);
    delay(50);

    // -----------------------------------------------------------
    // DIAGNOSTIC PRINTS
    // -----------------------------------------------------------
    // Serial.println(adc.readRegister(REG_STATUS), BIN);
    // delay(500);
    // -----------------------------------------------------------
  }
}

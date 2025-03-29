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
CANBUS can(FC);
Servo myServo;

double wiper_data = 0;
double horn_data = 0;
char* data_type_rec;


const int wiper_switch = 12;
const int horn_switch = 11;
const int wiper_pwm = 13;

//Pulse Widths
const int minPulseWidth = 500; // microseconds
const int maxPulseWidth = 2500; // microseconds
const int neutralPulseWidth = 1500; // microseconds


const int CLOCK_PIN = 26;

int servo_buffer = 0;
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
  
  
  delay(100); // Give the ADC time to recognize the clock
  

  // -----------------------------------------------------------------------------------------------------------
  // SHOULD MODIFY PINS TO MATCH YOUR BOARD - if you do not have DRDY, ignore it. DO NOT IGNORE RESET_PIN
  // -----------------------------------------------------------------------------------------------------------
  // ORDER: clk_pin, miso_pin, mosi_pin, cs_pin, drdy_pin, reset_pin
  adc.begin(14, 28, 27, 25, 20, 24);

  
  // Channel setup
  adc.setInputChannelSelection(0, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  adc.setInputChannelSelection(1, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  adc.setInputChannelSelection(2, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  adc.setInputChannelSelection(3, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  adc.setOsr(OSR_1024);      // 32KSPS only with 8MHz clock

  Serial.println("ADC initialized");
  pinMode(wiper_switch, OUTPUT);
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
    // Serial.print("Status = ");
    // Serial.println(res.status, BIN);
    // Serial.print("CH0 = ");
    // // adc.convert automatically converts the output into floating point voltage values
    // Serial.println(adc.convert(res.ch0));
    // Serial.print("CH1 = ");
    // Serial.println(adc.convert(res.ch1));
    // Serial.print("CH2 a= ");
    // Serial.println(adc.convert(res.ch2));
    // Serial.print("CH3 = ");
    // Serial.println(adc.convert(res.ch3));
    // Serial.println("");
    // delay(500);

    double data = adc.convert(res.ch1);
    
    can.send(data, Brake);
    can.looper();
    data_type_rec = can.getDataType();

    if (strcmp(data_type_rec, "Wipers") == 0){
      wiper_data = can.getData();
    }else if (strcmp(data_type_rec, "Horn") == 0){
      horn_data = can.getData();
    }

    if (wiper_data == 1){
      digitalWrite(wiper_switch, HIGH);
      if (servo_buffer == 0){
        myServo.write(180*servo_dir);
        servo_dir = !servo_dir;
      }
      servo_buffer += 1;
      servo_buffer = servo_buffer%10;
      } else if (wiper_data == 0){
      myServo.write(0);
      delay(10);
      digitalWrite(wiper_switch, LOW);
    }

    if (horn_data == 1){
      digitalWrite(horn_switch, HIGH);
    }else{
      digitalWrite(horn_switch, LOW);
    }
    delay(100);

    // Call adc.begin() at the end of the loop to counteract CAN-ADC bug
    // DO NOT REMOVE
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
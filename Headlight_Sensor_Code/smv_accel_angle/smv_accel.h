#ifndef SMV_ACCEL_H
#define SMV_ACCEL_H

#include <Arduino.h>
#include <SPI.h>
#include <ASM330LHHSensor.h>

class ASM330LHH {
public:
    ASM330LHH(int csPin);
    void begin();
    void readAccelerometer(int32_t *accelData);
    void readGyroscope(int32_t *gyroData);
    void printSensorData();

private:
    int _csPin;
    ASM330LHHSensor *_sensor;
};

#endif
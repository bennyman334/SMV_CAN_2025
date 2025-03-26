// Credit: https://github.com/stm32duino/ASM330LHH/blob/main/examples/ASM330LHH_SPI_HelloWorld/ASM330LHH_SPI_HelloWorld.ino

#include "smv_accel.h"

// Define standard SPI object for RP2040
#define dev_spi SPI  

ASM330LHH::ASM330LHH(int csPin) : _csPin(csPin) {
    _sensor = new ASM330LHHSensor(&dev_spi, _csPin);
}

void ASM330LHH::begin() {
    Serial.begin(115200);
    SPI.setSCK(14);
    SPI.setRX(28);
    SPI.setTX(27);
    SPI.setCS(13);
    dev_spi.begin();
    _sensor->begin();
    _sensor->Enable_X(); // Enable accelerometer
    _sensor->Enable_G(); // Enable gyroscope
}

// -----------------------------------------------------------------------------------------------
// READ ACCELEROMETER DATA
// -----------------------------------------------------------------------------------------------
void ASM330LHH::readAccelerometer(int32_t *accelData) {
    _sensor->Get_X_Axes(accelData);
}
// -----------------------------------------------------------------------------------------------
// END BLOCK
// -----------------------------------------------------------------------------------------------


// -----------------------------------------------------------------------------------------------
// READ GYROSCOPE DATA
// -----------------------------------------------------------------------------------------------
void ASM330LHH::readGyroscope(int32_t *gyroData) {
    _sensor->Get_G_Axes(gyroData);
}
// -----------------------------------------------------------------------------------------------
// END BLOCK
// -----------------------------------------------------------------------------------------------


// -----------------------------------------------------------------------------------------------
// PRINT DATA
// -----------------------------------------------------------------------------------------------
void ASM330LHH::printSensorData() {
    int32_t accelerometer[3] = {};
    int32_t gyroscope[3] = {};

    readAccelerometer(accelerometer);
    readGyroscope(gyroscope);

    Serial.print("ASM330LHH: | Acc[mg]: ");
    Serial.print(accelerometer[0]); Serial.print(" ");
    Serial.print(accelerometer[1]); Serial.print(" ");
    Serial.print(accelerometer[2]); Serial.print(" | Gyr[mdps]: ");
    Serial.print(gyroscope[0]); Serial.print(" ");
    Serial.print(gyroscope[1]); Serial.print(" ");
    Serial.print(gyroscope[2]); Serial.println(" |");
}
// -----------------------------------------------------------------------------------------------
// END BLOCK
// -----------------------------------------------------------------------------------------------
#include "drivers/BMPSensor.h"

#include "SysHead.h"
#include "Wire.h"

BMPSensor::BMPSensor() : Sensor("BMP") { bmp = Adafruit_BMP280(&Wire); }

// i2c addr default 0x77, alt 0x76
// I2C device found at address 0x38
// I2C device found at address 0x77
bool BMPSensor::verify() { return bmp.begin(0x77); }

void BMPSensor::readToSysVar() {
  BMPSensorData sensor_data;

  sensor_data.BMPTemp = bmp.readTemperature();
  sensor_data.BMPPressure = bmp.readPressure();

  sysvar_set_bmp_data(&sensor_data);
}
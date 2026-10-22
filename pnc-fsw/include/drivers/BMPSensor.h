#ifndef BMP_SENSOR_H
#define BMP_SENSOR_H

#include <Adafruit_BMP280.h>  
#include "Sensor.h"
#include "Wire.h"

struct __attribute__((packed)) BMPSensorData {
  double BMPTemp;       
  uint32_t BMPPressure;
};

/**
 * @brief Implementation of a Sensor for the onboard temperature sensor
 *
 */
class BMPSensor : public Sensor {
 private:
  Adafruit_BMP280 bmp;

 public:
  BMPSensor();
  bool verify() override;
  void readToSysVar() override;
};

#endif

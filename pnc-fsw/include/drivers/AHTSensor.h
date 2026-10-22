#ifndef AHT_SENSOR_H
#define AHT_SENSOR_H

#include <Adafruit_AHTX0.h> 
#include "Sensor.h"
#include "Wire.h"

struct __attribute__((packed)) AHTSensorData {
  float AHTTemp;       
  float AHTHumidity;
};

/**
 * @brief Implementation of a Sensor for the onboard temperature sensor
 *
 */
class AHTSensor : public Sensor {
 private:
  Adafruit_AHTX0 aht;

 public:
  AHTSensor();
  bool verify() override;
  void readToSysVar() override;
};

#endif


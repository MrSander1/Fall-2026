#include "drivers/AHTSensor.h"

#include "SysHead.h"
#include "Wire.h"

AHTSensor::AHTSensor() : Sensor("AHT") { aht = Adafruit_AHTX0(); }

// i2c addr default 0x77, alt 0x76
// I2C device found at address 0x38
// I2C device found at address 0x77
bool AHTSensor::verify() { return aht.begin(); }

void AHTSensor::readToSysVar() {
  AHTSensorData sensor_data;
  sensors_event_t temp_event;
  sensors_event_t humidity_event; 

  aht.getTemperatureSensor()->getEvent(&temp_event);
  aht.getHumiditySensor()->getEvent(&humidity_event);

  sensor_data.AHTTemp = temp_event.temperature;
  sensor_data.AHTHumidity = humidity_event.relative_humidity;

  sysvar_set_aht_data(&sensor_data);
}
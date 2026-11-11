#include "BME688Sensor.h"

#include "PayloadConfig.h"

/**
 * @brief Construct a new BME688 Sensor object with default minimum_period of 0
 * ms
 *
 */
BME688Sensor::BME688Sensor(TwoWire* i2c_bus) : BME688Sensor(0, i2c_bus) {
}

/**
 * @brief Construct a new BME688 Sensor object
 *
 * @param minimum_period Minimum time to wait between readings in ms
 */
BME688Sensor::BME688Sensor(unsigned long minimum_period, TwoWire* i2c_bus)
    : Sensor("BME688", "BME TempC,BME PresPa,BME Humidity,BME GasOhm,",
             minimum_period),
      bme(i2c_bus) {
  this->i2c_bus = i2c_bus;
  this->i2c_addr = BME688_I2C_ADDR;
  if (this->i2c_bus == &Wire1) this->device_name += "_1";
}

/**
 * @brief Returns if the BME688 sensor can be reached
 *
 * @return true
 * @return false
 */
bool BME688Sensor::verify() {
  this->i2c_bus->begin();
  bool verified = this->bme.begin(this->i2c_addr);

  if (!verified) {
    uint8_t alternate_addr = (this->i2c_addr == 0x77) ? 0x76 : 0x77;
    verified = this->bme.begin(alternate_addr);
    if (verified) {
      this->i2c_addr = alternate_addr;
    }
  }

  log_core(this->device_name +
           (verified ? " verify success at 0x" + String(this->i2c_addr, HEX)
                     : " verify failed"));

  return verified;
}

/**
 * @brief Reads temperature, pressure, humidity, and gas resistance from BME688
 *
 * @return String CSV stub of temperature, pressure, humidity, and gas values
 */
String BME688Sensor::readData() {
  if (!this->bme.performReading()) {
    return this->readEmpty();
  }

  const float temp = this->bme.temperature;
  const uint32_t pressure = (uint32_t)this->bme.pressure;
  const float humidity = this->bme.humidity;
  const uint32_t gas_resistance = this->bme.gas_resistance;

  return String(temp, 5) + "," + String(pressure) + "," + String(humidity, 5) +
         "," + String(gas_resistance) + ",";
}

/**
 * @brief Reads BME688 data and appends it to packet
 *
 * @param packet Pointer to the packet byte array
 */
void BME688Sensor::readDataPacket(uint8_t*& packet) {
  if (!this->bme.performReading()) {
    return;
  }

  const float temp = this->bme.temperature;
  const uint32_t pressure = (uint32_t)this->bme.pressure;
  const float humidity = this->bme.humidity;
  const uint32_t gas_resistance = this->bme.gas_resistance;

  memcpy(packet, &temp, sizeof(temp));
  packet += sizeof(temp);

  memcpy(packet, &pressure, sizeof(pressure));
  packet += sizeof(pressure);

  memcpy(packet, &humidity, sizeof(humidity));
  packet += sizeof(humidity);

  memcpy(packet, &gas_resistance, sizeof(gas_resistance));
  packet += sizeof(gas_resistance);
}

/**
 * @brief Decodes BME688 data from packet to CSV
 *
 * @param packet Pointer to packet bytes
 * @return String CSV stub of temperature, pressure, humidity, and gas values
 */
String BME688Sensor::decodeToCSV(uint8_t*& packet) {
  float temp = 0;
  uint32_t pressure = 0;
  float humidity = 0;
  uint32_t gas_resistance = 0;

  memcpy(&temp, packet, sizeof(temp));
  packet += sizeof(temp);

  memcpy(&pressure, packet, sizeof(pressure));
  packet += sizeof(pressure);

  memcpy(&humidity, packet, sizeof(humidity));
  packet += sizeof(humidity);

  memcpy(&gas_resistance, packet, sizeof(gas_resistance));
  packet += sizeof(gas_resistance);

  return String(temp, 5) + "," + String(pressure) + "," + String(humidity, 5) +
         "," + String(gas_resistance) + ",";
}

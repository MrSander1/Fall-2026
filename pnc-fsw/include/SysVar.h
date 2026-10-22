#ifndef SYS_VAR_H
#define SYS_VAR_H

#include <Arduino.h>
#include <drivers/BMESensor.h>
#include <drivers/AHTSensor.h>
#include <drivers/GPSSensor.h>
#include <drivers/INASensor.h>
#include <drivers/BMPSensor.h>

// setup
void sysvar_init();

// Access functions
int8_t sysvar_get_pico_temp_c(float* output);
int8_t sysvar_set_pico_temp_c(float input);

int8_t sysvar_get_rtc_time(uint32_t* output);
int8_t sysvar_set_rtc_time(uint32_t input);

int8_t sysvar_set_ina_data(INASensorData* ina_sensor_data);
int8_t sysvar_get_ina_data(INASensorData* ina_sensor_data);

int8_t sysvar_set_bme_data(BMESensorData* sensor_data);
int8_t sysvar_get_bme_data(BMESensorData* sensor_data);

int8_t sysvar_set_bmp_data(BMPSensorData* sensor_data);
int8_t sysvar_get_bmp_data(BMPSensorData* sensor_data);

int8_t sysvar_set_aht_data(AHTSensorData* sensor_data);
int8_t sysvar_get_aht_data(AHTSensorData* sensor_data);

int8_t sysvar_set_gps_data(GPSSensorData* sensor_data);
int8_t sysvar_get_gps_data(GPSSensorData* sensor_data);

#endif  // SYS_VAR_H

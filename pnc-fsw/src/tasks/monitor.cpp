#include "tasks/monitor.h"

#include <drivers/BMESensor.h>
#include <drivers/BMPSensor.h>
#include <drivers/GPSSensor.h>
#include <drivers/INASensor.h>

#include "SysHead.h"
#include "tasks/watchdog.h"

void monitor_task() {
  watchdog_intertask_update(WATCHDOG_MONITOR_TASK_ID);

  float pico_temp_c;
  int8_t res = sysvar_get_pico_temp_c(&pico_temp_c);

  log_task("Pico Temp: " + String(pico_temp_c) + "(" + String(res) + ")");

  uint32_t rtc_time;
  res = sysvar_get_rtc_time(&rtc_time);
  log_task("RTC Time: " + String(rtc_time) + "(" + String(res) + ")");

  BMPSensorData bmp_data;
  res = sysvar_get_bmp_data(&bmp_data);
  log_task("BMP Temperature: " + String(bmp_data.BMPTemp) + "(" + String(res) +
           ")");
  log_task("BMP Pressure: " + String(bmp_data.BMPPressure) + "(" + String(res) +
           ")");
           
  BMESensorData bme_data;
  res = sysvar_get_bme_data(&bme_data);
  log_task("BME Humidity: " + String(bme_data.BMEHumidity) + "(" + String(res) +
           ")");
  log_task("BME Gas Sensor: " + String(bme_data.BMEGasResistance) + "(" +
           String(res) + ")");

  AHTSensorData aht_data;
  res = sysvar_get_aht_data(&aht_data);
  log_task("AHT Temperature: " + String(aht_data.AHTTemp) + "(" + String(res) + ")");
  log_task("AHT Humidity: " + String(aht_data.AHTHumidity) + "(" + String(res) + ")");

  GPSSensorData gps_data;
  res = sysvar_get_gps_data(&gps_data);
  log_task("GPS Unix Time: " + String(gps_data.unix_time_s) + "(" +
           String(res) + ")");
  log_task("GPS Fix Type: " + String(gps_data.fix_type) + "(" + String(res) +
           ")");
  log_task("GPS Fix OK: " + String(gps_data.fix_ok) + "(" + String(res) + ")");
  log_task("GPS SIV: " + String(gps_data.siv) + "(" + String(res) + ")");
  log_task("GPS Lat E7: " + String(gps_data.lat_e7) + "(" + String(res) + ")");
  log_task("GPS Lon E7: " + String(gps_data.lon_e7) + "(" + String(res) + ")");
  log_task("GPS Alt MSL mm: " + String(gps_data.alt_msl_mm) + "(" +
           String(res) + ")");
  log_task("GPS Vel N mmps: " + String(gps_data.vel_n_mmps) + "(" +
           String(res) + ")");
  log_task("GPS Vel E mmps: " + String(gps_data.vel_e_mmps) + "(" +
           String(res) + ")");
  log_task("GPS Vel D mmps: " + String(gps_data.vel_d_mmps) + "(" +
           String(res) + ")");
  log_task("GPS HAcc mm: " + String(gps_data.hacc_mm) + "(" + String(res) +
           ")");
  log_task("GPS VAcc mm: " + String(gps_data.vacc_mm) + "(" + String(res) +
           ")");

  INASensorData ina_data;
  res = sysvar_get_ina_data(&ina_data);
  log_task("INA Current: " + String(ina_data.INACurrent) + "(" + String(res) +
           ")");
  log_task("INA Bus Voltage: " + String(ina_data.INABusVoltage) + "(" +
           String(res) + ")");
  log_task("INA Power: " + String(ina_data.INAPower) + "(" + String(res) + ")");
}

void monitor_task_init() {
  // do task setup

  log_task("monitor Task Started.");
}

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>


#include "ErrorDisplay.h"
#include "RadiacodeBLE.h"
#include "SysHead.h"
#include "pico/multicore.h"

// tasks
#include "tasks/monitor.h"
#include "tasks/watchdog.h"


// sensors
#include "drivers/BMESensor.h"
#include "drivers/BMPSensor.h"
#include "drivers/AHTSensor.h"
#include "drivers/GPSSensor.h"
#include "drivers/INASensor.h"
#include "drivers/PicoTempSensor.h"
#include "drivers/RTCSensor.h"
#include "drivers/Sensor.h"

// Todo: create a new sensor driver for BMP280 + AHT20

// create a sensor class for bmp280 and AHT20

PicoTempSensor pico_temp_sensor;
RTCSensor rtc_sensor;
BMESensor bme_sensor;
GPSSensor gps_sensor;
INASensor ina_sensor;
BMPSensor bmp_sensor;
AHTSensor aht_sensor;

// add it to the sensors array

Sensor* sensors[] = {
                    &pico_temp_sensor, 
                    &rtc_sensor, 
                    &bmp_sensor, 
                    &ina_sensor,
                    &gps_sensor,
                    &aht_sensor
                    };
const size_t sensors_len = sizeof(sensors) / sizeof(sensors[0]);
bool found_sensors[sensors_len];

String rc_target_mac = "52:43:06:60:17:DD";
String rc_filename = "";

// declarations
void save_radiacode_data();
void sysvar_update();
void store_data();
void sd_setup();
bool sd_status = false;
String filename = "";


// add the sensor data to the packet struct for BMP280 and AHT20
struct __attribute__((packed)) Packet {
  uint32_t sync_bytes = 0xDEADCAFE;
  uint32_t uptime;
  uint8_t id = 0;
  uint8_t length = sizeof(Packet);
  // uint8_t length = 3 * sizeof(uint32_t) + 3 * sizeof(uint8_t) + sizeof(float) +
  //                  sizeof(BMESensorData) + sizeof(INASensorData) +
  //                  sizeof(GPSSensorData) + sizeof(BMPSensorData) + sizeof(int8_t) + sizeof(BMPSesnorData) + sizeof(); // not sure if sizeof(int8_t) is needed, but just to be safe
  float temp_data;
  uint32_t rtc_time;
  BMESensorData bme_data;
  BMPSensorData bmp_data;
  INASensorData ina_data;
  GPSSensorData gps_data;
  AHTSensorData aht_data;
  int8_t checksum;
};

void setup() {
  sysvar_init();       // setup sysvar protection
  watchdog_disable();  // wait for setup later
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  // while (!Serial) delay(100);

  // Initialize I2C bus once before verifying any I2C sensors.
  Wire.begin();

  log_task("ASCEND PnC FSW");

  // sensor setups
  for (int i = 0; i < sensors_len; i++) {
    log_task("Verifying " + sensors[i]->getSensorName() + "...");
    if (sensors[i]->verify()) {
      found_sensors[i] = true;
      log_task("Success.");
    } else {
      found_sensors[i] = false;
      log_task("Failure.");
    }
  }

  // sd setup
  sd_setup();

  // radiacode setup not need right now
  radiacode_ble_init();
  uint8_t res = radiacode_ble_connect(rc_target_mac, true);
  if (res != 0) {
    watchdog_enable(100, true);
    while (1);  // trigger a reboot
  }

  log_task("Setup complete.");
}

void sd_setup() {
  if (SD.begin(SD_PIN)) {
    sd_status = true;
    int num = 0;
    while (SD.exists("data" + String(num) + ".bin")) num++;

    filename = "data" + String(num) + ".bin";
    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
      ErrorDisplay::instance().addCode(Error::SD_CARD_FAIL);
      sd_status = false;
      SD.end();
      return;
    }
    log_task("Saving to " + filename);
    file.close();

    rc_filename = "rc" + String(num) + ".txt";
    File rc_file = SD.open(rc_filename, FILE_WRITE);
    if (!rc_file) {
      ErrorDisplay::instance().addCode(Error::SD_CARD_FAIL);
      sd_status = false;
      SD.end();
      return;
    }
    rc_file.close();
    log_task("Saving to " + rc_filename);
  } else {
    ErrorDisplay::instance().addCode(Error::SD_CARD_FAIL);
  }
}

static uint8_t it = 0;
void loop() {
  digitalWrite(LED_BUILTIN, it & 0x1);
  it++;

  watchdog_intertask_update(WATCHDOG_MAIN_TASK_ID);
  sysvar_update();
  store_data();
  // not needed for now, but can be used to save radiacode data if needed
  //save_radiacode_data();

  delay(200);
}

static int spectrum[1024];  // to not be on stack
void save_radiacode_data() {
  static uint32_t last_spectrum = 0;
  static uint8_t fails = 0;

  log_task("save_radiacode_data");

  if (sd_status == false) {
    sd_setup();
    return;
  }

  if (fails > 10) {
    log_task("More than 10 fails, restarting...");
    while (1);
  }

  // get event data
  BytesBuffer* r = read_request(VS::DATA_BUF);

  if (r != nullptr) {
    File fout = SD.open(rc_filename, FILE_WRITE);
    while (r->size() >= 7) {
      DataPoint d = consume_data_buf(r);

      std::visit(
          [&fout](const auto& v) {
            char str[500];
            int len = v.to_string(str, 500);
            log_task_printf("%lu,%d,", millis(), len);
            log_printf("%s\n", str);
            fout.printf("%lu,%s\n", millis(), str);
          },
          d);
    }
    fout.close();
  } else {
    fails++;
  }

  if (millis() - last_spectrum > 30000) {  // 30s
    log_task("Reading spectrum");
    last_spectrum = millis();
    BytesBuffer* spec_buf = readSpectrumData();

    if (spec_buf == nullptr) {
      fails++;
      return;
    }

    File fout = SD.open(rc_filename, FILE_WRITE);

    float a0, a1, a2;
    uint32_t ts;
    decode_spectrum(spec_buf, spectrum, a0, a1, a2, ts);

    log_task_printf("a0: %f, a1: %f, a2: %f, ts: %u\n", a0, a1, a2, ts);
    fout.printf("a0: %f, a1: %f, a2: %f, ts: %u\n", a0, a1, a2, ts);

    log_task_printf("Spectrum: ");
    fout.printf("Spectrum: ");

    for (int i = 0; i < 1024; i++) {
      log_printf("%d, ", spectrum[i]);
      fout.printf("%d, ", spectrum[i]);
    }
    log_printf("\n");
    fout.printf("\n");

    fout.close();
  }
}

void sysvar_update() {
  // update pico temp
  log_task("Starting SysVar Update...");

  for (int i = 0; i < sensors_len; i++) {
    if (found_sensors[i]) {
      sensors[i]->readToSysVar();
    }
  }

  log_task("Done.");
}

void store_data() {
  log_task("store_data");
  if (sd_status == false) {
    sd_setup();
    return;
  }

  float pico_temp_c;
  uint32_t rtc_time;
  BMESensorData bme_data;
  BMPSensorData bmp_data;
  AHTSensorData aht_data;
  INASensorData ina_data;
  GPSSensorData gps_data;
  Packet packet;

  int8_t sum = 0;

  sysvar_get_pico_temp_c(&pico_temp_c);
  sysvar_get_rtc_time(&rtc_time);
  sysvar_get_bme_data(&bme_data);
  sysvar_get_ina_data(&ina_data);
  sysvar_get_gps_data(&gps_data);
  sysvar_get_bmp_data(&bmp_data);
  sysvar_get_aht_data(&aht_data);

  packet.uptime = millis();
  packet.temp_data = pico_temp_c;
  packet.rtc_time = rtc_time;
  packet.bme_data = bme_data;
  packet.aht_data = aht_data;
  packet.ina_data = ina_data;
  packet.gps_data = gps_data;
  packet.bmp_data = bmp_data;

  uint8_t* pos = (uint8_t*)&packet;

  for (size_t i = 0; i < packet.length; i++) {
    sum += *pos;
    pos++;
  }
  packet.checksum = -sum;  // calculate checksum with sum complement parity
  File output = SD.open(filename, FILE_WRITE);
  if (!output) {
    ErrorDisplay::instance().addCode(Error::SD_CARD_FAIL);
    sd_status = false;
    SD.end();
  }
  output.write((uint8_t*)&packet, packet.length);
  output.close();
  log_task("Done.");
}

extern "C" bool core1_separate_stack = true;
// core 1
// ---------------------------------------------------------------------------------------------
// monitor and watchdog
void setup1() {
  // start watchdog and monitor tasks after ble is set up
  delay(20000);

  monitor_task_init();
  watchdog_task_init();
}

void loop1() {
  watchdog_task();
  monitor_task();
  delay(1000);
}
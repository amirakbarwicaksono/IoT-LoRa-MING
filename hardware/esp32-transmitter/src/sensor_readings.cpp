#include "sensor_readings.h"
#include <EEPROM.h>
#include "sensors_config.h"


DHT dht(DHTPIN, DHTTYPE);

// Pin analog MQ sensor
#define PIN_MQ135 34
#define PIN_MQ7   35
#define PIN_MQ9   32

// RL values
#define RL_MQ135 10.0
#define RL_MQ7   10.0
#define RL_MQ9   10.0

// Clean air ratios
#define MQ135_CLEAN_AIR 3.6
#define MQ7_CLEAN_AIR   27.0
#define MQ9_CLEAN_AIR   9.6

// Curves [a, b, c]
const float MQ135_CURVE[3] = {116.6020682, 2.769034857, 0.0};
const float MQ7_CURVE[3]   = {99.042, -1.518, 0.0};
const float MQ9_CURVE[3]   = {100.0, -2.0, 0.0};

SensorData readAllSensors() {
  SensorData data;
  EEPROM.begin(512);

  float ro135, ro7, ro9;
  EEPROM.get(0, ro135);
  EEPROM.get(sizeof(float), ro7);
  EEPROM.get(2 * sizeof(float), ro9);

  data.mq135_ppm = mq_read_ppm(PIN_MQ135, RL_MQ135, ro135, MQ135_CURVE);
  data.mq7_ppm   = mq_read_ppm(PIN_MQ7,   RL_MQ7,   ro7,   MQ7_CURVE);
  data.mq9_ppm   = mq_read_ppm(PIN_MQ9,   RL_MQ9,   ro9,   MQ9_CURVE);

  data.temperature = dht.readTemperature();
  data.humidity    = dht.readHumidity();

  return data;
}

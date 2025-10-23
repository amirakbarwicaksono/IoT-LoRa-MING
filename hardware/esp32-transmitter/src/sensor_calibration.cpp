#include "sensor_calibration.h"
#include "sensor_readings.h"
#include <EEPROM.h>
#include "sensors_config.h"


extern DHT dht;  // gunakan instance dari sensor_readings.cpp

void calibrateSensors() {
  Serial.println("\n[Calibration] Starting MQ sensor calibration...");

  // Inisialisasi EEPROM
  EEPROM.begin(512);

  float ro135 = calibrateMQ(PIN_MQ135, RL_MQ135, MQ135_CLEAN_AIR);
  float ro7   = calibrateMQ(PIN_MQ7,   RL_MQ7,   MQ7_CLEAN_AIR);
  float ro9   = calibrateMQ(PIN_MQ9,   RL_MQ9,   MQ9_CLEAN_AIR);

  EEPROM.put(0, ro135);
  EEPROM.put(sizeof(float), ro7);
  EEPROM.put(2 * sizeof(float), ro9);
  EEPROM.commit();

  Serial.println("[Calibration] Saved calibration to EEPROM.");
  Serial.printf("Ro135=%.2f | Ro7=%.2f | Ro9=%.2f\n\n", ro135, ro7, ro9);
}

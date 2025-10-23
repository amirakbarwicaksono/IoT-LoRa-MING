#ifndef SENSOR_READINGS_H
#define SENSOR_READINGS_H

#include <Arduino.h>
#include <DHT.h>
#include "mq_sensors.h"

#define DHTPIN  27
#define DHTTYPE DHT22

extern DHT dht;

struct SensorData {
  float mq135_ppm;
  float mq7_ppm;
  float mq9_ppm;
  float temperature;
  float humidity;
};

SensorData readAllSensors();

#endif

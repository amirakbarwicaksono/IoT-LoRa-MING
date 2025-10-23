#ifndef MQ_SENSORS_H
#define MQ_SENSORS_H

#include <Arduino.h>

float mq_read_ppm(int pin, float rl_value, float ro_value, const float curve[3]);
float calibrateMQ(int pin, float rl_value, float clean_air_factor);

#endif

#include "mq_sensors.h"

float mq_resistance(int raw_adc, float rl_value) {
  return ((4095.0 / raw_adc) - 1.0) * rl_value;
}

float mq_read_ppm(int pin, float rl_value, float ro_value, const float curve[3]) {
  int raw_adc = analogRead(pin);
  float rs = mq_resistance(raw_adc, rl_value);
  float ratio = rs / ro_value;
  float ppm = curve[0] * pow(ratio, -curve[1]);
  return ppm;
}

float calibrateMQ(int pin, float rl_value, float clean_air_factor) {
  long val = 0;
  for (int i = 0; i < 50; i++) {
    val += analogRead(pin);
    delay(100);
  }
  val /= 50;
  float rs = mq_resistance(val, rl_value);
  float ro = rs / clean_air_factor;
  return ro;
}

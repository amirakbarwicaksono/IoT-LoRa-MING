#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// --- Deklarasi fungsi utama ---
void setupNetwork();
void mqttLoop();
void mqttPublish(const char* topic, const String &message);

#endif

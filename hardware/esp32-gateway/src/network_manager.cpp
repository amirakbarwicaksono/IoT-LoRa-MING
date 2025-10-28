//Added INF Handle.
// #include "network_manager.h"

// // ---------- Konfigurasi WiFi ----------
// const char* ssid = "FTI-USAKTI";
// const char* password = "trisakti2026";

// // ---------- Konfigurasi MQTT ----------
// const char* mqtt_server = "10.24.80.89";
// const char* mqtt_user = "admin";
// const char* mqtt_pass = "gracias, idiota";

// WiFiClient espClient;
// PubSubClient client(espClient);

// // ---------- Setup WiFi ----------
// void setupWifi() {
//   delay(10);
//   Serial.println();
//   Serial.print("[WiFi] Connecting to ");
//   Serial.println(ssid);

//   WiFi.mode(WIFI_STA);
//   WiFi.begin(ssid, password);
//   int retry = 0;

//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//     if (++retry > 40) {
//       Serial.println("\n[WiFi] Connection failed!");
//       return;
//     }
//   }

//   Serial.println("");
//   Serial.println("[WiFi] Connected!");
//   Serial.print("[WiFi] IP: ");
//   Serial.println(WiFi.localIP());
// }

// // ---------- Reconnect MQTT ----------
// void reconnect() {
//   while (!client.connected()) {
//     Serial.print("[MQTT] Attempting connection...");
//     String clientId = "ESP32Gateway-";
//     clientId += String(random(0xffff), HEX);
//     if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
//       Serial.println("connected");
//       client.publish("lora/status", "ESP32 Gateway Connected");
//     } else {
//       Serial.print("failed, rc=");
//       Serial.print(client.state());
//       Serial.println(" retry in 5s");
//       delay(5000);
//     }
//   }
// }

// // ---------- Public Functions ----------
// void setupNetwork() {
//   setupWifi();
//   client.setServer(mqtt_server, 1883);
// }

// void mqttLoop() {
//   if (!client.connected()) {
//     reconnect();
//   }
//   client.loop();
// }

// void mqttPublish(const char* topic, const String &message) {
//   if (client.publish(topic, message.c_str())) {
//     Serial.printf("[MQTT] Published to %s\n", topic);
//   } else {
//     Serial.println("[MQTT] ❌ Publish failed!");
//   }
// }
#include "network_manager.h"

// ---------- Konfigurasi WiFi ----------
const char* ssid = "FTI-USAKTI";
const char* password = "trisakti2026";

// ---------- Konfigurasi MQTT ----------
const char* mqtt_server = "10.24.80.89";
const char* mqtt_user = "admin";
const char* mqtt_pass = "gracias, idiota";

WiFiClient espClient;
PubSubClient client(espClient);

// ---------- Fungsi bantu ----------
float safeFloat(float value) {
  // Hapus nilai tidak valid (inf, NaN, dll)
  if (isnan(value) || isinf(value) || value > 1e6 || value < -1e6) {
    return 0.0f;
  }
  return value;
}

// ---------- Setup WiFi ----------
void setupWifi() {
  Serial.println();
  Serial.print("[WiFi] Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int retry = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++retry > 40) {
      Serial.println("\n[WiFi] ❌ Connection failed!");
      return;
    }
  }

  Serial.println("\n[WiFi] ✅ Connected!");
  Serial.print("[WiFi] IP: ");
  Serial.println(WiFi.localIP());
}

// ---------- Reconnect MQTT ----------
void reconnect() {
  while (!client.connected()) {
    Serial.print("[MQTT] Connecting...");
    String clientId = "ESP32Gateway-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println(" connected!");
      client.publish("lora/status", "ESP32 Gateway Connected");
    } else {
      Serial.printf(" failed (rc=%d), retrying in 5s...\n", client.state());
      delay(5000);
    }
  }
}

// ---------- Public Functions ----------
void setupNetwork() {
  setupWifi();
  client.setServer(mqtt_server, 1883);
}

void mqttLoop() {
  if (!client.connected()) reconnect();
  client.loop();
}

// ---------- Publikasi MQTT ----------
void mqttPublish(const char* topic, const String &message) {
  // Pastikan JSON valid (hapus inf/nan, ubah 0.00 ke 0)
  String sanitized = message;

  sanitized.replace("inf", "0");
  sanitized.replace("nan", "0");
  sanitized.replace("NaN", "0");
  sanitized.replace("INF", "0");
  sanitized.replace("0.00", "0");
  sanitized.replace(",,", ",");

  // Pastikan kurung tutup JSON ada
  if (!sanitized.endsWith("}")) {
    sanitized += "}";
  }

  if (client.publish(topic, sanitized.c_str())) {
    Serial.printf("[MQTT] Published to %s: %s\n", topic, sanitized.c_str());
  } else {
    Serial.println("[MQTT] ❌ Publish failed!");
  }
}

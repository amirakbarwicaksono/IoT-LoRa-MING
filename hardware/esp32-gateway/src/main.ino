// //Gateway Code for ESP32 LoRa to MQTT
// #include <Arduino.h>
// #include <SPI.h>
// #include <LoRa.h>
// #include <WiFi.h>
// #include <PubSubClient.h>

// #define LORA_SS 18
// #define LORA_RST 14
// #define LORA_DIO0 26
// #define LORA_BAND 433E6

// // --- WiFi & MQTT config ---
// const char* ssid = "YOUR_WIFI_SSID";
// const char* password = "YOUR_WIFI_PASSWORD";

// const char* mqtt_server = "RASPBERRY_PI_IP"; // e.g. 192.168.1.10
// const int mqtt_port = 1883;
// const char* mqtt_user = "admin";
// const char* mqtt_pass = "gracias, idiota";

// WiFiClient espClient;
// PubSubClient mqttClient(espClient);

// // --- LoRa setup ---
// void setupLoRa() {
//   SPI.begin(5, 19, 27);  // SCK, MISO, MOSI
//   LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
//   if (!LoRa.begin(LORA_BAND)) {
//     Serial.println("LoRa init failed!");
//     while (1);
//   }
//   Serial.println("LoRa initialized");
// }

// // --- WiFi setup ---
// void setupWiFi() {
//   WiFi.begin(ssid, password);
//   Serial.print("Connecting WiFi");
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("\nWiFi connected, IP: " + WiFi.localIP().toString());
// }

// // --- MQTT reconnect ---
// void mqttReconnect() {
//   while (!mqttClient.connected()) {
//     Serial.print("Attempting MQTT connection...");
//     if (mqttClient.connect("esp32-gateway", mqtt_user, mqtt_pass)) {
//       Serial.println("connected");
//     } else {
//       Serial.print("failed, rc=");
//       Serial.print(mqttClient.state());
//       Serial.println(" try again in 5 seconds");
//       delay(5000);
//     }
//   }
// }

// void setup() {
//   Serial.begin(115200);
//   delay(500);

//   setupLoRa();
//   setupWiFi();
//   mqttClient.setServer(mqtt_server, mqtt_port);
// }

// void loop() {
//   if (!mqttClient.connected()) {
//     mqttReconnect();
//   }
//   mqttClient.loop();

//   int packetSize = LoRa.parsePacket();
//   if (packetSize) {
//     String incoming = "";
//     while (LoRa.available()) {
//       incoming += (char)LoRa.read();
//     }
//     Serial.print("Received: ");
//     Serial.println(incoming);

//     mqttClient.publish("iot/lora/sensor1", incoming.c_str());
//     Serial.println("Published to MQTT as admin user");
//   }

//   delay(50);
// }

//Before Iterasi

// #include <Arduino.h>
// #include <SPI.h>
// #include <LoRa.h>
// #include <ArduinoJson.h>
// #include "secure_comm.h"

// #define LORA_SS   15
// #define LORA_RST  14
// #define LORA_DIO0 26

// void setup() {
//   Serial.begin(115200);
//   Serial.println("\n[RX] Secure LoRa Receiver Starting...");

//   LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
//   if (!LoRa.begin(433E6)) {
//     Serial.println("[RX] LoRa init failed. Check wiring!");
//     while (true);
//   }

//   Serial.println("[RX] LoRa init OK. Waiting for packets...");
// }


// void loop() {
//   int packetSize = LoRa.parsePacket();
//   if (!packetSize) return;

//   String incoming = "";
//   while (LoRa.available()) incoming += (char)LoRa.read();

//   int sepIndex = incoming.indexOf('|');
//   if (sepIndex == -1) {
//     Serial.println("[RX] Invalid format!");
//     return;
//   }

//   String cipherHex = incoming.substring(0, sepIndex);
//   String hmacReceived = incoming.substring(sepIndex + 1);

//   String computedHmac = computeHMAC(cipherHex);
//   if (computedHmac != hmacReceived) {
//     Serial.println("[RX] ❌ HMAC mismatch. Packet rejected!");
//     return;
//   }

//   uint8_t cipherBytes[256];
//   int cipherLen = hexToBytes(cipherHex, cipherBytes);
//   String decrypted = aesDecrypt(cipherBytes, cipherLen);

//   Serial.println("[RX] ✅ Verified and decrypted JSON:");
//   Serial.println(decrypted);

//   StaticJsonDocument<256> doc;
//   if (deserializeJson(doc, decrypted)) {
//     Serial.println("[RX] ⚠️ JSON parse error!");
//     return;
//   }

//   Serial.printf("[RX] MQ135: %.2f | MQ7: %.2f | MQ9: %.2f | Temp: %.2f | Hum: %.2f\n",
//                 (float)doc["MQ135"], (float)doc["MQ7"], (float)doc["MQ9"],
//                 (float)doc["Temp"], (float)doc["Hum"]);
// }
#include <Arduino.h>
#include <LoRa.h>
#include "secure_comm.h"

#define LORA_SS   15
#define LORA_RST  14
#define LORA_DIO0 26

void setup() {
  Serial.begin(115200);
  Serial.println("[RX] Secure LoRa Receiver Starting...");
  
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("[RX] LoRa init failed!");
    while (true);
  }
  Serial.println("[RX] LoRa init OK.");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }

    Serial.println("[RX] Received encrypted packet:");
    Serial.println(received);

    String decrypted = decryptAndVerify(received);
    if (decrypted != "") {
      Serial.println("[RX]  HMAC OK! Decrypted payload:");
      Serial.println(decrypted);
    }
  }
}

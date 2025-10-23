//Code For Calibration and Reading Sensors
// #include <Arduino.h>
// #include "sensor_calibration.h"
// #include "sensor_readings.h"

// void setup() {
//   Serial.begin(115200);
//   dht.begin();

//   Serial.println("Starting ESP32 Sensor Node...");

//   // Uncomment this once to calibrate all MQ sensors
//   calibrateSensors();

//   delay(2000);
// }

// void loop() {
//   SensorData data = readAllSensors();

//   Serial.printf("MQ135: %.2f ppm | MQ7: %.2f ppm | MQ9: %.2f ppm\n", 
//                 data.mq135_ppm, data.mq7_ppm, data.mq9_ppm);
//   Serial.printf("Temp: %.1f°C | Hum: %.1f%%\n\n", 
//                 data.temperature, data.humidity);

//   delay(5000);
// }

// 💡 Tips penting:

// Jika kamu tidak ingin mengkalibrasi ulang tiap boot, hapus baris ini sementara:
// calibrateSensors();
// di file main.ino, setelah selesai kalibrasi pertama.

// Karena nilai Ro sudah tersimpan di EEPROM, jadi tidak perlu kalibrasi ulang setiap kali nyala.

// //coding untuk send mqtt
// #include <WiFi.h>
// #include <PubSubClient.h>
// #include "sensor_readings.h"

// const char* ssid = "FTI USAKTI";
// const char* password = "trisakti2026";
// const char* mqtt_server = "10.24.80.89"; // IP Raspberry Pi kamu

// WiFiClient espClient;
// PubSubClient client(espClient);

// void setup_wifi() {
//   delay(10);
//   Serial.println("Connecting to WiFi...");
//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("Connected to WiFi!");
// }

// void reconnect() {
//   while (!client.connected()) {
//     Serial.print("Connecting to MQTT...");
//     if (client.connect("ESP32_SensorNode", "admin", "gracias, idiota")) {
//       Serial.println("connected!");
//     } else {
//       Serial.print("failed, rc=");
//       Serial.print(client.state());
//       delay(2000);
//     }
//   }
// }

// void setup() {
//   Serial.begin(115200);
//   dht.begin();
//   setup_wifi();
//   client.setServer(mqtt_server, 1883);
// }

// void loop() {
//   if (!client.connected()) reconnect();
//   client.loop();

//   SensorData data = readAllSensors();

//   char payload[128];
//   snprintf(payload, sizeof(payload),
//            "{\"mq135\":%.2f,\"mq7\":%.2f,\"mq9\":%.2f,\"temp\":%.1f,\"hum\":%.1f}",
//            data.mq135_ppm, data.mq7_ppm, data.mq9_ppm, data.temperature, data.humidity);

//   client.publish("esp32/sensors", payload);
//   Serial.println(payload);

//   delay(5000);
// }

#include <Arduino.h>
#include <LoRa.h>
#include "sensor_readings.h"
#include "sensor_calibration.h"
#include "crypto_utils.h"

#define LORA_SS    15
#define LORA_RST   14
#define LORA_DIO0  26


void setup() {
  Serial.begin(115200);
  Serial.println("\n[TX] Secure LoRa Transmitter Starting...");

  dht.begin();
  // calibrateSensors();

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("[TX] LoRa init failed. Check wiring!");
    while (true);
  }
  Serial.println("[TX] LoRa init OK.");
  delay(1000);
}

void loop() {
  // --- Ambil data sensor
  SensorData data = readAllSensors();

  String payload = String("{\"MQ135\":") + data.mq135_ppm +
                   ",\"MQ7\":" + data.mq7_ppm +
                   ",\"MQ9\":" + data.mq9_ppm +
                   ",\"Temp\":" + data.temperature +
                   ",\"Hum\":" + data.humidity + "}";

  // --- Enkripsi + tanda tangan
  String encrypted = encryptAndSign(payload);

  //Pengaturan power TX LoRa
  LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);
  // --- Kirim via LoRa
  LoRa.beginPacket();
  LoRa.print(encrypted);
  LoRa.endPacket();

  Serial.println("[TX] Sending secure packet:");
  Serial.println(payload);
  Serial.println("[TX] Encrypted:");
  Serial.println(encrypted);
  Serial.println("----------------------------------");
  delay(5000);
}

/**
 * ============================================================
 * Project  : IoT-Based Solar-Powered Weather & Air Quality
 *            Monitoring Station
 * File     : main.ino
 * Hardware : ESP8266 NodeMCU
 *            + ENS160 Air Quality Sensor (I2C, addr 0x53)
 *            + AHT2x Temperature & Humidity Sensor (I2C)
 * Note     : This file uses ESP32 WiFi.h by default. For ESP8266, replace
 *            `#include <WiFi.h>` with `#include <ESP8266WiFi.h>`.
 * Protocol : MQTT over TCP (HiveMQ public/cloud broker)
 * Authors  : Nguyễn Việt Chân, Đỗ Minh Phúc, Hà Thanh Sang
 * Date     : 2026
 * ============================================================
 */

#include <WiFi.h>             // For ESP32 — replace with ESP8266WiFi.h for ESP8266 boards
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <ScioSense_ENS160.h>

// --- 1. WIFI & MQTT CONFIGURATION ---
// TODO: Replace with your credentials
const char* ssid     = "Huflit-CB-NV";
const char* password = "huflitcbnv@123";

const char* mqtt_server = "broker.hivemq.com";
const int   mqtt_port   = 1883;

// MQTT Topics (you can rename the "simon_house" prefix as needed)
const char* topic_alert = "simon_house/air/alert"; // Alert/warning topic
const char* topic_aqi   = "simon_house/air/aqi";
const char* topic_eco2  = "simon_house/air/eco2";
const char* topic_tvoc  = "simon_house/air/tvoc";
const char* topic_temp  = "simon_house/air/temp";
const char* topic_hum   = "simon_house/air/hum";

// --- 2. OBJECT INITIALIZATION ---
WiFiClient    espClient;
PubSubClient  client(espClient);

Adafruit_AHTX0     aht;
ScioSense_ENS160   ens160(ENS160_I2CADDR_1); // I2C address 0x53

// --- 3. LOGIC VARIABLES ---
unsigned long lastMsg  = 0;
unsigned long interval = 5000; // Default slow measurement interval: 5 s
bool isDanger = false;          // Flag for dangerous air quality state

// Alert thresholds: AQI >= 3 (1-2 = good, 3-5 = poor) OR eCO2 > 1000 ppm
const int THRESHOLD_AQI = 3;
const int THRESHOLD_CO2 = 1000;

// --- WiFi CONNECTION FUNCTION ---
void setup_wifi() {
  delay(10);
  Serial.print("\nConnecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

// --- MQTT RECONNECT FUNCTION ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "ESP32_AirClient_";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("Connected!");
      client.publish(topic_alert, "System started - Air is safe");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize I2C bus
  Wire.begin();

  // Initialize AHT temperature & humidity sensor
  if (!aht.begin()) {
    Serial.println("Error: AHT sensor not responding!");
    while (1) delay(10);
  }
  Serial.println("AHT OK!");

  // Initialize ENS160 air quality sensor
  if (!ens160.begin()) {
    Serial.println("Error: ENS160 sensor not responding!");
    while (1) delay(10);
  }
  ens160.setMode(ENS160_OPMODE_STD);
  Serial.println("ENS160 OK!");

  // Setup network and MQTT
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  // Keep MQTT connection alive
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // --- ADAPTIVE MEASUREMENT LOGIC ---
  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    // 1. Read sensor data
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);

    // Feed temperature & humidity into ENS160 for compensation (IMPORTANT for accuracy)
    ens160.set_envdata(temp.temperature, humidity.relative_humidity);
    ens160.measure(true);
    ens160.measure(false);

    int currentAQI  = ens160.getAQI();
    int currentCO2  = ens160.geteCO2();
    int currentTVOC = ens160.getTVOC();

    // 2. Publish sensor readings to MQTT (always publish, regardless of alert state)
    client.publish(topic_temp, String(temp.temperature).c_str());
    client.publish(topic_hum,  String(humidity.relative_humidity).c_str());
    client.publish(topic_aqi,  String(currentAQI).c_str());
    client.publish(topic_eco2, String(currentCO2).c_str());
    client.publish(topic_tvoc, String(currentTVOC).c_str());

    // Print to Serial Monitor for debugging
    Serial.print("AQI: ");  Serial.print(currentAQI);
    Serial.print(" | CO2: "); Serial.print(currentCO2);
    Serial.print(" | Temp: "); Serial.println(temp.temperature);

    // 3. ALERT LOGIC — trigger if AQI >= 3 (Poor) OR eCO2 > 1000 ppm (stuffy)
    if (currentAQI >= THRESHOLD_AQI || currentCO2 > THRESHOLD_CO2) {

      // First time entering danger state: publish alert and switch to fast sampling
      if (!isDanger) {
        client.publish(topic_alert, "WARNING: AIR QUALITY POOR!");
        interval  = 1000; // Speed up sampling to 1 s
        isDanger  = true;
        Serial.println("-> Danger! Switching to fast sampling 1s");
      }

    } else {
      // Air quality has returned to safe levels
      if (isDanger) {
        client.publish(topic_alert, "Air quality is safe");
        interval = 5000; // Return to slow sampling 5 s
        isDanger = false;
        Serial.println("-> Safe. Returning to slow sampling 5s");
      }
    }
  }
}

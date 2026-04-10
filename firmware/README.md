# Firmware — Setup Guide

This guide explains how to set up, configure, and flash the firmware for the IoT Weather & Air Quality Monitoring Station onto an ESP8266 (or compatible ESP32) board.

---

## Prerequisites

| Tool | Version | Notes |
|---|---|---|
| Arduino IDE | 1.8.x or 2.x | [Download](https://www.arduino.cc/en/software) |
| ESP8266 Board Package | ≥ 3.1 | Install via Boards Manager |

### Installing the ESP8266 Board Package

1. In Arduino IDE open **File** → **Preferences**
2. Add this URL to **Additional Board Manager URLs**:
   ```
   https://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
3. Open **Tools** → **Board** → **Boards Manager…**, search for **esp8266**, and install the package by ESP8266 Community.

---

## Library Installation

See [`libraries.md`](libraries.md) for the full list and installation steps.

---

## Configuration

Open `main/main.ino` and update the following constants near the top of the file:

```cpp
// TODO: Replace with your credentials
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* mqtt_server = "broker.hivemq.com"; // or your own broker
const int   mqtt_port   = 1883;
```

You may also rename the MQTT topic prefix `simon_house` to match your own project namespace.

---

## Flashing Instructions

1. Select the correct board: **Tools** → **Board** → **ESP8266 Boards** → **NodeMCU 1.0 (ESP-12E Module)** (adjust for your exact board)
2. Select the correct port: **Tools** → **Port** → (your COM/ttyUSB port)
3. Click **Upload** (▶)

---

## Serial Monitor Expected Output

After a successful flash, open **Serial Monitor** at **115200 baud**. You should see:

```
Connecting to WiFi: <your_ssid>
.....
WiFi connected!
AHT OK!
ENS160 OK!
Connecting to MQTT...Connected!
AQI: 1 | CO2: 412 | Temp: 28.34
AQI: 1 | CO2: 415 | Temp: 28.37
...
```

- Readings are published every **5 seconds** under normal air quality conditions.
- If AQI ≥ 3 or eCO2 > 1000 ppm, the firmware switches to **1-second sampling** and publishes a `WARNING` alert.
- Once air quality recovers, it reverts to 5-second sampling.

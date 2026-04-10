# System Architecture

## Overview

The monitoring station is built around a 3-layer architecture that separates concerns into power management, sensing & processing, and communication & display.

```
┌─────────────────────────────────────────────────────────┐
│  LAYER 3 — Communication & Display                      │
│  Wi-Fi → MQTT (HiveMQ) → Web Dashboard (GitHub Pages)  │
└─────────────────────────┬───────────────────────────────┘
                          │
┌─────────────────────────▼───────────────────────────────┐
│  LAYER 2 — Sensor & Processing                          │
│  ESP8266 NodeMCU                                        │
│  ├── I2C Bus: ENS160, AHT2x, BMP280                    │
│  ├── UART: PMS7003 (PM2.5)                              │
│  └── ADC (A0): GUVA-S12SD (UV)                         │
└─────────────────────────┬───────────────────────────────┘
                          │
┌─────────────────────────▼───────────────────────────────┐
│  LAYER 1 — Power                                        │
│  Solar Panel 6V 6W → BMS 2S → LM2596 Buck → 3.3V/5V   │
└─────────────────────────────────────────────────────────┘
```

---

## Layer 1 — Power Layer

| Component | Role |
|---|---|
| Solar Panel 6V 6W | Primary energy source |
| BMS 2S (Battery Management System) | Manages two Li-ion cells in series; handles charging, balancing, and over-discharge protection |
| LM2596 Buck Converter | Steps down battery voltage (~7.4V) to stable 5V (USB-level) for the NodeMCU and 3.3V for sensors |

The system prioritises continuous operation by charging the battery during daylight hours while simultaneously powering the electronics. At night or during low-light conditions, stored battery energy sustains operation.

---

## Layer 2 — Sensor & Processing Layer

The ESP8266 NodeMCU acts as the central microcontroller. It orchestrates all sensor readings and coordinates network communication.

### Sensor Interfaces

| Sensor | Interface | Parameter Measured |
|---|---|---|
| ENS160 | I2C (0x53) | AQI, eCO2, TVOC |
| AHT2x | I2C (0x38) | Temperature, Humidity |
| BMP280 | I2C (0x76 or 0x77) | Barometric pressure, ambient temperature |
| PMS7003 | UART (9600 baud) | PM1.0, PM2.5, PM10 particle counts |
| GUVA-S12SD | ADC (A0) | UV Index (analog voltage → UV level) |

### Processing Logic

The firmware runs an adaptive measurement loop:

- **Normal state** (AQI < 3 AND eCO2 ≤ 1000 ppm): publish sensor data every **5 seconds**
- **Alert state** (AQI ≥ 3 OR eCO2 > 1000 ppm): publish every **1 second** and send a warning MQTT message
- On every cycle, temperature and humidity from AHT2x are fed into the ENS160 as environmental compensation data, improving AQI accuracy

---

## Layer 3 — Communication & Display Layer

### Data Flow

```
ESP8266 firmware
    │
    ├─ MQTT publish → broker.hivemq.com (TCP 1883)
    │       │
    │       └─ HiveMQ Cloud broker
    │               │
    │               └─ WebSocket (WSS 8884)
    │                       │
    │                       └─ Web Dashboard (Paho.MQTT.js)
    │                               │
    │                               ├─ Live sensor cards
    │                               ├─ Gauge charts (PM2.5, AQI)
    │                               ├─ Temperature history chart
    │                               └─ CSV export (localStorage)
    │
    └─ Open-Meteo API (HTTP GET) → Hourly weather forecast panel
```

### Dashboard Technology Stack

| Component | Technology |
|---|---|
| MQTT client | Eclipse Paho.MQTT.js (via CDN) |
| Charts | Chart.js (via CDN) |
| Weather API | Open-Meteo (free, no API key required) |
| Hosting | GitHub Pages |

---

## Duty Cycling & Energy Management

To extend battery life during cloudy days or nights, the firmware uses an **adaptive sampling rate**:

- Under safe air quality conditions the ESP8266 spends most of its time idle between 5-second publish cycles — this reduces average power consumption.
- Under alert conditions, the 1-second cycle ensures rapid response but consumes more power; this is intentional since dangerous air quality is a short-lived event.

Future improvement: implement Deep Sleep mode (ESP8266 `ESP.deepSleep()`) between cycles for dramatic power savings in battery-only scenarios.

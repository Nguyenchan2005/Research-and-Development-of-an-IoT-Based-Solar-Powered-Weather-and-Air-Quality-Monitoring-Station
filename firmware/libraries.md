# Required Arduino Libraries

Install all libraries listed below before compiling `main.ino`.

## Library Table

| Library | Version | Source | Purpose |
|---|---|---|---|
| PubSubClient | ≥ 2.8 | Arduino Library Manager | MQTT client for publishing sensor data |
| Adafruit AHTX0 | ≥ 2.0 | Arduino Library Manager | Driver for AHT20/AHT21 temperature & humidity sensor |
| ScioSense ENS160 | latest | GitHub: [sciosense/ENS160_driver](https://github.com/sciosense/ENS160_driver) | Driver for ENS160 air quality sensor (AQI, eCO2, TVOC) |
| Wire | built-in | Arduino built-in | I2C bus communication |
| WiFi / ESP8266WiFi | built-in | ESP8266/ESP32 board package | Wi-Fi connectivity |

## Installation Steps

1. Open **Arduino IDE** → **Sketch** → **Include Library** → **Manage Libraries…**
2. Search for and install **PubSubClient** by Nick O'Leary (≥ 2.8)
3. Search for and install **Adafruit AHTX0** (≥ 2.0) — also install its dependency **Adafruit Unified Sensor** when prompted
4. For **ScioSense ENS160**: download the ZIP from [GitHub](https://github.com/sciosense/ENS160_driver), then in Arduino IDE choose **Sketch** → **Include Library** → **Add .ZIP Library…**

> **Note:** `Wire` and `ESP8266WiFi` / `WiFi` are bundled with the board package and do not need to be installed separately.

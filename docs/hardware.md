# Hardware Components Reference

## Bill of Materials

| # | Component | Model / Part | Role | Interface | Supply Voltage |
|---|---|---|---|---|---|
| 1 | Microcontroller | ESP8266 NodeMCU v1.0 (ESP-12E) | Central MCU, Wi-Fi | GPIO, I2C, UART, ADC | 3.3V (on-board LDO) |
| 2 | Air Quality Sensor | ScioSense ENS160 | AQI (1–5), eCO2 (ppm), TVOC (ppb) | I2C, addr 0x53 | 1.8V / 3.3V |
| 3 | Temp & Humidity | Aosong AHT20 / AHT21 | Temperature (°C), Relative Humidity (%) | I2C, addr 0x38 | 3.3V |
| 4 | Barometric Pressure | Bosch BMP280 | Pressure (hPa), temperature | I2C, addr 0x76 or 0x77 | 3.3V |
| 5 | Particulate Matter | Plantower PMS7003 | PM1.0, PM2.5, PM10 (µg/m³) | UART 9600 baud | 5V |
| 6 | UV Sensor | GUVA-S12SD breakout | UV Index (0–11+) | Analog (ADC) | 3.3V |
| 7 | Solar Panel | 6V, 6W polycrystalline | Primary power source | — | Output: 6V @ 1A |
| 8 | Battery Management | BMS 2S (e.g. HY2120) | Li-ion charge / protection / balancing | — | 7.4V nominal |
| 9 | Voltage Regulator | LM2596S Buck Converter | 7.4V → 5V DC-DC step-down | — | Input ≤ 40V, Output adj. |

---

## Detailed Specifications

### 1. ESP8266 NodeMCU v1.0

- **CPU:** Tensilica L106 @ 80 / 160 MHz
- **Flash:** 4 MB
- **Wi-Fi:** 802.11 b/g/n, 2.4 GHz
- **GPIO:** 11 usable digital pins, 1× ADC (10-bit, 0–1V input range)
- **I2C:** SDA = D2 (GPIO4), SCL = D1 (GPIO5)
- **UART:** D7 (GPIO13) RX, D8 (GPIO15) TX (software serial recommended for PMS7003)
- **Operating voltage:** 3.3V (powered via USB 5V → on-board AMS1117 LDO)

### 2. ScioSense ENS160

- **Measurement:** AQI (1–5 scale), eCO2 (400–65000 ppm), TVOC (0–65000 ppb)
- **Response time:** < 1 s
- **Interface:** I2C (address selectable: 0x52 or 0x53)
- **Key feature:** Accepts external temperature/humidity compensation for improved accuracy
- **Driver:** [sciosense/ENS160_driver](https://github.com/sciosense/ENS160_driver)

### 3. AHT20 / AHT21 (Aosong AHTX0 family)

- **Temperature range:** −40°C to +85°C, ±0.3°C accuracy
- **Humidity range:** 0–100% RH, ±2% RH accuracy
- **Interface:** I2C, address 0x38
- **Library:** Adafruit AHTX0

### 4. BMP280 (Bosch)

- **Pressure range:** 300–1100 hPa, ±1 hPa absolute accuracy
- **Temperature range:** −40°C to +85°C
- **Interface:** I2C (0x76 / 0x77) or SPI
- **Library:** Adafruit BMP280 or Adafruit Unified Sensor

### 5. PMS7003 (Plantower)

- **Measurement:** PM1.0, PM2.5, PM10 concentration (µg/m³)
- **Range:** 0–500 µg/m³
- **Interface:** UART 9600-8N1, binary protocol
- **Supply:** 5V, ~80 mA peak during fan operation
- **Note:** Use a logic-level shifter if connecting RX/TX directly to ESP8266 3.3V pins

### 6. GUVA-S12SD UV Sensor

- **Spectral range:** 240–370 nm (UV-A + UV-B)
- **Output:** Analog voltage (0–1V) proportional to UV intensity
- **Conversion:** UV Index ≈ `V_out (mV) / 110`
- **Note:** ESP8266 ADC input max is 1.0V; use a voltage divider if sensor output exceeds this

### 7. Solar Panel — 6V 6W

- **Open-circuit voltage (Voc):** ~7.2V
- **Maximum power voltage (Vmp):** ~6V
- **Short-circuit current (Isc):** ~1.1A
- **Dimensions:** approx. 200 × 130 mm (varies by manufacturer)

### 8. BMS 2S

- **Cell configuration:** 2 Li-ion/LiPo cells in series → ~7.4V nominal
- **Functions:** Over-charge, over-discharge, over-current, and short-circuit protection; cell balancing
- **Charging input:** Connect solar panel output through a suitable solar charge controller

### 9. LM2596S Buck Converter

- **Input voltage:** 4.5V – 40V
- **Output voltage:** Adjustable 1.25V – 37V (set to 5V for this project)
- **Maximum output current:** 3A
- **Efficiency:** ~92% typical

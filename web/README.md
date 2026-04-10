# Web Dashboard — Setup Guide

## Overview

The dashboard is a single-page HTML application (`dashboard.html`) that:

- Connects to an MQTT broker over WebSockets (SSL) to receive real-time sensor readings
- Displays live values for Temperature, Humidity, PM2.5, AQI, eCO2, TVOC, and UV Index
- Shows gauge charts (doughnut style) for PM2.5 and AQI
- Plots a live temperature history line chart (last 30 readings)
- Fetches current weather and a 24-hour forecast from the [Open-Meteo API](https://open-meteo.com/)
- Saves up to 7 days of readings in browser `localStorage` for CSV export
- Supports Dark / Light mode toggle with state persisted across sessions

---

## Running Locally

1. Clone or download the repository
2. Open `web/dashboard.html` directly in any modern browser (Chrome, Firefox, Edge, Safari)
3. No web server or build step is required — all dependencies are loaded from CDNs

---

## Live Demo (GitHub Pages)

The dashboard is automatically deployed to GitHub Pages on every push to `master`:

🌐 **[https://nguyenchan2005.github.io/Research-and-Development-of-an-IoT-Based-Solar-Powered-Weather-and-Air-Quality-Monitoring-Station/](https://nguyenchan2005.github.io/Research-and-Development-of-an-IoT-Based-Solar-Powered-Weather-and-Air-Quality-Monitoring-Station/)**

---

## MQTT Topics Subscribed

| Topic | Payload Type | Description |
|---|---|---|
| `simon_house/air/temp` | float | Temperature in °C |
| `simon_house/air/hum` | float | Relative humidity in % |
| `simon_house/air/pm25` | float | PM2.5 particulate in µg/m³ |
| `simon_house/air/aqi` | int (1–5) | Air Quality Index from ENS160 |
| `simon_house/air/eco2` | int (ppm) | Equivalent CO₂ concentration |
| `simon_house/air/tvoc` | int (ppb) | Total Volatile Organic Compounds |
| `simon_house/air/uv` | float | UV Index |
| `simon_house/air/alert` | string | Alert message from firmware |

---

## Configuring MQTT Credentials

The MQTT connection is configured at the top of the `<script>` section in `dashboard.html`:

```js
var mqtt_server = "e35ff2a8c16f4c60981704de63982340.s1.eu.hivemq.cloud";
var mqtt_port   = 8884;  // WSS port
var client      = new Paho.MQTT.Client(mqtt_server, mqtt_port, "Web_" + new Date().getTime());

client.connect({
    useSSL: true,
    userName: "Huflit",
    password: "Huflit123456",
    ...
});
```

Replace `mqtt_server`, `userName`, and `password` with your own HiveMQ Cloud (or other broker) credentials.

> **Security note:** Never commit real credentials to a public repository. Move sensitive values to a separate `config.js` file that is excluded via `.gitignore`, or use environment variables.

---

## Customising the Location

The dashboard default coordinates are set to Ho Chi Minh City (Su Van Hanh Campus). To change the weather location, update the `userLocation` variable near the top of the `<script>` section in `dashboard.html`:

```js
var userLocation = { lat: YOUR_LAT, lon: YOUR_LON }; // Your location
```

The location label shown in the weather widget is set a few lines further down:

```js
document.getElementById("weatherLocation").innerText = "Your Location Name";
```

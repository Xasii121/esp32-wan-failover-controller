# ESP32 Automated WAN Failover Controller

A lightweight, hardware-assisted network engineering project that monitors WAN link quality and automatically switches hardware internet gateways via low-voltage relays when carrier data caps are reached.

## 📋 The Network Problem
Many mobile broadband / 4G LTE infrastructure providers do not hard-disconnect users when monthly high-speed quotas expire. Instead, they dynamically shape traffic down to a near-unusable speed (e.g., 128 kbps). This causes massive network latency spikes and severe bufferbloat, which standard automated software gateways fail to register as a dead link.

## 💡 The Solution
This ESP32 application acts as an independent out-of-band network orchestrator:
1. It continuously polls public IP roots (`8.8.8.8`) via ICMP echo requests to log baseline connection behaviors.
2. If traffic shaping is detected (monitored latency spikes past `450ms` for several consecutive iterations), it registers that data limits are exhausted.
3. It posts an automated API emergency alert payload over **Telegram** to notify the engineering team.
4. It fires a GPIO high/low state change across a dual-channel 5V relay module to physically isolate the unmetered router and spin up the backup infrastructure.

## 🛠️ Hardware Stack
- ESP32 Development Board (NodeMCU / ESP32-WROOM-32)
- 2-Channel 5V Optocoupled Relay Module
- External DC barrel jack breakout routing paths

## 🚀 Deployment Guide
1. Install dependencies in your IDE: `ESP32Ping`, `UniversalTelegramBot`, and `ArduinoJson`.
2. Clone this repository to your local directory.
3. Duplicate the `config.h.example` file and rename the new copy to exactly `config.h`.
4. Update the placeholder credentials in `config.h` with your Wi-Fi keys, Telegram bot details, and environment thresholds.
5. Flash the codebase directly onto your ESP32 target over Micro-USB.

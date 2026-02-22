<h1 align="center">Sky-Fi</h1>
<p align="center">
  <em>Wi-Fi Controlled FPV RC Plane — Fly from your browser</em>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/MCU-ESP32-E7352C?style=for-the-badge&logo=espressif&logoColor=white"/>
  <img src="https://img.shields.io/badge/Language-C%2B%2B%20(Arduino)-00979D?style=for-the-badge&logo=arduino&logoColor=white"/>
  <img src="https://img.shields.io/badge/Control-Wi--Fi%20Web%20UI-3498DB?style=for-the-badge&logo=wifi&logoColor=white"/>
  <img src="https://img.shields.io/badge/PCB-Custom%20Design-blueviolet?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Domain-IoT%20%7C%20Robotics-green?style=for-the-badge"/>
</p>

---

## What is Sky-Fi?

**Sky-Fi** is a Wi-Fi controlled FPV (First Person View) RC airplane powered by an **ESP32 microcontroller**.  
It hosts a **real-time web-based control interface** accessible from any phone or browser on your local network — no dedicated transmitter required.

> **In short:** Connect to the plane's Wi-Fi, open a browser, and control throttle and steering via a touch interface.

### System Architecture

```
[ Phone / Browser ]
    | HTTP fetch (every 50ms)
    v
[ ESP32 Web Server (port 80) ]
    |  /set?motor=left&value=XXX
    |  /set?motor=right&value=XXX
    v
[ PWM Motor Driver ]
   |-- GPIO 5  --> Left Motor  (0-255 PWM)
   |-- GPIO 43 --> Right Motor (0-255 PWM)
    v
[ RC Plane — Throttle + Differential Steering ]
```

### Control Interface

The ESP32 serves a **mobile-optimised HTML page** (stored in PROGMEM) with:

| Control                             | Function                                              |
|-------------------------------------|-------------------------------------------------------|
| **Vertical Throttle Slider** (left) | Sets motor speed from 0 to 255                        |
| **Left Button** (right side)        | Differential steer left — reduces left, boosts right  |
| **Right Button** (right side)       | Differential steer right — reduces right, boosts left |
| **Debug Display** (top center)      | Shows live `L: xxx | R: xxx` PWM values               |

### Steering Logic — 60% Sensitivity

```
Turning Left:
  leftMotor  = throttle - (throttle x 0.6)
  rightMotor = throttle + (throttle x 0.6)

Turning Right:
  rightMotor = throttle - (throttle x 0.6)
  leftMotor  = throttle + (throttle x 0.6)

All values clamped to [0, 255]
```

---

## Project Structure

```
Sky-Fi/
├── Src.c                                  # Main ESP32 firmware (Arduino C++)
├── PCB_PCB_plane_2025-08-26.json          # EasyEDA PCB design file
├── PCB_PCB_plane_2025-08-26.pdf           # PCB layout PDF
├── Schematic_plane_2025-08-26.pdf         # Circuit schematic PDF
├── Gerber_plane_PCB_plane_2025-08-26.zip  # Gerber files for PCB fabrication
├── EIOT_SKY-FI_FINAL.pdf                  # Final project report
└── README.md
```

---

## Tools & Technologies

| Category              | Technology                                           |
|-----------------------|------------------------------------------------------|
| **Microcontroller**   | ESP32 (Dual-core 240MHz, Wi-Fi built-in)             |
| **Firmware Language** | Arduino C++ (`WiFi.h`, `WebServer.h`)                |
| **Motor Control**     | `ledcAttach` / `ledcWrite` — 8-bit PWM @ 5kHz       |
| **Web Stack**         | HTML5 + CSS + Vanilla JavaScript (served via PROGMEM)|
| **Communication**     | IEEE 802.11 Wi-Fi (AP or STA mode)                   |
| **PCB Design**        | EasyEDA                                              |
| **PCB Fabrication**   | Gerber files (ready for JLCPCB / PCBWay)             |
| **IDE**               | Arduino IDE / VS Code + Arduino Extension            |
| **Serial Monitor**    | 115200 baud (for IP address and debug logs)          |

---

## Getting Started

### 1. Configure Wi-Fi Credentials
Edit `Src.c` at the top:
```cpp
const char* ssid     = "your_wifi_name";
const char* password = "your_password";
```

### 2. Flash to ESP32
- Open in **Arduino IDE**
- Select board: **ESP32 Dev Module**
- Upload `Src.c`

### 3. Get the IP Address
Open **Serial Monitor** at `115200 baud`. After connecting, you will see:
```
Connected to Wi-Fi!
IP Address: 192.168.x.x
```

### 4. Open the Control UI
On your phone or PC connected to the same Wi-Fi network:
```
http://192.168.x.x/
```
Use the **throttle slider** and **steering buttons** to fly.

### Motor Pin Reference

| Motor        | GPIO    | PWM Channel          |
|:---:|:---:|:---:|
| Left Motor  | GPIO 5  | `ledcAttach(5, ...)`  |
| Right Motor | GPIO 43 | `ledcAttach(43, ...)` |

---

## PCB & Hardware

The custom PCB was designed in **EasyEDA** — fabrication-ready Gerber files are included.

| File                                     | Description                       |
|------------------------------------------|-----------------------------------|
| `Schematic_plane_2025-08-26.pdf`         | Full circuit schematic            |
| `PCB_PCB_plane_2025-08-26.pdf`           | PCB copper layer layout           |
| `Gerber_plane_PCB_plane_2025-08-26.zip`  | Ready to upload to JLCPCB / PCBWay|
| `EIOT_SKY-FI_FINAL.pdf`                  | Detailed project report           |

---

## License

This project is open-source under the [MIT License](LICENSE).

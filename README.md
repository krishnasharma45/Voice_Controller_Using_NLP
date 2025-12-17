# Voice Controller Automation System

A production‑grade, ESP32‑based voice controller that captures audio from an INMP441 I2S MEMS microphone, streams audio for cloud Speech‑to‑Text, interprets the transcribed text with simple NLP/intent logic, and triggers device actions over Wi‑Fi.

> Repo highlights: low‑level I2S audio capture (`I2S.*`), audio framing (`Audio.*`), cloud speech client (`CloudSpeechClient.*`), Wi‑Fi/API config (`network_param.h`). Two text notes show sample command/response flows.

---

## 🚀 Features

* **Hands‑free control**: Speak commands like “turn on light”, “increase speed”, etc.
* **Clear audio input** with the **INMP441** digital MEMS mic over I2S (low noise, no ADC needed).
* **Cloud Speech‑to‑Text** via a pluggable client (current code: `CloudSpeechClient`), so you can swap providers.
* **Simple NLP/intent mapping**: Map phrases → actions in one place; easy to extend.
* **Wi‑Fi first‑run config** via `network_param.h`.
* **Modular C/C++ codebase** suitable for Arduino IDE or PlatformIO.

---

## 🧩 Architecture

```
┌───────────┐      I2S       ┌──────────────┐     Wi‑Fi       ┌──────────────────┐
│  INMP441  │ ─────────────▶ │   ESP32 MCU   │ ──────────────▶ │  Speech‑to‑Text   │
│  (Mic)    │                │ (I2S + Wi‑Fi) │                 │  Cloud Service    │
└───────────┘                └──────┬───────┘                 └─────────┬────────┘
                                    │                                      │
                                    │ text                                 │
                                    ▼                                      │
                               ┌───────────┐                                │
                               │   NLP     │  phrase→intent→action         │
                               └─────┬─────┘                                │
                                     │                                      │
                                     ▼                                      ▼
                                ┌───────────┐      Control Channels    ┌───────────┐
                                │  Actions  │  (GPIO, HTTP, MQTT, etc) │  Devices  │
                                └───────────┘                           └───────────┘
```

**Core modules**

* `I2S.[h|cpp]` — I2S driver glue: sampling rate, buffer size, DMA, pin map.
* `Audio.[h|cpp]` — Audio capture utilities: ring buffers, framing, basic conditioning.
* `CloudSpeechClient.[h|cpp]` — Uploads audio chunks, handles auth, receives transcripts.
* `network_param.h` — Wi‑Fi SSID/password and (optionally) API keys/endpoints.

---

## 🛠️ Hardware

* **ESP32** development board (dual‑core, Wi‑Fi).
* **INMP441** I2S MEMS microphone (pins: `SCK/BCLK`, `WS/LRCLK`, `SD`, `L/R`, `VDD`, `GND`).
* Cables, breadboard, and the controlled device(s) (LED, relay, motor driver, etc.).

### Suggested wiring (generic)

> Pin numbers vary by board; if the code defines custom pins in `I2S.h/cpp`, **follow those**. Adjust below as needed.

| INMP441  | ESP32 (example) |
| -------- | --------------- |
| VDD      | 3V3             |
| GND      | GND             |
| SCK/BCLK | GPIO 26         |
| WS/LRCLK | GPIO 25         |
| SD       | GPIO 32         |
| L/R      | GND (mono left) |

---

## 💻 Software Requirements

* **Arduino IDE 2.x** (or **PlatformIO**)
* **ESP32 board support** (via Boards Manager)
* **Wi‑Fi access**
* A **Speech‑to‑Text** backend (Google Cloud STT, etc.) and credentials if required

## 🗣️ How It Works (Firmware Flow)

1. **Boot** → connects to Wi‑Fi using `network_param.h`.
2. **I2S init** → sets sample rate (e.g., 16 kHz or 48 kHz), bit depth, and DMA buffers.
3. **Capture** → `Audio` reads mic frames from the I2S ring/DMA buffer.
4. **Stream** → `CloudSpeechClient` sends frames to the speech service (chunked/streaming).
5. **Transcribe** → receives partial/final transcripts.
6. **NLP/Intent** → text is normalized and matched to intents/commands.
7. **Actuate** → triggers GPIO/HTTP/MQTT/etc. handlers.

---

## 🧠 NLP & Command Mapping

Start simple: a dictionary of **phrases → actions**.

```cpp
struct Intent { const char* phrase; void (*handler)(); };

void turnOnLight()  { /* set GPIO HIGH or POST /device/light:on */ }
void turnOffLight() { /* set GPIO LOW  or POST /device/light:off */ }

Intent intents[] = {
  {"turn on light",  turnOnLight},
  {"light on",       turnOnLight},
  {"turn off light", turnOffLight},
  {"light off",      turnOffLight}
};
```

## 🔌 Controlling Devices

You can drive actions via:

* **GPIO** (LED/relay/motor driver)
* **HTTP/REST** (smart plugs, ESPHome, custom APIs)
* **MQTT** (home automation brokers)
* **Serial** (connected MCU)

Example: toggle a relay on GPIO 5


## ✅ Testing Checklist

* [ ] Mic wired correctly (try swapping L/R to force left channel)
* [ ] Device boots and joins Wi‑Fi (check Serial Monitor 115200 baud)
* [ ] Audio frames non‑zero (log peak/RMS periodically)
* [ ] Cloud STT credentials valid (print HTTP status/errors)
* [ ] First command recognized ("turn on light")

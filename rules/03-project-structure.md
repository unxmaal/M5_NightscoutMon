# Rule 03: Project Structure

## Layout

This is a **flat Arduino IDE sketch** — all source files live at the repository root. There is no `src/` directory.

```
M5_NightscoutMon.ino    — Main sketch (~2900 lines): setup(), loop(), display, alarms, WiFi, HTTP, JSON
M5NSconfig.h / .cpp     — tConfig and NSinfo structs, INI file reading, NVS flash read/write
M5NSWebConfig.h / .cpp  — Embedded web server: config UI, OTA update, route handlers
externs.h               — Shared extern declarations across compilation units
Free_Fonts.h            — TFT_eSPI GFX free font #define aliases
IniFile.h / .cpp        — INI file parser (3rd-party, Steve Marple, LGPL v2.1)
DHT12.h / .cpp          — DHT12 I2C temp/humidity sensor driver (bundled)
SHT3X.h / .cpp          — SHT30 I2C temp/humidity sensor driver (bundled)
microdot.h / .cpp       — Pimoroni Micro Dot pHAT I2C display driver (bundled)
iot_iconset_16x16.c     — 16x16 monochrome bitmap icon data
SD/M5NS.INI             — Example INI config file for SD card
Binaries/               — Pre-built firmware binaries
PlatformIO/             — Archived PlatformIO project (2020, not current)
Stand/                  — 3D printable STL files
```

## Build System

**Primary:** Arduino IDE with M5Stack board package.
**Board URL:** `https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json`

There is NO `platformio.ini` at the root. The PlatformIO zip in `PlatformIO/` is a 2020 snapshot and not the active build path.

## Key Dependencies

| Library | Source | Purpose |
|---------|--------|---------|
| M5Stack / M5Core2 | Board package | Hardware abstraction |
| ArduinoJson | Library manager | JSON parsing (16KB DynamicJsonDocument) |
| Adafruit NeoPixel | Library manager | WS2812 LED strip |
| WiFi, WiFiMulti, HTTPClient, WebServer, ESPmDNS | ESP32 core | Networking |
| Preferences | ESP32 core | NVS flash storage |

## Adding New Files

If you add a `.cpp` file, it will be compiled automatically by Arduino IDE. If it needs access to globals, include `"externs.h"`. If new externs are needed, declare them in `externs.h`.

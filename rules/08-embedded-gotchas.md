# Rule 08: Embedded / ESP32 Gotchas

## Memory

- ESP32 has ~520KB SRAM. Free heap is shown on the error log page (page 3).
- The 16KB `DynamicJsonDocument` is a significant allocation — it's a global, reused.
- Audio buffers are large: 25,000 elements (25KB for BASIC, 50KB for Core2 int16_t).
- Avoid dynamic allocation in loops. Prefer stack or global buffers.

## Timing

- `loop()` runs continuously. Timing is managed via `millis()` comparisons, not `delay()`.
- Web server processing happens every 20ms (`msCount` check).
- Nightscout API polling: every 15 seconds, but only fetches when data age > 5 minutes.
- Don't add blocking operations to `loop()` — they freeze the display and web server.

## NVS Flash (Preferences)

- Namespace: `"M5NSconfig"`
- Key names limited to 15 characters by ESP32 NVS
- Used for: config backup, soft restart flag (`SoftReset`), snooze state (`SnoozeUntil`)
- `preferences.begin("M5NSconfig", false)` — false = read-write

## I2C Bus

- Shared by: DHT12, SHT30, Micro Dot pHAT
- Default pins: SDA=21, SCL=22 (M5Stack standard)
- No bus contention handling — sensors polled infrequently

## Power

- BASIC/GRAY/FIRE: `M5.Power.getBatteryLevel()` returns 0-100
- Core2: `M5.Axp.GetBatVoltage()` mapped to 0/25/50/75/100%
- Power off (BASIC): `M5.Power.setWakeupButton(BUTTON_A_PIN)` + `M5.Power.powerOFF()`

## Watchdog / Restart

- Scheduled restart via `restart_at_time` config (HH:MM format)
- Error-triggered restart via `restart_at_logged_errors`
- Soft restart preserves snooze state through NVS

## Build Defines

- `ARDUINOJSON_USE_LONG_LONG 1` — MUST be defined before `#include <ArduinoJson.h>`
- `ARDUINO_M5STACK_Core2` — defined by the board package, not by us

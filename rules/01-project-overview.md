# Rule 01: Project Overview

## What This Is

M5_NightscoutMon is an ESP32/M5Stack Arduino firmware that displays Nightscout CGM (continuous glucose monitor) data on M5Stack devices. It fetches blood glucose readings from a Nightscout server and shows them with trend arrows, color-coded alerts, and alarms.

## License

GNU General Public License v3. Copyright 2018-2021 Martin Lukasek.

## Hardware Targets

Two M5Stack variants, selected at compile time via `#ifdef ARDUINO_M5STACK_Core2`:

| Target | Board ID | Library | Audio | Notes |
|--------|----------|---------|-------|-------|
| M5Stack BASIC/GRAY/FIRE | `M5Stack-Core-ESP32` | `M5Stack.h` | DAC, 5kHz | Physical buttons, vibration motor, NeoPixel LEDs |
| M5Stack Core2 | `M5Stack-Core2` | `M5Core2.h` | I2S DMA, 11025Hz | Touch screen, no vibration/LEDs/Micro Dot pHAT |

## Key External Services

- **Nightscout** — primary CGM data source (REST API v1 + v2)
- **Sugarmate** — alternative Dexcom follower API
- **NTP** — `pool.ntp.org`, `time.nist.gov`, `time.google.com`
- **OTA server** — `http://m5ns.goit.cz/update/`

## Version Format

`YYYYMMDDNN` — e.g., `2022100201` means 2022-10-02, revision 01. Stored in `String M5NSversion`.

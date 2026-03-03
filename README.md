# M5_NightscoutMon

## M5Stack CoreS3 Nightscout Monitor

A bedside CGM (Continuous Glucose Monitor) display for the M5Stack CoreS3, showing real-time glucose data from [Nightscout](https://nightscout.github.io/) with visual and audio alerts.

Copyright (C) 2024-2026 Eric Dodd <eric.e.dodd@gmail.com>
Licensed under the GNU General Public License v3.0 or later. See [LICENSE](LICENSE).

### Originally based on

[M5_NightscoutMon](https://github.com/mlukasek/M5_NightscoutMon) by Martin Lukasek (Copyright 2018-2020), with contributions from Peter Leimbach, Patrick Sonnerat, Sulka Haro, and Dominik Dzienia. The original project targeted the M5Stack Core/Core2 using Arduino IDE.

This version is a ground-up rewrite for the M5Stack CoreS3, with a new architecture, pure-logic libraries, native test suite, and PlatformIO build system. No original source code remains.

### Features

- Large, dark-room-friendly glucose display (color-coded by range)
- Melodic audio alerts — distinct tones for low/high warnings and alarms
- Touch controls — brightness, snooze, page switching
- Web configuration UI — edit settings from any browser on your network
- OTA firmware updates over WiFi
- Multi-WiFi support (up to 10 networks)
- SD card configuration via M5NS.INI

### Hardware

- **M5Stack CoreS3** (ESP32-S3, 320x240 IPS touch display, I2S speaker)
- **MicroSD card** (FAT32, MBR partition scheme) for configuration
- Optional: DIN Base with 500mAh battery (has a physical power switch — must be ON for battery)

### Quick Start

1. Format a microSD card as FAT32 (MBR, not GPT)
2. Create `M5NS.INI` in the root — see [Configuration](#configuration) below
3. Insert the SD card into the CoreS3
4. Flash the firmware:
   ```bash
   cd cores3 && pio run -t upload
   ```
5. Access the web config at `http://<device_name>.local` once connected to WiFi

### Configuration

Create `M5NS.INI` on the SD card root:

```ini
[config]
nightscout = https://your-nightscout-site.example.com
token = your-api-token
name = YourName
device_name = BedsideMon
time_zone = -18000
show_mgdl = 1
show_current_time = 1
brightness1 = 5
brightness2 = 40
brightness3 = 80
yellow_low = 4.5
yellow_high = 9.0
red_low = 3.9
red_high = 11.0
snd_alarm = 3.0
snd_warning = 3.7
snd_alarm_high = 20.0
snd_warning_high = 14.0
warning_volume = 30
alarm_volume = 100

[wlan1]
ssid = YourWiFiSSID
pass = YourWiFiPassword
```

Threshold and alarm values are in mmol/L (converted automatically if using mg/dL display). All settings can be edited live via the web UI.

### Touch Controls

The CoreS3 touch zones are at the bottom of the screen:

| Zone | Action |
|------|--------|
| Left third | Cycle brightness |
| Center third | Snooze alarms |
| Right third | Switch page |

### Building

Requires [PlatformIO](https://platformio.org/).

```bash
# Build firmware
cd cores3 && pio run

# Flash via USB (first time)
cd cores3 && pio run -t upload

# Run native tests
pio test -e native

# Serial monitor
cd cores3 && pio device monitor -b 115200
```

After the first flash, OTA updates are available by setting `upload_protocol = espota` and `upload_port = <device_name>.local` in `cores3/platformio.ini`.

### Architecture

- `cores3/src/` — CoreS3 firmware (display, WiFi, alerts, OTA, web config)
- `lib/ns_pure_logic/` — glucose color, alarm levels, formatting (no hardware deps)
- `lib/ns_json_parse/` — Nightscout API JSON parsing
- `lib/ns_display_model/` — display layout as pure data structs
- `lib/ns_config_parse/` — INI config parsing and serialization
- `test/native/` — 180+ native tests covering all pure logic

### License

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

See [LICENSE](LICENSE) for the full text.

# Rule 04: Configuration System

## Two Sources (with fallback)

1. **SD card INI file** (`/M5NS.INI`) — parsed at boot by `IniFile` library. Primary source.
2. **NVS Flash** (`Preferences` namespace `"M5NSconfig"`) — fallback if SD/INI missing. Also where the web config UI saves.

If neither has WiFi credentials, the device enters **bootstrap/AP mode**.

## Config Struct

All settings live in `tConfig cfg` (defined in `M5NSconfig.h`). Key fields:

- `url[128]` — Nightscout URL (auto-prefixed with `https://` if no scheme)
- `token[64]` — Nightscout security token
- `show_mgdl` — 0=mmol/L, 1=mg/dL
- `yellow_low/high`, `red_low/high` — display color thresholds (float, mmol/L)
- `snd_warning/alarm/warning_high/alarm_high` — audio alarm thresholds (float, mmol/L)
- `wlanssid[10][64]`, `wlanpass[10][64]` — up to 10 WiFi networks
- `LED_strip_pin/count/brightness` — NeoPixel config
- `vibration_mode/pin/strength` — vibration motor config
- `brightness1/2/3` — LCD brightness levels

## Live Data Struct

CGM readings live in `NSinfo ns` (defined in `M5NSconfig.h`):
- `sensSgv` / `sensSgvMgDl` — current glucose value
- `sensDir[32]` — trend direction string
- `last10sgv[10]` — mini-graph history
- `iob`, `cob`, `delta_*`, `loop_*`, `basal_*` — extended Nightscout data

## INI File Format

See `SD/M5NS.INI` for the example. Sections: `[M5NS]` for main config, `[wlan0]`–`[wlan9]` for WiFi networks.

## Adding a New Config Field

1. Add the field to `tConfig` in `M5NSconfig.h` (with default value)
2. Add INI reading in `readConfiguration()` in `M5NSconfig.cpp`
3. Add NVS read/write in `readConfigFromFlash()` / `saveConfigToFlash()` in `M5NSconfig.cpp`
4. Add to the web config UI in `M5NSWebConfig.cpp` (HTML form + `/savecfg` handler)
5. Update `SD/M5NS.INI` with the new field and a comment

# Rule 05: Display and UI

## Hardware

320x240 TFT LCD (ILI9341) via M5Stack library (wraps TFT_eSPI).

## Pages

4 display pages, cycled by Button C (short press):

| Page | Content |
|------|---------|
| 0 | Main: date/time, username, delta, IOB/COB, big SGV, trend arrow, mini-graph, sensor age, info line |
| 1 | Large SGV: oversized glucose value centered, time, delta, trend arrow |
| 2 | Analog clock: clock face with CGM-colored ring, temp/humidity (non-Core2), mini trend arrow |
| 3 (MAX_PAGE) | Error log: last 6 errors, free heap, uptime, IP/mDNS, firmware version |

## Fonts

TFT_eSPI GFX Free Fonts. Aliases in `Free_Fonts.h`:
- Common: `FSSB12`, `FSSB24`, `FSSB18` (FreeSansBold), `FSS9` (FreeSans), `FMB9`/`FM9` (FreeMono)
- Rendering: `M5.Lcd.setFreeFont(font)` then `M5.Lcd.drawString(text, x, y, GFXFF)`

## Icons

16x16 monochrome bitmaps in `iot_iconset_16x16.c`, rendered pixel-by-pixel via `drawIcon()` using `pgm_read_byte()` + `M5.Lcd.drawPixel()`.

## Color Coding

| Color | Meaning |
|-------|---------|
| Green (TFT_GREEN) | Within normal range |
| Yellow (TFT_YELLOW) | Between yellow_low/yellow_high thresholds |
| Red (TFT_RED) | Below red_low or above red_high |
| Light grey → white → red | Time-since-reading indicator (fresh → >5min → >15min) |

## Trend Arrow

`drawArrow()` — filled triangle + thick line, angle mapped from Nightscout direction strings (DoubleDown through DoubleUp, 7 states).

## Key Drawing Functions

- `draw_page()` — full page redraw (dispatches by `dispPage`)
- `drawMiniGraph()` — last 10 SGV readings, colored circles, 88x90px area
- `drawAnalogClock()` — incremental hand redraw on page 2
- `displayCurrentTimeLCD()` — time update on page 0
- `handleAlarmsInfoLine()` — bottom 20px alarm/snooze status bar

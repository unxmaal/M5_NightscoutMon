# Rule 07: Alarms and Audio

## Alarm Thresholds (all in mmol/L internally)

| Alarm | Config field | Default | Sound |
|-------|-------------|---------|-------|
| Low alarm | `snd_alarm` | 3.0 | 6x 660Hz beeps, red LEDs |
| Low warning | `snd_warning` | 3.7 | 3x 3000Hz beeps, amber LEDs |
| High alarm | `snd_alarm_high` | 20 | Same as low alarm |
| High warning | `snd_warning_high` | 14 | Same as low warning |
| No readings | `snd_no_readings` | 20 min | Warning after N minutes without new data |
| Loop error | `snd_loop_error` | 1 (on) | When loop status contains "Err" |

## Audio Hardware Differences

| Feature | BASIC/GRAY/FIRE | Core2 |
|---------|----------------|-------|
| Method | DAC (`dacWrite`) + `ledcWriteTone` | I2S DMA (`i2s_write`) |
| Sample rate | 5000 Hz | 11025 Hz |
| Buffer type | `uint8_t[25000]` | `int16_t[25000]` |
| Volume control | `alarm_volume`, `warning_volume` (0-100) | Same |

## Snooze

- Button B cycles snooze: 1x → 2x → 3x → 4x `snooze_timeout` minutes (press within 2s to stack)
- Snooze state broadcast via UDP to other M5Stacks on same LAN
- Snooze survives soft restart via NVS `SnoozeUntil` key
- Bottom status bar shows snooze countdown (yellow background)

## LED Strip

- `LED_strip_mode`: 0=off, 1=visualize sound, 2=alarms only, 3=always green
- Pin/count/brightness configurable
- Core2: forced off (`cfg.LED_strip_mode = 0`)

## Vibration Motor

- `vibration_mode`: 0=off, 1=vibrate during sound
- PWM on configurable pin (default GPIO 26), 10-bit resolution
- Core2: forced off

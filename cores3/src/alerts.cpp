#include "alerts.h"
#include "ns_pure_logic.h"

#include <M5Unified.h>
#include <time.h>

/* ── AlarmState ────────────────────────────────────────────────── */

void AlarmState::snooze(int timeout_min) {
    snoozeMult++;
    struct tm now;
    if (getLocalTime(&now)) {
        snoozeUntil = mktime(&now) + (timeout_min * snoozeMult * 60);
    }
}

int AlarmState::snoozeRemaining() const {
    struct tm now;
    if (!getLocalTime(&now))
        return 0;
    int rem = (int)difftime(snoozeUntil, mktime(&now));
    return (rem > 0) ? rem : 0;
}

bool AlarmState::shouldFire(int alarm_repeat_min) const {
    struct tm now;
    if (!getLocalTime(&now))
        return false;
    int elapsed = (int)difftime(mktime(&now), lastAlarmTime);
    return elapsed > (alarm_repeat_min * 60);
}

void AlarmState::recordFired() {
    struct tm now;
    if (getLocalTime(&now))
        lastAlarmTime = mktime(&now);
}

/* ── Sound helpers ─────────────────────────────────────────────── */

static void playAlarm(int volume) {
    M5.Speaker.setVolume(volume);
    // Urgent: three short high beeps
    for (int i = 0; i < 3; i++) {
        M5.Speaker.tone(2000, 200);
        delay(300);
    }
}

static void playWarning(int volume) {
    M5.Speaker.setVolume(volume);
    // Less urgent: two lower beeps
    for (int i = 0; i < 2; i++) {
        M5.Speaker.tone(1000, 200);
        delay(400);
    }
}

/* ── Check and fire alarms ─────────────────────────────────────── */

void checkAlarms(const Config &cfg, const NSinfo &ns, AlarmState &alarm) {
    // Calculate sensor age
    struct tm now;
    unsigned int sensorAgeMin = 999;
    if (getLocalTime(&now)) {
        int ageSec = (int)difftime(mktime(&now), ns.sensTime);
        sensorAgeMin = (ageSec + 30) / 60;
    }

    bool loopErr = false;  // TODO: wire up if loop status is added later

    int level = alarmLevel(ns.sensSgv, cfg.snd_alarm, cfg.snd_warning,
                           cfg.snd_alarm_high, cfg.snd_warning_high,
                           sensorAgeMin, cfg.snd_no_readings, loopErr);

    if (level == ALARM_LEVEL_NORMAL)
        return;

    // Don't fire during snooze
    if (alarm.snoozeRemaining() > 0)
        return;

    // Don't fire too frequently
    if (!alarm.shouldFire(cfg.alarm_repeat))
        return;

    // Fire
    switch (level) {
        case ALARM_LEVEL_LOW_ALARM:
        case ALARM_LEVEL_HIGH_ALARM:
        case ALARM_LEVEL_LOOP_ERROR:
            playAlarm(cfg.alarm_volume);
            break;
        case ALARM_LEVEL_LOW_WARNING:
        case ALARM_LEVEL_HIGH_WARNING:
        case ALARM_LEVEL_NO_READINGS:
            playWarning(cfg.warning_volume);
            break;
    }
    alarm.recordFired();
}

/* ── Alarm info line ───────────────────────────────────────────── */

void drawAlarmInfoLine(const Config &cfg, const NSinfo &ns,
                       const AlarmState &alarm) {
    // Calculate sensor age
    struct tm now;
    unsigned int sensorAgeMin = 999;
    if (getLocalTime(&now)) {
        int ageSec = (int)difftime(mktime(&now), ns.sensTime);
        sensorAgeMin = (ageSec + 30) / 60;
    }

    bool loopErr = false;

    int level = alarmLevel(ns.sensSgv, cfg.snd_alarm, cfg.snd_warning,
                           cfg.snd_alarm_high, cfg.snd_warning_high,
                           sensorAgeMin, cfg.snd_no_readings, loopErr);

    int snoozeRem = alarm.snoozeRemaining();

    // Status bar text
    char statusStr[16];
    if (snoozeRem > 0)
        snprintf(statusStr, sizeof(statusStr), "%d", (snoozeRem + 59) / 60);
    else
        strlcpy(statusStr, "", sizeof(statusStr));

    // Bar background color
    uint16_t bgColor = TFT_BLACK;
    uint16_t fgColor = TFT_LIGHTGREY;

    switch (level) {
        case ALARM_LEVEL_LOW_ALARM:
        case ALARM_LEVEL_HIGH_ALARM:
        case ALARM_LEVEL_LOOP_ERROR:
            bgColor = TFT_RED;
            fgColor = TFT_BLACK;
            break;
        case ALARM_LEVEL_LOW_WARNING:
        case ALARM_LEVEL_HIGH_WARNING:
        case ALARM_LEVEL_NO_READINGS:
            bgColor = TFT_YELLOW;
            fgColor = TFT_BLACK;
            break;
        default:
            break;
    }

    M5.Display.fillRect(0, 220, 320, 20, bgColor);
    if (statusStr[0] != '\0') {
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.setTextColor(fgColor, bgColor);
        M5.Display.setFont(&FreeSansBold12pt7b);
        M5.Display.setTextSize(1);
        M5.Display.drawString(statusStr, 160, 230);
    }
}

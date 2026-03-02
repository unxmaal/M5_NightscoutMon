#include "display.h"
#include "ns_display_model.h"
#include "ns_pure_logic.h"

#include <M5Unified.h>
#include <WiFi.h>
#include <stdio.h>
#include <math.h>

/* ── Color mapping: model COLOR_* → TFT hardware colors ──────── */

static uint16_t mapColor(int c) {
    switch (c) {
        case COLOR_BLACK:     return TFT_BLACK;
        case COLOR_GREEN:     return TFT_GREEN;
        case COLOR_YELLOW:    return TFT_YELLOW;
        case COLOR_RED:       return TFT_RED;
        case COLOR_WHITE:     return TFT_WHITE;
        case COLOR_LIGHTGREY: return TFT_LIGHTGREY;
        default:              return TFT_WHITE;
    }
}

/* ── Font mapping: model FONT_* → actual font objects ─────────── */

static const lgfx::GFXfont* mapFont(int f) {
    switch (f) {
        case FONT_SANS_BOLD_24: return &FreeSansBold24pt7b;
        case FONT_SANS_BOLD_18: return &FreeSansBold18pt7b;
        case FONT_SANS_BOLD_12: return &FreeSansBold12pt7b;
        case FONT_SANS_9:       return &FreeSans9pt7b;
        case FONT_MONO_9:       return &FreeMono9pt7b;
        default:                return &FreeSans9pt7b;
    }
}

/* ── Trend arrow ───────────────────────────────────────────────── */

void drawArrow(int x, int y, int size, int angle, uint16_t color) {
    if (angle == 180)
        return;  // no arrow for unknown direction

    float rad = (angle + 85) * M_PI / 180.0f;
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    // Arrow shaft endpoint
    int tipX = x + (int)(size * 2 * cosA);
    int tipY = y + (int)(size * 2 * sinA);

    // Draw shaft
    M5.Display.drawLine(x, y, tipX, tipY, color);

    // Arrowhead — two lines from tip at ±30 degrees
    float headLen = size * 0.8f;
    for (int sign = -1; sign <= 1; sign += 2) {
        float headRad = rad + sign * (M_PI / 6.0f) + M_PI;
        int hx = tipX + (int)(headLen * cosf(headRad));
        int hy = tipY + (int)(headLen * sinf(headRad));
        M5.Display.drawLine(tipX, tipY, hx, hy, color);
    }
}

/* ── Battery icon ──────────────────────────────────────────────── */

static void drawBattery(int x, int y, int pct) {
    uint16_t color = TFT_GREEN;
    if (pct < 0) {
        color = TFT_LIGHTGREY;  // unknown
        pct = 0;
    } else if (pct < 25) {
        color = TFT_RED;
    } else if (pct < 50) {
        color = TFT_YELLOW;
    }

    // Battery outline
    M5.Display.drawRect(x, y, 20, 10, TFT_LIGHTGREY);
    M5.Display.fillRect(x + 20, y + 2, 2, 6, TFT_LIGHTGREY);

    // Fill
    int fillW = (16 * pct) / 100;
    if (fillW > 0)
        M5.Display.fillRect(x + 2, y + 2, fillW, 6, color);
}

/* ── Page 0: Large glucose ─────────────────────────────────────── */

void drawGlucosePage(const Config &cfg, const NSinfo &ns, const ErrorLog &errLog) {
    // Build the display model — all decisions happen here
    struct tm now;
    long now_sec = 0;
    int time_hour = ns.sensTm.tm_hour;
    int time_min  = ns.sensTm.tm_min;

    if (getLocalTime(&now)) {
        now_sec = mktime(&now);
        if (cfg.show_current_time) {
            time_hour = now.tm_hour;
            time_min  = now.tm_min;
        }
    }

    int batteryPct = M5.Power.getBatteryLevel();

    // Compute alarm level for the model
    unsigned int sensorAgeMin = 999;
    if (now_sec > 0 && ns.sensTime > 0) {
        int ageSec = (int)(now_sec - ns.sensTime);
        sensorAgeMin = (ageSec + 30) / 60;
    }
    int level = alarmLevel(ns.sensSgv, cfg.snd_alarm, cfg.snd_warning,
                           cfg.snd_alarm_high, cfg.snd_warning_high,
                           sensorAgeMin, cfg.snd_no_readings, false);

    // Compute snooze remaining (passed in from alarm state via info line)
    // For now, alarm bar is driven by level alone; snooze handled by drawAlarmInfoLine
    int snoozeRem = 0;

    GlucosePageModel model;
    buildGlucoseModel(&model,
        ns.sensSgv, ns.sensSgvMgDl, cfg.show_mgdl,
        ns.sensDir, ns.arrowAngle,
        ns.delta_display,
        time_hour, time_min,
        now_sec, (long)ns.sensTime,
        cfg.yellow_low, cfg.yellow_high, cfg.red_low, cfg.red_high,
        level, snoozeRem,
        batteryPct, errLog.count);

    // ── Render from model ─────────────────────────────────────────

    M5.Display.fillRect(0, 0, 320, 240, TFT_BLACK);

    // Top bar: time (left), delta (right)
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    M5.Display.setFont(&FreeSansBold24pt7b);
    M5.Display.drawString(model.time_str, 0, 0);

    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.drawString(model.delta_str, 180, 0);

    // Center: glucose value
    M5.Display.setTextColor(mapColor(model.glucose_color), TFT_BLACK);
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.setTextSize(4);
    M5.Display.setFont(mapFont(model.glucose_font));
    M5.Display.drawString(model.glucose_str, 160, 120);
    M5.Display.setTextSize(1);

    // Trend arrow
    int arrowY = 0;
    if (model.arrow_angle >= 45)
        arrowY = 4;
    else if (model.arrow_angle > -45)
        arrowY = 18;
    else
        arrowY = 30;
    drawArrow(280, arrowY, 10, model.arrow_angle, mapColor(model.arrow_color));

    // Sensor staleness
    if (model.show_age) {
        M5.Display.setTextDatum(TR_DATUM);
        M5.Display.setTextColor(mapColor(model.age_color), TFT_BLACK);
        M5.Display.setFont(&FreeSans9pt7b);
        M5.Display.drawString(model.age_str, 318, 45);
    }

    // Bottom bar
    drawBattery(296, 226, model.battery_pct);

    if (model.show_error_badge) {
        M5.Display.setTextDatum(TL_DATUM);
        M5.Display.setTextColor(TFT_RED, TFT_BLACK);
        M5.Display.setFont(&FreeSans9pt7b);
        M5.Display.drawString("!", 2, 224);
    }

    // Alarm bar
    if (model.show_alarm_bar) {
        M5.Display.fillRect(0, 220, 320, 20, mapColor(model.alarm_bar_bg));
        if (model.alarm_bar_text[0] != '\0') {
            M5.Display.setTextDatum(MC_DATUM);
            M5.Display.setTextColor(mapColor(model.alarm_bar_fg),
                                    mapColor(model.alarm_bar_bg));
            M5.Display.setFont(&FreeSansBold12pt7b);
            M5.Display.drawString(model.alarm_bar_text, 160, 230);
        }
    }
}

/* ── Page 1: Error log / status ────────────────────────────────── */

void drawStatusPage(const Config &cfg, const NSinfo &ns, const ErrorLog &errLog) {
    (void)cfg; (void)ns;

    // Prepare error data for the model
    int codes[STATUS_MAX_ERRORS];
    char dates[STATUS_MAX_ERRORS][16];
    int displayCount = errLog.ptr;
    if (displayCount > STATUS_MAX_ERRORS)
        displayCount = STATUS_MAX_ERRORS;

    for (int i = 0; i < displayCount; i++) {
        codes[i] = errLog.entries[i].err_code;
        snprintf(dates[i], sizeof(dates[i]), "%02d.%02d.%02d:%02d",
                 errLog.entries[i].err_time.tm_mday,
                 errLog.entries[i].err_time.tm_mon + 1,
                 errLog.entries[i].err_time.tm_hour,
                 errLog.entries[i].err_time.tm_min);
    }

    char ipStr[32];
    IPAddress ip = WiFi.localIP();
    snprintf(ipStr, sizeof(ipStr), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);

    StatusPageModel model;
    buildStatusModel(&model,
        codes, dates, displayCount, errLog.count,
        ESP.getFreeHeap(), millis(),
        ipStr, "CoreS3",
        M5.Power.getBatteryLevel());

    // ── Render from model ─────────────────────────────────────────

    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.setTextSize(1);

    // Header
    M5.Display.setFont(&FreeMono9pt7b);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.drawString("Date  Time  Error Log", 0, 0);

    if (model.display_count == 0) {
        M5.Display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        M5.Display.drawString("no errors in log", 0, 20);
    } else {
        for (int i = 0; i < model.display_count; i++) {
            M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
            M5.Display.drawString(model.errors[i].date_str, 0, 20 + i * 18);

            M5.Display.setTextColor(mapColor(model.errors[i].color), TFT_BLACK);
            M5.Display.drawString(model.errors[i].desc_str, 132, 20 + i * 18);
        }

        M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
        char countStr[32];
        snprintf(countStr, sizeof(countStr), "Total errors %d", model.error_count);
        M5.Display.drawString(countStr, 0, 20 + model.display_count * 18);
    }

    // System info
    int infoY = 20 + 7 * 18;
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.drawString(model.heap_str, 0, infoY);
    M5.Display.drawString(model.uptime_str, 0, infoY + 18);
    M5.Display.drawString(model.ip_str, 0, infoY + 36);
    M5.Display.drawString(model.version_str, 0, infoY + 54);

    drawBattery(296, 226, model.battery_pct);
}

/* ── Page dispatcher ───────────────────────────────────────────── */

void drawPage(int page, const Config &cfg, const NSinfo &ns, const ErrorLog &errLog) {
    switch (page) {
        case PAGE_GLUCOSE: drawGlucosePage(cfg, ns, errLog); break;
        case PAGE_STATUS:  drawStatusPage(cfg, ns, errLog); break;
        default:           drawGlucosePage(cfg, ns, errLog); break;
    }
}

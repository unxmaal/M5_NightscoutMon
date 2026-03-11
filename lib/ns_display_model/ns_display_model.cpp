/*  ns_display_model.cpp — Display model builders
 *
 *  Copyright (C) 2024-2026 Eric Dodd <eric.e.dodd@gmail.com>
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ns_display_model.h"
#include "ns_pure_logic.h"
#include <string.h>
#include <stdio.h>

/* ── Error code → description ──────────────────────────────────── */

static void errorCodeToString(int code, char *buf, size_t bufsize) {
    switch (code) {
        case 1001: strlcpy(buf, "JSON parse failed", bufsize); break;
        case 1002: strlcpy(buf, "No data from NS", bufsize); break;
        case 1003: strlcpy(buf, "JSON2 parse failed", bufsize); break;
        default:
            if (code < 0)
                snprintf(buf, bufsize, "HTTP err %d", code);
            else
                snprintf(buf, bufsize, "HTTP %d", code);
            break;
    }
}

/* ── Alarm bar colors ──────────────────────────────────────────── */

static void alarmBarColors(int alarm_level, int *bg, int *fg) {
    switch (alarm_level) {
        case ALARM_LEVEL_LOW_ALARM:
        case ALARM_LEVEL_HIGH_ALARM:
        case ALARM_LEVEL_LOOP_ERROR:
            *bg = COLOR_RED;
            *fg = COLOR_BLACK;
            break;
        case ALARM_LEVEL_LOW_WARNING:
        case ALARM_LEVEL_HIGH_WARNING:
        case ALARM_LEVEL_NO_READINGS:
            *bg = COLOR_YELLOW;
            *fg = COLOR_BLACK;
            break;
        default:
            *bg = COLOR_BLACK;
            *fg = COLOR_LIGHTGREY;
            break;
    }
}

/* ── Glucose page ──────────────────────────────────────────────── */

void buildGlucoseModel(
    GlucosePageModel *model,
    float sgv_mmol, float sgv_mgdl, bool show_mgdl,
    const char * /* direction */, int arrow_angle,
    const char *delta_display,
    int time_hour, int time_min,
    long now_sec, long sensor_time_sec,
    float yellow_low, float yellow_high,
    float red_low, float red_high,
    int alarm_level, int snooze_remaining_sec,
    int battery_pct, int error_count
) {
    memset(model, 0, sizeof(*model));

    /* Time */
    snprintf(model->time_str, sizeof(model->time_str), "%02d:%02d",
             time_hour, time_min);

    /* Glucose */
    model->glucose_font = formatGlucose(model->glucose_str,
                                         sizeof(model->glucose_str),
                                         sgv_mmol, sgv_mgdl, show_mgdl);
    /* Map font hint to model font */
    model->glucose_font = (model->glucose_font == FONT_MEDIUM)
                          ? FONT_SANS_BOLD_18 : FONT_SANS_BOLD_24;

    int cl = glucoseColor(sgv_mmol, yellow_low, yellow_high, red_low, red_high);
    switch (cl) {
        case GLUCOSE_COLOR_YELLOW: model->glucose_color = COLOR_YELLOW; break;
        case GLUCOSE_COLOR_RED:    model->glucose_color = COLOR_RED; break;
        default:                   model->glucose_color = COLOR_GREEN; break;
    }

    /* Delta */
    strlcpy(model->delta_str, delta_display ? delta_display : "", sizeof(model->delta_str));

    /* Arrow */
    model->arrow_angle = arrow_angle;
    model->arrow_color = model->glucose_color;

    /* Sensor staleness */
    if (now_sec > 0 && sensor_time_sec > 0) {
        long age_sec = now_sec - sensor_time_sec;
        int age_min = (int)((age_sec + 30) / 60);
        if (age_min > SENSOR_AGE_STALE_MIN) {
            model->show_age = true;
            snprintf(model->age_str, sizeof(model->age_str), "%d min", age_min);
            model->age_color = (age_min > SENSOR_AGE_CRITICAL_MIN) ? COLOR_RED : COLOR_WHITE;
        }
    }

    /* Battery */
    model->battery_pct = battery_pct;

    /* Error badge */
    model->show_error_badge = (error_count > 0);

    /* Alarm bar */
    int bar_bg, bar_fg;
    alarmBarColors(alarm_level, &bar_bg, &bar_fg);
    model->alarm_bar_bg = bar_bg;
    model->alarm_bar_fg = bar_fg;
    model->show_alarm_bar = (alarm_level != ALARM_LEVEL_NORMAL || snooze_remaining_sec > 0);
    if (snooze_remaining_sec > 0) {
        snprintf(model->alarm_bar_text, sizeof(model->alarm_bar_text),
                 "%d", (snooze_remaining_sec + 59) / 60);
    }
}

/* ── Sparkline ─────────────────────────────────────────────────── */

void buildSparklineModel(
    SparklineModel *model,
    const float *sgv_mgdl, int count,
    int area_x, int area_y, int area_w, int area_h,
    float yellow_low, float yellow_high,
    float red_low, float red_high
) {
    memset(model, 0, sizeof(*model));

    if (!sgv_mgdl || count < 2 || area_w <= 0 || area_h <= 0)
        return;

    if (count > SPARKLINE_MAX_POINTS)
        count = SPARKLINE_MAX_POINTS;

    // Find min/max across all values
    float vmin = sgv_mgdl[0], vmax = sgv_mgdl[0];
    for (int i = 1; i < count; i++) {
        if (sgv_mgdl[i] < vmin) vmin = sgv_mgdl[i];
        if (sgv_mgdl[i] > vmax) vmax = sgv_mgdl[i];
    }

    // Minimum range of 20 mg/dL to avoid flat line
    float range = vmax - vmin;
    if (range < 20.0f) {
        float mid = (vmin + vmax) / 2.0f;
        vmin = mid - 10.0f;
        vmax = mid + 10.0f;
    }

    // 10% padding on each side
    float pad = (vmax - vmin) * 0.1f;
    vmin -= pad;
    vmax += pad;
    range = vmax - vmin;

    model->count = count;

    for (int i = 0; i < count; i++) {
        // X: newest (i=0) on the right, oldest on left
        if (count == 1)
            model->points[i].x = area_x + area_w / 2;
        else
            model->points[i].x = area_x + area_w - 1
                                 - (i * (area_w - 1)) / (count - 1);

        // Y: high glucose = top (low Y), low glucose = bottom (high Y)
        float norm = (sgv_mgdl[i] - vmin) / range;  // 0..1
        model->points[i].y = area_y + area_h - 1
                             - (int)(norm * (area_h - 1));

        // Color based on thresholds
        float mmol = sgv_mgdl[i] / 18.0f;
        int cl = glucoseColor(mmol, yellow_low, yellow_high, red_low, red_high);
        switch (cl) {
            case GLUCOSE_COLOR_YELLOW: model->points[i].color = COLOR_YELLOW; break;
            case GLUCOSE_COLOR_RED:    model->points[i].color = COLOR_RED; break;
            default:                   model->points[i].color = COLOR_GREEN; break;
        }
    }
}

/* ── Status page ───────────────────────────────────────────────── */

void buildStatusModel(
    StatusPageModel *model,
    const int *err_codes, const char (*err_dates)[16],
    int err_display_count, int err_total_count,
    unsigned long heap_free, unsigned long uptime_ms,
    const char *ip_str, const char *version,
    int battery_pct
) {
    memset(model, 0, sizeof(*model));

    model->error_count = err_total_count;
    model->display_count = err_display_count;
    if (model->display_count > STATUS_MAX_ERRORS)
        model->display_count = STATUS_MAX_ERRORS;
    if (err_codes == NULL || err_dates == NULL)
        model->display_count = 0;

    for (int i = 0; i < model->display_count; i++) {
        strlcpy(model->errors[i].date_str, err_dates[i],
                sizeof(model->errors[i].date_str));
        errorCodeToString(err_codes[i], model->errors[i].desc_str,
                          sizeof(model->errors[i].desc_str));
        model->errors[i].color = (err_codes[i] < 0) ? COLOR_RED : COLOR_YELLOW;
    }

    snprintf(model->heap_str, sizeof(model->heap_str),
             "Free Heap = %lu", heap_free);

    char uptimeBuf[32];
    formatUptime(uptimeBuf, sizeof(uptimeBuf), uptime_ms);
    snprintf(model->uptime_str, sizeof(model->uptime_str),
             "Up time = %s", uptimeBuf);

    strlcpy(model->ip_str, ip_str ? ip_str : "0.0.0.0", sizeof(model->ip_str));
    strlcpy(model->version_str, version ? version : "?", sizeof(model->version_str));
    model->battery_pct = battery_pct;
}

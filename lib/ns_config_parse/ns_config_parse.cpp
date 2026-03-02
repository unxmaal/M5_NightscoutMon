/*  ns_config_parse.cpp — INI config file parsing
 *
 *  Copyright (C) 2024-2026 Eric Dodd <eric.e.dodd@gmail.com>
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ns_config_parse.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

/* ── Defaults ──────────────────────────────────────────────────── */

void configDefaults(ParsedConfig *cfg) {
    memset(cfg, 0, sizeof(*cfg));
    strlcpy(cfg->deviceName, "NightscoutMon", sizeof(cfg->deviceName));
    cfg->timeZone          = 3600;
    cfg->yellow_low        = 4.5f;
    cfg->yellow_high       = 9.0f;
    cfg->red_low           = 3.9f;
    cfg->red_high          = 11.0f;
    cfg->snd_alarm         = 3.0f;
    cfg->snd_warning       = 3.7f;
    cfg->snd_alarm_high    = 20.0f;
    cfg->snd_warning_high  = 14.0f;
    cfg->snd_no_readings   = 20;
    cfg->snooze_timeout    = 30;
    cfg->alarm_repeat      = 5;
    cfg->warning_volume    = 30;
    cfg->alarm_volume      = 100;
    cfg->brightness1       = 10;
    cfg->brightness2       = 50;
    cfg->brightness3       = 100;
    cfg->info_line         = 1;
    cfg->snd_loop_error    = 1;
    strlcpy(cfg->restart_at_time, "NORES", sizeof(cfg->restart_at_time));
}

/* ── Helpers ───────────────────────────────────────────────────── */

static char *trimWhitespace(char *s) {
    while (isspace((unsigned char)*s)) s++;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) *end-- = '\0';
    return s;
}

/* ── Parsing ───────────────────────────────────────────────────── */

int parseConfigBuffer(char *buf, size_t len, ParsedConfig *cfg) {
    int parsed = 0;
    int currentWlan = -1;  // -1 = [config] section, 0-9 = wlan index

    char *p = buf;
    char *end = buf + len;

    while (p < end) {
        // Find end of line
        char *eol = p;
        while (eol < end && *eol != '\n' && *eol != '\r')
            eol++;

        // Null-terminate this line
        if (eol < end) *eol = '\0';

        char *line = trimWhitespace(p);

        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == ';' || line[0] == '#') {
            p = eol + 1;
            continue;
        }

        // Section header
        if (line[0] == '[') {
            char *close = strchr(line, ']');
            if (close) {
                *close = '\0';
                char *section = line + 1;
                if (strcmp(section, "config") == 0) {
                    currentWlan = -1;
                } else if (strncmp(section, "wlan", 4) == 0) {
                    int idx = atoi(section + 4) - 1;  // wlan1 → 0
                    if (idx >= 0 && idx < CFG_MAX_WLAN)
                        currentWlan = idx;
                }
            }
            p = eol + 1;
            continue;
        }

        // Key = value
        char *eq = strchr(line, '=');
        if (!eq) {
            p = eol + 1;
            continue;
        }

        *eq = '\0';
        char *key = trimWhitespace(line);
        char *val = trimWhitespace(eq + 1);

        if (currentWlan >= 0 && currentWlan < CFG_MAX_WLAN) {
            // WiFi section
            if (strcmp(key, "ssid") == 0) {
                strlcpy(cfg->wlanssid[currentWlan], val, 64);
                parsed++;
            } else if (strcmp(key, "pass") == 0) {
                strlcpy(cfg->wlanpass[currentWlan], val, 64);
                parsed++;
            }
        } else {
            // [config] section
            if (strcmp(key, "nightscout") == 0) {
                strlcpy(cfg->url, val, sizeof(cfg->url));
                parsed++;
            } else if (strcmp(key, "token") == 0) {
                strlcpy(cfg->token, val, sizeof(cfg->token));
                parsed++;
            } else if (strcmp(key, "name") == 0) {
                strlcpy(cfg->userName, val, sizeof(cfg->userName));
                parsed++;
            } else if (strcmp(key, "device_name") == 0) {
                strlcpy(cfg->deviceName, val, sizeof(cfg->deviceName));
                parsed++;
            } else if (strcmp(key, "time_zone") == 0) {
                cfg->timeZone = atoi(val); parsed++;
            } else if (strcmp(key, "dst") == 0) {
                cfg->dst = atoi(val); parsed++;
            } else if (strcmp(key, "show_mgdl") == 0) {
                cfg->show_mgdl = atoi(val); parsed++;
            } else if (strcmp(key, "show_current_time") == 0) {
                cfg->show_current_time = atoi(val); parsed++;
            } else if (strcmp(key, "default_page") == 0) {
                cfg->default_page = atoi(val); parsed++;
            } else if (strcmp(key, "sgv_only") == 0) {
                cfg->sgv_only = atoi(val); parsed++;
            } else if (strcmp(key, "info_line") == 0) {
                cfg->info_line = atoi(val); parsed++;
            } else if (strcmp(key, "date_format") == 0) {
                cfg->date_format = atoi(val); parsed++;
            } else if (strcmp(key, "time_format") == 0) {
                cfg->time_format = atoi(val); parsed++;
            } else if (strcmp(key, "yellow_low") == 0) {
                cfg->yellow_low = (float)atof(val); parsed++;
            } else if (strcmp(key, "yellow_high") == 0) {
                cfg->yellow_high = (float)atof(val); parsed++;
            } else if (strcmp(key, "red_low") == 0) {
                cfg->red_low = (float)atof(val); parsed++;
            } else if (strcmp(key, "red_high") == 0) {
                cfg->red_high = (float)atof(val); parsed++;
            } else if (strcmp(key, "snd_alarm") == 0) {
                cfg->snd_alarm = (float)atof(val); parsed++;
            } else if (strcmp(key, "snd_warning") == 0) {
                cfg->snd_warning = (float)atof(val); parsed++;
            } else if (strcmp(key, "snd_alarm_high") == 0) {
                cfg->snd_alarm_high = (float)atof(val); parsed++;
            } else if (strcmp(key, "snd_warning_high") == 0) {
                cfg->snd_warning_high = (float)atof(val); parsed++;
            } else if (strcmp(key, "snd_no_readings") == 0) {
                cfg->snd_no_readings = atoi(val); parsed++;
            } else if (strcmp(key, "snooze_timeout") == 0) {
                cfg->snooze_timeout = atoi(val); parsed++;
            } else if (strcmp(key, "alarm_repeat") == 0) {
                cfg->alarm_repeat = atoi(val); parsed++;
            } else if (strcmp(key, "warning_volume") == 0) {
                cfg->warning_volume = atoi(val); parsed++;
            } else if (strcmp(key, "alarm_volume") == 0) {
                cfg->alarm_volume = atoi(val); parsed++;
            } else if (strcmp(key, "brightness1") == 0) {
                cfg->brightness1 = atoi(val); parsed++;
            } else if (strcmp(key, "brightness2") == 0) {
                cfg->brightness2 = atoi(val); parsed++;
            } else if (strcmp(key, "brightness3") == 0) {
                cfg->brightness3 = atoi(val); parsed++;
            } else if (strcmp(key, "restart_at_logged_errors") == 0) {
                cfg->restart_at_logged_errors = atoi(val); parsed++;
            } else if (strcmp(key, "restart_at_time") == 0) {
                strlcpy(cfg->restart_at_time, val, sizeof(cfg->restart_at_time)); parsed++;
            } else if (strcmp(key, "snd_loop_error") == 0) {
                cfg->snd_loop_error = atoi(val); parsed++;
            }
        }

        p = eol + 1;
    }

    return parsed;
}

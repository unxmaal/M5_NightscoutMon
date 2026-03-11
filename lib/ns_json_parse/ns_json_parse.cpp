/*  ns_json_parse.cpp — Nightscout JSON response parsing
 *
 *  Copyright (C) 2024-2026 Eric Dodd <eric.e.dodd@gmail.com>
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ns_json_parse.h"
#include "ns_pure_logic.h"

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <string.h>
#include <stdio.h>

/* ── Shared SGV entry extraction ───────────────────────────────── */

static void fillSGVEntry(SGVEntry *entry, JsonObject obj) {
    strlcpy(entry->device, obj["device"] | "N/A", sizeof(entry->device));
    entry->date_ms  = static_cast<uint64_t>(obj["date"].as<long long>());
    entry->date_sec = entry->date_ms / 1000;

    // Direction: try "direction", fall back to numeric "trend"
    const char *dir = obj["direction"] | "";
    if (dir[0] == '\0' || strcmp(dir, "N/A") == 0) {
        if (obj["trend"].is<int>()) {
            int trend = obj["trend"].as<int>();
            switch (trend) {
                case 1: dir = "DoubleUp"; break;
                case 2: dir = "SingleUp"; break;
                case 3: dir = "FortyFiveUp"; break;
                case 4: dir = "Flat"; break;
                case 5: dir = "FortyFiveDown"; break;
                case 6: dir = "SingleDown"; break;
                case 7: dir = "DoubleDown"; break;
                default: dir = "NONE"; break;
            }
        } else {
            const char *trendStr = obj["trend"] | "";
            if (trendStr[0] != '\0')
                dir = trendStr;
            else
                dir = "NONE";
        }
    }
    strlcpy(entry->direction, dir, sizeof(entry->direction));

    entry->sgv_mgdl = obj["sgv"].as<float>();
    entry->sgv_mmol = entry->sgv_mgdl / 18.0f;
    entry->arrow_angle = directionToAngle(entry->direction);
}

/* ── Nightscout SGV response (single entry) ───────────────────── */

int parseSGVResponse(const char *json, size_t len, SGVEntry *entry) {
    int rc;
    int count = parseSGVResponseMulti(json, len, entry, 1, &rc);
    (void)count;
    return rc;
}

/* ── Nightscout SGV response (multi entry) ────────────────────── */

int parseSGVResponseMulti(const char *json, size_t len,
                          SGVEntry *entries, int max_entries,
                          int *parse_rc) {
    if (!json || !entries || !parse_rc || max_entries <= 0) {
        if (parse_rc) *parse_rc = PARSE_ERR_JSON;
        return 0;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json, len);
    if (err) {
        *parse_rc = PARSE_ERR_JSON;
        return 0;
    }

    JsonArray arr = doc.as<JsonArray>();
    if (arr.isNull() || arr.size() == 0) {
        *parse_rc = PARSE_ERR_EMPTY;
        return 0;
    }

    int found = 0;
    for (int i = 0; i < (int)arr.size() && found < max_entries; i++) {
        if (arr[i]["sgv"].is<float>() || arr[i]["sgv"].is<int>()) {
            fillSGVEntry(&entries[found], arr[i].as<JsonObject>());
            found++;
        }
    }

    if (found == 0) {
        *parse_rc = PARSE_ERR_NO_SGV;
        return 0;
    }

    *parse_rc = PARSE_OK;
    return found;
}

/* ── Sugarmate response ────────────────────────────────────────── */

int parseSugarmateResponse(const char *json, size_t len, SGVEntry *entry,
                           DeltaInfo *delta) {
    if (!json || !entry || !delta)
        return PARSE_ERR_JSON;

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json, len);
    if (err)
        return PARSE_ERR_JSON;

    strlcpy(entry->device, "Sugarmate", sizeof(entry->device));

    entry->sgv_mgdl = doc["value"].as<float>();
    entry->sgv_mmol = entry->sgv_mgdl / 18.0f;

    entry->date_ms  = doc["x"].as<long long>();
    entry->date_sec = entry->date_ms / 1000;

    const char *dir = doc["trend_words"] | "NONE";
    strlcpy(entry->direction, dir, sizeof(entry->direction));
    entry->arrow_angle = directionToAngle(entry->direction);

    delta->mgdl = doc["delta"].as<int>();
    delta->mmol = delta->mgdl / 18.0f;

    return PARSE_OK;
}

/* ── Delta/properties response ─────────────────────────────────── */

int parseDeltaResponse(const char *json, size_t len, DeltaInfo *delta) {
    if (!json || !delta)
        return PARSE_ERR_JSON;

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json, len);
    if (err)
        return PARSE_ERR_JSON;

    delta->mgdl = doc["delta"]["mgdl"].as<int>();
    delta->mmol = delta->mgdl / 18.0f;

    return PARSE_OK;
}

/* ── Delta formatting ──────────────────────────────────────────── */

void formatDelta(char *buf, size_t bufsize, const DeltaInfo *delta, bool show_mgdl) {
    if (show_mgdl)
        snprintf(buf, bufsize, "%+d", delta->mgdl);
    else
        snprintf(buf, bufsize, "%+.1f", delta->mmol);
}

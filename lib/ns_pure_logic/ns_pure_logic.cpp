/*  ns_pure_logic.cpp — Hardware-free pure logic for M5_NightscoutMon
 *
 *  Copyright (C) 2024-2026 Eric Dodd <eric.e.dodd@gmail.com>
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ns_pure_logic.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* ── CRC-16 ─────────────────────────────────────────────────────── */

uint16_t crc16_update(uint16_t crc, uint8_t a) {
    crc ^= a;
    for (int i = 0; i < 8; ++i) {
        if (crc & 1)
            crc = (crc >> 1) ^ 0xA001;
        else
            crc = (crc >> 1);
    }
    return crc;
}

uint16_t calcCRC(const char* str) {
    uint16_t crc = 0;
    for (size_t i = 0; i < strlen(str); i++) {
        crc = crc16_update(crc, (uint8_t)str[i]);
    }
    return crc;
}

/* ── Direction → angle mapping ──────────────────────────────────── */

int directionToAngle(const char* direction) {
    if (direction == NULL)
        return 180;

    if (strcmp(direction, "DoubleDown") == 0 || strcmp(direction, "DOUBLE_DOWN") == 0)
        return 90;
    if (strcmp(direction, "SingleDown") == 0 || strcmp(direction, "SINGLE_DOWN") == 0)
        return 75;
    if (strcmp(direction, "FortyFiveDown") == 0 || strcmp(direction, "FORTY_FIVE_DOWN") == 0)
        return 45;
    if (strcmp(direction, "Flat") == 0 || strcmp(direction, "FLAT") == 0)
        return 0;
    if (strcmp(direction, "FortyFiveUp") == 0 || strcmp(direction, "FORTY_FIVE_UP") == 0)
        return -45;
    if (strcmp(direction, "SingleUp") == 0 || strcmp(direction, "SINGLE_UP") == 0)
        return -75;
    if (strcmp(direction, "DoubleUp") == 0 || strcmp(direction, "DOUBLE_UP") == 0)
        return -90;

    return 180;  // NONE, NOT COMPUTABLE, unknown
}

/* ── Snooze packet helpers ──────────────────────────────────────── */

bool isValidSnoozePacket(const char* packetBuffer) {
    return strncmp(packetBuffer, "M5_Nightscout SNOOZE: USR=", 26) == 0;
}

bool parseSnoozePacket(const char* packetBuffer,
                       int* urlCRC_out,
                       unsigned long* snoozeUntil_out) {
    if (packetBuffer == NULL || urlCRC_out == NULL || snoozeUntil_out == NULL)
        return false;

    int parsed = sscanf(packetBuffer,
                        "M5_Nightscout SNOOZE: USR=%d, SnoozeUntil=%lu",
                        urlCRC_out, snoozeUntil_out);
    return parsed == 2;
}

/* ── JSON sanitization ──────────────────────────────────────────── */

/* Helper: replace 6-char sequence starting at buf[pos] with a space,
   shifting the rest of the buffer left by 5. Returns new len. */
static size_t replace6WithSpace(char* buf, size_t len, size_t pos) {
    buf[pos] = ' ';
    size_t tail = len - (pos + 6);
    memmove(&buf[pos + 1], &buf[pos + 6], tail + 1);  // +1 for null
    return len - 5;
}

size_t sanitizeJson(char* buf, size_t len) {
    // 1. Replace control characters (< 32) with space
    for (size_t i = 0; i < len; i++) {
        if ((unsigned char)buf[i] < 32)
            buf[i] = ' ';
    }

    // 2. Replace problematic unicode escape sequences
    const char* escapes[] = { "\\u0000", "\\u000b", "\\u0032" };
    for (int e = 0; e < 3; e++) {
        size_t i = 0;
        while (i + 5 < len) {
            if (strncmp(&buf[i], escapes[e], 6) == 0) {
                len = replace6WithSpace(buf, len, i);
                // don't advance i — check same position again
            } else {
                i++;
            }
        }
    }

    // 3. Strip fractional milliseconds from "date": fields
    const char* dateKey = "\"date\":";
    size_t dkLen = strlen(dateKey);
    size_t i = 0;
    while (i + dkLen < len) {
        if (strncmp(&buf[i], dateKey, dkLen) == 0) {
            // Find the dot after digits
            size_t numStart = i + dkLen;
            size_t j = numStart;
            // skip digits
            while (j < len && buf[j] >= '0' && buf[j] <= '9')
                j++;
            // if dot follows, strip dot and trailing digits
            if (j < len && buf[j] == '.') {
                size_t dotPos = j;
                j++;
                while (j < len && buf[j] >= '0' && buf[j] <= '9')
                    j++;
                // remove from dotPos to j-1
                size_t removeCount = j - dotPos;
                memmove(&buf[dotPos], &buf[j], len - j + 1);  // +1 for null
                len -= removeCount;
            }
            i = numStart;
        } else {
            i++;
        }
    }

    return len;
}

/* ── INI file helpers ───────────────────────────────────────────── */

bool isCommentChar(char c) {
    return (c == ';' || c == '#');
}

char* skipWhiteSpace(char* str) {
    char* cp = str;
    while (isspace((unsigned char)*cp))
        ++cp;
    return cp;
}

void removeTrailingWhiteSpace(char* str) {
    char* cp = str + strlen(str) - 1;
    while (cp >= str && isspace((unsigned char)*cp))
        *cp-- = '\0';
}

bool parseIPAddress(const char* str, uint8_t ip[4]) {
    if (str == NULL)
        return false;

    int i = 0;
    const char* cp = str;
    ip[0] = ip[1] = ip[2] = ip[3] = 0;

    while (*cp != '\0' && i < 4) {
        if (*cp == '.') {
            ++i;
            ++cp;
            continue;
        }
        if (isdigit((unsigned char)*cp)) {
            ip[i] *= 10;
            ip[i] += (*cp - '0');
        } else {
            ip[0] = ip[1] = ip[2] = ip[3] = 0;
            return false;
        }
        ++cp;
    }
    return (i == 3);  // must have seen exactly 3 dots
}

bool parseMACAddress(const char* str, uint8_t mac[6]) {
    if (str == NULL)
        return false;

    int i = 0;
    const char* cp = str;
    memset(mac, 0, 6);

    while (*cp != '\0' && i < 6) {
        if (*cp == ':' || *cp == '-') {
            ++i;
            ++cp;
            continue;
        }
        if (isdigit((unsigned char)*cp)) {
            mac[i] *= 16;
            mac[i] += (*cp - '0');
        } else if (isxdigit((unsigned char)*cp)) {
            mac[i] *= 16;
            mac[i] += (toupper((unsigned char)*cp) - 55);
        } else {
            memset(mac, 0, 6);
            return false;
        }
        ++cp;
    }
    return (i == 5);  // must have seen exactly 5 separators
}

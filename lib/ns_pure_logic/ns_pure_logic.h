/*  ns_pure_logic.h — Hardware-free pure logic for M5_NightscoutMon
 *
 *  Every function here compiles on both ESP32 (Arduino) and host (native).
 *  No Arduino.h, no M5Stack.h, no WiFi.h — only standard C/C++ types.
 *
 *  Copyright (C) 2024-2026 Eric Dodd <eric.e.dodd@gmail.com>
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef NS_PURE_LOGIC_H
#define NS_PURE_LOGIC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── CRC-16 ─────────────────────────────────────────────────────── */

uint16_t crc16_update(uint16_t crc, uint8_t a);
uint16_t calcCRC(const char* str);

/* ── Direction → angle mapping ──────────────────────────────────── */

/**
 * Convert a Nightscout direction string to a display angle (degrees).
 *   "DoubleUp"   / "DOUBLE_UP"       → -90
 *   "SingleUp"   / "SINGLE_UP"       → -75
 *   "FortyFiveUp"/ "FORTY_FIVE_UP"   → -45
 *   "Flat"       / "FLAT"            →   0
 *   "FortyFiveDown"/"FORTY_FIVE_DOWN"→  45
 *   "SingleDown" / "SINGLE_DOWN"     →  75
 *   "DoubleDown" / "DOUBLE_DOWN"     →  90
 *   anything else (incl. "NONE", "NOT COMPUTABLE", NULL) → 180
 */
int directionToAngle(const char* direction);

/* ── Snooze packet helpers ──────────────────────────────────────── */

bool isValidSnoozePacket(const char* packetBuffer);

/**
 * Parse a validated snooze packet.
 * Returns true if both fields were extracted.
 *   urlCRC_out  — the USR= value
 *   snoozeUntil_out — the SnoozeUntil= value
 */
bool parseSnoozePacket(const char* packetBuffer,
                       int* urlCRC_out,
                       unsigned long* snoozeUntil_out);

/* ── JSON sanitization ──────────────────────────────────────────── */

/**
 * Sanitize a JSON buffer in-place:
 *   1. Replace chars < 32 with space
 *   2. Replace \u0000, \u000b, \u0032 with spaces
 *   3. Strip fractional milliseconds from "date": fields
 *      (e.g. "date":1234567890.123 → "date":1234567890)
 *
 * buf must be a writable, null-terminated C string.
 * Returns the new length (may be shorter due to removals).
 */
size_t sanitizeJson(char* buf, size_t len);

/* ── INI file helpers (replicated from IniFile.cpp) ─────────────── */

bool isCommentChar(char c);
char* skipWhiteSpace(char* str);
void removeTrailingWhiteSpace(char* str);

/**
 * Parse a dotted-decimal IP address string into 4 octets.
 * Returns true on success.
 */
bool parseIPAddress(const char* str, uint8_t ip[4]);

/**
 * Parse a MAC address string (AA:BB:CC:DD:EE:FF or AA-BB-CC-DD-EE-FF)
 * into 6 bytes.  Returns true on success.
 */
bool parseMACAddress(const char* str, uint8_t mac[6]);

#ifdef __cplusplus
}
#endif

#endif /* NS_PURE_LOGIC_H */

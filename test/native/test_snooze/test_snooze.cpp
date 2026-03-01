#include <unity.h>
#include "ns_pure_logic.h"

void setUp(void) {}
void tearDown(void) {}

/* ── isValidSnoozePacket ────────────────────────────────────────── */

void test_valid_snooze_packet(void) {
    TEST_ASSERT_TRUE(isValidSnoozePacket(
        "M5_Nightscout SNOOZE: USR=12345, SnoozeUntil=9999999"));
}

void test_invalid_snooze_wrong_prefix(void) {
    TEST_ASSERT_FALSE(isValidSnoozePacket("WRONG PREFIX"));
}

void test_invalid_snooze_empty(void) {
    TEST_ASSERT_FALSE(isValidSnoozePacket(""));
}

void test_invalid_snooze_partial_prefix(void) {
    TEST_ASSERT_FALSE(isValidSnoozePacket("M5_Nightscout SNOOZE: USR"));
}

void test_valid_snooze_exact_prefix(void) {
    // Just the prefix with nothing after
    TEST_ASSERT_TRUE(isValidSnoozePacket("M5_Nightscout SNOOZE: USR="));
}

/* ── parseSnoozePacket ──────────────────────────────────────────── */

void test_parse_valid_packet(void) {
    int urlCRC = 0;
    unsigned long snoozeUntil = 0;
    bool ok = parseSnoozePacket(
        "M5_Nightscout SNOOZE: USR=42, SnoozeUntil=1700000000",
        &urlCRC, &snoozeUntil);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(42, urlCRC);
    TEST_ASSERT_EQUAL(1700000000UL, snoozeUntil);
}

void test_parse_large_values(void) {
    int urlCRC = 0;
    unsigned long snoozeUntil = 0;
    bool ok = parseSnoozePacket(
        "M5_Nightscout SNOOZE: USR=65535, SnoozeUntil=4294967295",
        &urlCRC, &snoozeUntil);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(65535, urlCRC);
}

void test_parse_truncated_packet(void) {
    int urlCRC = 0;
    unsigned long snoozeUntil = 0;
    bool ok = parseSnoozePacket(
        "M5_Nightscout SNOOZE: USR=42",
        &urlCRC, &snoozeUntil);
    TEST_ASSERT_FALSE(ok);
}

void test_parse_missing_snooze_value(void) {
    int urlCRC = 0;
    unsigned long snoozeUntil = 0;
    bool ok = parseSnoozePacket(
        "M5_Nightscout SNOOZE: USR=42, SnoozeUntil=",
        &urlCRC, &snoozeUntil);
    TEST_ASSERT_FALSE(ok);
}

void test_parse_null_inputs(void) {
    int urlCRC = 0;
    unsigned long snoozeUntil = 0;
    TEST_ASSERT_FALSE(parseSnoozePacket(NULL, &urlCRC, &snoozeUntil));
    TEST_ASSERT_FALSE(parseSnoozePacket("valid", NULL, &snoozeUntil));
    TEST_ASSERT_FALSE(parseSnoozePacket("valid", &urlCRC, NULL));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_valid_snooze_packet);
    RUN_TEST(test_invalid_snooze_wrong_prefix);
    RUN_TEST(test_invalid_snooze_empty);
    RUN_TEST(test_invalid_snooze_partial_prefix);
    RUN_TEST(test_valid_snooze_exact_prefix);
    RUN_TEST(test_parse_valid_packet);
    RUN_TEST(test_parse_large_values);
    RUN_TEST(test_parse_truncated_packet);
    RUN_TEST(test_parse_missing_snooze_value);
    RUN_TEST(test_parse_null_inputs);
    return UNITY_END();
}

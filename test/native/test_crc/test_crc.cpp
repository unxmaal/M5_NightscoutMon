#include <unity.h>
#include "ns_pure_logic.h"

void setUp(void) {}
void tearDown(void) {}

/* ── crc16_update ───────────────────────────────────────────────── */

void test_crc16_update_zero_input(void) {
    // CRC of 0 XOR 0 should give a known value
    uint16_t result = crc16_update(0, 0);
    TEST_ASSERT_EQUAL_UINT16(0, result);
}

void test_crc16_update_single_byte(void) {
    uint16_t result = crc16_update(0, 'A');
    TEST_ASSERT_NOT_EQUAL(0, result);
    // Verify determinism
    TEST_ASSERT_EQUAL_UINT16(result, crc16_update(0, 'A'));
}

void test_crc16_update_different_bytes_differ(void) {
    uint16_t a = crc16_update(0, 'A');
    uint16_t b = crc16_update(0, 'B');
    TEST_ASSERT_NOT_EQUAL(a, b);
}

/* ── calcCRC ────────────────────────────────────────────────────── */

void test_calcCRC_empty_string(void) {
    TEST_ASSERT_EQUAL_UINT16(0, calcCRC(""));
}

void test_calcCRC_known_value(void) {
    uint16_t crc = calcCRC("hello");
    // Must be deterministic
    TEST_ASSERT_EQUAL_UINT16(crc, calcCRC("hello"));
    TEST_ASSERT_NOT_EQUAL(0, crc);
}

void test_calcCRC_different_strings_differ(void) {
    uint16_t a = calcCRC("https://my.nightscout.site/api/v1");
    uint16_t b = calcCRC("https://other.nightscout.site/api/v1");
    TEST_ASSERT_NOT_EQUAL(a, b);
}

void test_calcCRC_url_uniqueness(void) {
    // Typical Nightscout URLs should produce different CRCs
    uint16_t c1 = calcCRC("https://ns1.herokuapp.com");
    uint16_t c2 = calcCRC("https://ns2.herokuapp.com");
    uint16_t c3 = calcCRC("https://myns.azure.com");
    TEST_ASSERT_NOT_EQUAL(c1, c2);
    TEST_ASSERT_NOT_EQUAL(c1, c3);
    TEST_ASSERT_NOT_EQUAL(c2, c3);
}

void test_calcCRC_single_char(void) {
    uint16_t crc = calcCRC("A");
    // Should match a single crc16_update call
    TEST_ASSERT_EQUAL_UINT16(crc16_update(0, 'A'), crc);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_crc16_update_zero_input);
    RUN_TEST(test_crc16_update_single_byte);
    RUN_TEST(test_crc16_update_different_bytes_differ);
    RUN_TEST(test_calcCRC_empty_string);
    RUN_TEST(test_calcCRC_known_value);
    RUN_TEST(test_calcCRC_different_strings_differ);
    RUN_TEST(test_calcCRC_url_uniqueness);
    RUN_TEST(test_calcCRC_single_char);
    return UNITY_END();
}

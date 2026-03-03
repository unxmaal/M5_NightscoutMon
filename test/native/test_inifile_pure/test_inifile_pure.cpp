#include <unity.h>
#include <string.h>
#include "ns_pure_logic.h"

void setUp(void) {}
void tearDown(void) {}

/* ── isCommentChar ──────────────────────────────────────────────── */

void test_semicolon_is_comment(void) {
    TEST_ASSERT_TRUE(isCommentChar(';'));
}

void test_hash_is_comment(void) {
    TEST_ASSERT_TRUE(isCommentChar('#'));
}

void test_letter_not_comment(void) {
    TEST_ASSERT_FALSE(isCommentChar('a'));
}

void test_space_not_comment(void) {
    TEST_ASSERT_FALSE(isCommentChar(' '));
}

void test_null_not_comment(void) {
    TEST_ASSERT_FALSE(isCommentChar('\0'));
}

/* ── skipWhiteSpace ─────────────────────────────────────────────── */

void test_skip_leading_spaces(void) {
    char str[] = "   hello";
    char* result = skipWhiteSpace(str);
    TEST_ASSERT_EQUAL_STRING("hello", result);
}

void test_skip_leading_tabs(void) {
    char str[] = "\t\thello";
    char* result = skipWhiteSpace(str);
    TEST_ASSERT_EQUAL_STRING("hello", result);
}

void test_skip_mixed_whitespace(void) {
    char str[] = " \t \nhello";
    char* result = skipWhiteSpace(str);
    TEST_ASSERT_EQUAL_STRING("hello", result);
}

void test_skip_no_whitespace(void) {
    char str[] = "hello";
    char* result = skipWhiteSpace(str);
    TEST_ASSERT_EQUAL_STRING("hello", result);
}

void test_skip_all_whitespace(void) {
    char str[] = "   ";
    char* result = skipWhiteSpace(str);
    TEST_ASSERT_EQUAL_STRING("", result);
}

/* ── removeTrailingWhiteSpace ───────────────────────────────────── */

void test_remove_trailing_spaces(void) {
    char str[] = "hello   ";
    removeTrailingWhiteSpace(str);
    TEST_ASSERT_EQUAL_STRING("hello", str);
}

void test_remove_trailing_tabs(void) {
    char str[] = "hello\t\t";
    removeTrailingWhiteSpace(str);
    TEST_ASSERT_EQUAL_STRING("hello", str);
}

void test_remove_trailing_mixed(void) {
    char str[] = "hello \t \n";
    removeTrailingWhiteSpace(str);
    TEST_ASSERT_EQUAL_STRING("hello", str);
}

void test_remove_trailing_none(void) {
    char str[] = "hello";
    removeTrailingWhiteSpace(str);
    TEST_ASSERT_EQUAL_STRING("hello", str);
}

/* ── parseIPAddress ─────────────────────────────────────────────── */

void test_parse_ip_valid(void) {
    uint8_t ip[4] = {0};
    TEST_ASSERT_TRUE(parseIPAddress("192.168.1.100", ip));
    TEST_ASSERT_EQUAL_UINT8(192, ip[0]);
    TEST_ASSERT_EQUAL_UINT8(168, ip[1]);
    TEST_ASSERT_EQUAL_UINT8(1, ip[2]);
    TEST_ASSERT_EQUAL_UINT8(100, ip[3]);
}

void test_parse_ip_zeros(void) {
    uint8_t ip[4] = {0};
    TEST_ASSERT_TRUE(parseIPAddress("0.0.0.0", ip));
    TEST_ASSERT_EQUAL_UINT8(0, ip[0]);
    TEST_ASSERT_EQUAL_UINT8(0, ip[3]);
}

void test_parse_ip_max(void) {
    uint8_t ip[4] = {0};
    TEST_ASSERT_TRUE(parseIPAddress("255.255.255.255", ip));
    TEST_ASSERT_EQUAL_UINT8(255, ip[0]);
    TEST_ASSERT_EQUAL_UINT8(255, ip[3]);
}

void test_parse_ip_invalid_char(void) {
    uint8_t ip[4] = {0};
    TEST_ASSERT_FALSE(parseIPAddress("192.168.x.1", ip));
    // ip should be zeroed on failure
    TEST_ASSERT_EQUAL_UINT8(0, ip[0]);
}

void test_parse_ip_null(void) {
    uint8_t ip[4] = {0};
    TEST_ASSERT_FALSE(parseIPAddress(NULL, ip));
}

/* ── parseMACAddress ────────────────────────────────────────────── */

void test_parse_mac_colon(void) {
    uint8_t mac[6] = {0};
    TEST_ASSERT_TRUE(parseMACAddress("AA:BB:CC:DD:EE:FF", mac));
    TEST_ASSERT_EQUAL_UINT8(0xAA, mac[0]);
    TEST_ASSERT_EQUAL_UINT8(0xBB, mac[1]);
    TEST_ASSERT_EQUAL_UINT8(0xCC, mac[2]);
    TEST_ASSERT_EQUAL_UINT8(0xDD, mac[3]);
    TEST_ASSERT_EQUAL_UINT8(0xEE, mac[4]);
    TEST_ASSERT_EQUAL_UINT8(0xFF, mac[5]);
}

void test_parse_mac_dash(void) {
    uint8_t mac[6] = {0};
    TEST_ASSERT_TRUE(parseMACAddress("01-23-45-67-89-AB", mac));
    TEST_ASSERT_EQUAL_UINT8(0x01, mac[0]);
    TEST_ASSERT_EQUAL_UINT8(0x23, mac[1]);
    TEST_ASSERT_EQUAL_UINT8(0xAB, mac[5]);
}

void test_parse_mac_lowercase(void) {
    uint8_t mac[6] = {0};
    TEST_ASSERT_TRUE(parseMACAddress("aa:bb:cc:dd:ee:ff", mac));
    TEST_ASSERT_EQUAL_UINT8(0xAA, mac[0]);
    TEST_ASSERT_EQUAL_UINT8(0xFF, mac[5]);
}

void test_parse_mac_invalid(void) {
    uint8_t mac[6] = {0};
    TEST_ASSERT_FALSE(parseMACAddress("GG:HH:II:JJ:KK:LL", mac));
}

void test_parse_mac_null(void) {
    uint8_t mac[6] = {0};
    TEST_ASSERT_FALSE(parseMACAddress(NULL, mac));
}

/* ── Overflow tests ────────────────────────────────────────────── */

void test_parse_ip_overflow_256(void) {
    uint8_t ip[4] = {0};
    TEST_ASSERT_FALSE(parseIPAddress("256.0.0.0", ip));
}

void test_parse_ip_overflow_999(void) {
    uint8_t ip[4] = {0};
    TEST_ASSERT_FALSE(parseIPAddress("999.0.0.0", ip));
}

void test_parse_mac_overflow(void) {
    uint8_t mac[6] = {0};
    TEST_ASSERT_FALSE(parseMACAddress("FFF:00:00:00:00:00", mac));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_semicolon_is_comment);
    RUN_TEST(test_hash_is_comment);
    RUN_TEST(test_letter_not_comment);
    RUN_TEST(test_space_not_comment);
    RUN_TEST(test_null_not_comment);
    RUN_TEST(test_skip_leading_spaces);
    RUN_TEST(test_skip_leading_tabs);
    RUN_TEST(test_skip_mixed_whitespace);
    RUN_TEST(test_skip_no_whitespace);
    RUN_TEST(test_skip_all_whitespace);
    RUN_TEST(test_remove_trailing_spaces);
    RUN_TEST(test_remove_trailing_tabs);
    RUN_TEST(test_remove_trailing_mixed);
    RUN_TEST(test_remove_trailing_none);
    RUN_TEST(test_parse_ip_valid);
    RUN_TEST(test_parse_ip_zeros);
    RUN_TEST(test_parse_ip_max);
    RUN_TEST(test_parse_ip_invalid_char);
    RUN_TEST(test_parse_ip_null);
    RUN_TEST(test_parse_mac_colon);
    RUN_TEST(test_parse_mac_dash);
    RUN_TEST(test_parse_mac_lowercase);
    RUN_TEST(test_parse_mac_invalid);
    RUN_TEST(test_parse_mac_null);
    RUN_TEST(test_parse_ip_overflow_256);
    RUN_TEST(test_parse_ip_overflow_999);
    RUN_TEST(test_parse_mac_overflow);
    return UNITY_END();
}

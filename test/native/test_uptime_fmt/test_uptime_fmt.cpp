#include <unity.h>
#include <string.h>
#include "ns_pure_logic.h"

void setUp(void) {}
void tearDown(void) {}

/* ── formatUptime ──────────────────────────────────────────────── */

void test_formatUptime_zero(void) {
    char buf[32];
    formatUptime(buf, sizeof(buf), 0);
    TEST_ASSERT_EQUAL_STRING("00d 00:00:00", buf);
}

void test_formatUptime_one_second(void) {
    char buf[32];
    formatUptime(buf, sizeof(buf), 1000);
    TEST_ASSERT_EQUAL_STRING("00d 00:00:01", buf);
}

void test_formatUptime_one_minute(void) {
    char buf[32];
    formatUptime(buf, sizeof(buf), 60000);
    TEST_ASSERT_EQUAL_STRING("00d 00:01:00", buf);
}

void test_formatUptime_one_hour(void) {
    char buf[32];
    formatUptime(buf, sizeof(buf), 3600000UL);
    TEST_ASSERT_EQUAL_STRING("00d 01:00:00", buf);
}

void test_formatUptime_one_day(void) {
    char buf[32];
    formatUptime(buf, sizeof(buf), 86400000UL);
    TEST_ASSERT_EQUAL_STRING("01d 00:00:00", buf);
}

void test_formatUptime_complex(void) {
    // 2 days, 13 hours, 45 minutes, 30 seconds
    unsigned long ms = (2UL * 86400 + 13 * 3600 + 45 * 60 + 30) * 1000UL;
    char buf[32];
    formatUptime(buf, sizeof(buf), ms);
    TEST_ASSERT_EQUAL_STRING("02d 13:45:30", buf);
}

void test_formatUptime_max_reasonable(void) {
    // 99 days
    unsigned long ms = 99UL * 86400UL * 1000UL;
    char buf[32];
    formatUptime(buf, sizeof(buf), ms);
    TEST_ASSERT_EQUAL_STRING("99d 00:00:00", buf);
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_formatUptime_zero);
    RUN_TEST(test_formatUptime_one_second);
    RUN_TEST(test_formatUptime_one_minute);
    RUN_TEST(test_formatUptime_one_hour);
    RUN_TEST(test_formatUptime_one_day);
    RUN_TEST(test_formatUptime_complex);
    RUN_TEST(test_formatUptime_max_reasonable);
    return UNITY_END();
}

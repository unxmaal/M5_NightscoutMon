#include <unity.h>
#include <string.h>
#include "ns_pure_logic.h"

void setUp(void) {}
void tearDown(void) {}

/* ── formatGlucose ─────────────────────────────────────────────── */

void test_formatGlucose_mmol_single_digit(void) {
    char buf[32];
    int font_hint = formatGlucose(buf, sizeof(buf), 8.4f, 0.0f, false);
    TEST_ASSERT_EQUAL_STRING("8.4", buf);
    TEST_ASSERT_EQUAL_INT(FONT_LARGE, font_hint);
}

void test_formatGlucose_mmol_double_digit(void) {
    char buf[32];
    int font_hint = formatGlucose(buf, sizeof(buf), 12.3f, 0.0f, false);
    TEST_ASSERT_EQUAL_STRING("12.3", buf);
    TEST_ASSERT_EQUAL_INT(FONT_MEDIUM, font_hint);
}

void test_formatGlucose_mgdl_two_digit(void) {
    char buf[32];
    int font_hint = formatGlucose(buf, sizeof(buf), 0.0f, 85.0f, true);
    TEST_ASSERT_EQUAL_STRING("85", buf);
    TEST_ASSERT_EQUAL_INT(FONT_LARGE, font_hint);
}

void test_formatGlucose_mgdl_three_digit(void) {
    char buf[32];
    int font_hint = formatGlucose(buf, sizeof(buf), 0.0f, 120.0f, true);
    TEST_ASSERT_EQUAL_STRING("120", buf);
    TEST_ASSERT_EQUAL_INT(FONT_LARGE, font_hint);
}

void test_formatGlucose_mmol_low_value(void) {
    char buf[32];
    int font_hint = formatGlucose(buf, sizeof(buf), 3.2f, 0.0f, false);
    TEST_ASSERT_EQUAL_STRING("3.2", buf);
    TEST_ASSERT_EQUAL_INT(FONT_LARGE, font_hint);
}

void test_formatGlucose_mgdl_low_value(void) {
    char buf[32];
    int font_hint = formatGlucose(buf, sizeof(buf), 0.0f, 55.0f, true);
    TEST_ASSERT_EQUAL_STRING("55", buf);
    TEST_ASSERT_EQUAL_INT(FONT_LARGE, font_hint);
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_formatGlucose_mmol_single_digit);
    RUN_TEST(test_formatGlucose_mmol_double_digit);
    RUN_TEST(test_formatGlucose_mgdl_two_digit);
    RUN_TEST(test_formatGlucose_mgdl_three_digit);
    RUN_TEST(test_formatGlucose_mmol_low_value);
    RUN_TEST(test_formatGlucose_mgdl_low_value);
    return UNITY_END();
}

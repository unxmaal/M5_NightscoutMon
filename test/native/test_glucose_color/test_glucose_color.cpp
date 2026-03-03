#include <unity.h>
#include "ns_pure_logic.h"

void setUp(void) {}
void tearDown(void) {}

/* ── glucoseColor ──────────────────────────────────────────────── */

/* Thresholds used across tests (mimicking typical config):
 *   red_low=3.0  yellow_low=4.0  yellow_high=10.0  red_high=14.0
 */

void test_glucoseColor_normal_range(void) {
    // 7.0 is within yellow_low..yellow_high → green
    TEST_ASSERT_EQUAL_INT(GLUCOSE_COLOR_GREEN,
        glucoseColor(7.0f, 4.0f, 10.0f, 3.0f, 14.0f));
}

void test_glucoseColor_exactly_yellow_low(void) {
    // At the boundary: 4.0 is NOT < 4.0, so still green
    TEST_ASSERT_EQUAL_INT(GLUCOSE_COLOR_GREEN,
        glucoseColor(4.0f, 4.0f, 10.0f, 3.0f, 14.0f));
}

void test_glucoseColor_below_yellow_low(void) {
    // 3.5 < yellow_low(4.0) but >= red_low(3.0) → yellow
    TEST_ASSERT_EQUAL_INT(GLUCOSE_COLOR_YELLOW,
        glucoseColor(3.5f, 4.0f, 10.0f, 3.0f, 14.0f));
}

void test_glucoseColor_below_red_low(void) {
    // 2.5 < red_low(3.0) → red
    TEST_ASSERT_EQUAL_INT(GLUCOSE_COLOR_RED,
        glucoseColor(2.5f, 4.0f, 10.0f, 3.0f, 14.0f));
}

void test_glucoseColor_above_yellow_high(void) {
    // 12.0 > yellow_high(10.0) but < red_high(14.0) → yellow
    TEST_ASSERT_EQUAL_INT(GLUCOSE_COLOR_YELLOW,
        glucoseColor(12.0f, 4.0f, 10.0f, 3.0f, 14.0f));
}

void test_glucoseColor_above_red_high(void) {
    // 15.0 > red_high(14.0) → red
    TEST_ASSERT_EQUAL_INT(GLUCOSE_COLOR_RED,
        glucoseColor(15.0f, 4.0f, 10.0f, 3.0f, 14.0f));
}

void test_glucoseColor_exactly_yellow_high(void) {
    // At boundary: 10.0 is NOT > 10.0, so still green
    TEST_ASSERT_EQUAL_INT(GLUCOSE_COLOR_GREEN,
        glucoseColor(10.0f, 4.0f, 10.0f, 3.0f, 14.0f));
}

void test_glucoseColor_exactly_red_low(void) {
    // 3.0 is NOT < 3.0, so yellow (still below yellow_low)
    TEST_ASSERT_EQUAL_INT(GLUCOSE_COLOR_YELLOW,
        glucoseColor(3.0f, 4.0f, 10.0f, 3.0f, 14.0f));
}

void test_glucoseColor_exactly_red_high(void) {
    // 14.0 is NOT > 14.0, so yellow (still above yellow_high)
    TEST_ASSERT_EQUAL_INT(GLUCOSE_COLOR_YELLOW,
        glucoseColor(14.0f, 4.0f, 10.0f, 3.0f, 14.0f));
}

/* Red overrides yellow — if value is below BOTH thresholds,
   the .ino checks yellow first then red, so red wins. */
void test_glucoseColor_red_overrides_yellow(void) {
    TEST_ASSERT_EQUAL_INT(GLUCOSE_COLOR_RED,
        glucoseColor(2.0f, 4.0f, 10.0f, 3.0f, 14.0f));
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_glucoseColor_normal_range);
    RUN_TEST(test_glucoseColor_exactly_yellow_low);
    RUN_TEST(test_glucoseColor_below_yellow_low);
    RUN_TEST(test_glucoseColor_below_red_low);
    RUN_TEST(test_glucoseColor_above_yellow_high);
    RUN_TEST(test_glucoseColor_above_red_high);
    RUN_TEST(test_glucoseColor_exactly_yellow_high);
    RUN_TEST(test_glucoseColor_exactly_red_low);
    RUN_TEST(test_glucoseColor_exactly_red_high);
    RUN_TEST(test_glucoseColor_red_overrides_yellow);
    return UNITY_END();
}

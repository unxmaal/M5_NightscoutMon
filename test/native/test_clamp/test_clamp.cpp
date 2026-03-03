#include <unity.h>
#include "ns_pure_logic.h"

void setUp(void) {}
void tearDown(void) {}

void test_clamp_below_min(void) {
    TEST_ASSERT_EQUAL_INT(0, clampInt(-5, 0, 100));
}

void test_clamp_above_max(void) {
    TEST_ASSERT_EQUAL_INT(100, clampInt(200, 0, 100));
}

void test_clamp_in_range(void) {
    TEST_ASSERT_EQUAL_INT(50, clampInt(50, 0, 100));
}

void test_clamp_at_min_boundary(void) {
    TEST_ASSERT_EQUAL_INT(0, clampInt(0, 0, 100));
}

void test_clamp_at_max_boundary(void) {
    TEST_ASSERT_EQUAL_INT(100, clampInt(100, 0, 100));
}

void test_clamp_negative_range(void) {
    TEST_ASSERT_EQUAL_INT(-5, clampInt(-10, -5, 5));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_clamp_below_min);
    RUN_TEST(test_clamp_above_max);
    RUN_TEST(test_clamp_in_range);
    RUN_TEST(test_clamp_at_min_boundary);
    RUN_TEST(test_clamp_at_max_boundary);
    RUN_TEST(test_clamp_negative_range);
    return UNITY_END();
}

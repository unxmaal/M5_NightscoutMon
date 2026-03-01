#include <unity.h>
#include "ns_pure_logic.h"

void setUp(void) {}
void tearDown(void) {}

/* ── camelCase direction strings ────────────────────────────────── */

void test_direction_DoubleDown(void) {
    TEST_ASSERT_EQUAL_INT(90, directionToAngle("DoubleDown"));
}

void test_direction_SingleDown(void) {
    TEST_ASSERT_EQUAL_INT(75, directionToAngle("SingleDown"));
}

void test_direction_FortyFiveDown(void) {
    TEST_ASSERT_EQUAL_INT(45, directionToAngle("FortyFiveDown"));
}

void test_direction_Flat(void) {
    TEST_ASSERT_EQUAL_INT(0, directionToAngle("Flat"));
}

void test_direction_FortyFiveUp(void) {
    TEST_ASSERT_EQUAL_INT(-45, directionToAngle("FortyFiveUp"));
}

void test_direction_SingleUp(void) {
    TEST_ASSERT_EQUAL_INT(-75, directionToAngle("SingleUp"));
}

void test_direction_DoubleUp(void) {
    TEST_ASSERT_EQUAL_INT(-90, directionToAngle("DoubleUp"));
}

/* ── UPPER_CASE direction strings ───────────────────────────────── */

void test_direction_DOUBLE_DOWN(void) {
    TEST_ASSERT_EQUAL_INT(90, directionToAngle("DOUBLE_DOWN"));
}

void test_direction_SINGLE_DOWN(void) {
    TEST_ASSERT_EQUAL_INT(75, directionToAngle("SINGLE_DOWN"));
}

void test_direction_FORTY_FIVE_DOWN(void) {
    TEST_ASSERT_EQUAL_INT(45, directionToAngle("FORTY_FIVE_DOWN"));
}

void test_direction_FLAT(void) {
    TEST_ASSERT_EQUAL_INT(0, directionToAngle("FLAT"));
}

void test_direction_FORTY_FIVE_UP(void) {
    TEST_ASSERT_EQUAL_INT(-45, directionToAngle("FORTY_FIVE_UP"));
}

void test_direction_SINGLE_UP(void) {
    TEST_ASSERT_EQUAL_INT(-75, directionToAngle("SINGLE_UP"));
}

void test_direction_DOUBLE_UP(void) {
    TEST_ASSERT_EQUAL_INT(-90, directionToAngle("DOUBLE_UP"));
}

/* ── Edge cases ─────────────────────────────────────────────────── */

void test_direction_NONE(void) {
    TEST_ASSERT_EQUAL_INT(180, directionToAngle("NONE"));
}

void test_direction_NOT_COMPUTABLE(void) {
    TEST_ASSERT_EQUAL_INT(180, directionToAngle("NOT COMPUTABLE"));
}

void test_direction_unknown(void) {
    TEST_ASSERT_EQUAL_INT(180, directionToAngle("garbage"));
}

void test_direction_empty(void) {
    TEST_ASSERT_EQUAL_INT(180, directionToAngle(""));
}

void test_direction_null(void) {
    TEST_ASSERT_EQUAL_INT(180, directionToAngle(NULL));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_direction_DoubleDown);
    RUN_TEST(test_direction_SingleDown);
    RUN_TEST(test_direction_FortyFiveDown);
    RUN_TEST(test_direction_Flat);
    RUN_TEST(test_direction_FortyFiveUp);
    RUN_TEST(test_direction_SingleUp);
    RUN_TEST(test_direction_DoubleUp);
    RUN_TEST(test_direction_DOUBLE_DOWN);
    RUN_TEST(test_direction_SINGLE_DOWN);
    RUN_TEST(test_direction_FORTY_FIVE_DOWN);
    RUN_TEST(test_direction_FLAT);
    RUN_TEST(test_direction_FORTY_FIVE_UP);
    RUN_TEST(test_direction_SINGLE_UP);
    RUN_TEST(test_direction_DOUBLE_UP);
    RUN_TEST(test_direction_NONE);
    RUN_TEST(test_direction_NOT_COMPUTABLE);
    RUN_TEST(test_direction_unknown);
    RUN_TEST(test_direction_empty);
    RUN_TEST(test_direction_null);
    return UNITY_END();
}

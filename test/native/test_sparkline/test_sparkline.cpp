#include <unity.h>
#include <string.h>
#include "ns_display_model.h"
#include "ns_json_parse.h"

void setUp(void) {}
void tearDown(void) {}

/* ── parseSGVResponseMulti ────────────────────────────────────── */

static const char *THREE_ENTRIES =
    "[{\"sgv\":165,\"date\":1700003000000,\"direction\":\"Flat\",\"device\":\"share2\"},"
     "{\"sgv\":170,\"date\":1700002700000,\"direction\":\"FortyFiveUp\",\"device\":\"share2\"},"
     "{\"sgv\":155,\"date\":1700002400000,\"direction\":\"Flat\",\"device\":\"share2\"}]";

void test_multi_parse_three_entries(void) {
    SGVEntry entries[10];
    int rc;
    int count = parseSGVResponseMulti(THREE_ENTRIES, strlen(THREE_ENTRIES),
                                      entries, 10, &rc);
    TEST_ASSERT_EQUAL(PARSE_OK, rc);
    TEST_ASSERT_EQUAL(3, count);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 165.0f, entries[0].sgv_mgdl);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 170.0f, entries[1].sgv_mgdl);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 155.0f, entries[2].sgv_mgdl);
}

void test_multi_parse_first_matches_single(void) {
    SGVEntry single;
    int singleRc = parseSGVResponse(THREE_ENTRIES, strlen(THREE_ENTRIES), &single);
    TEST_ASSERT_EQUAL(PARSE_OK, singleRc);

    SGVEntry multi[10];
    int rc;
    parseSGVResponseMulti(THREE_ENTRIES, strlen(THREE_ENTRIES), multi, 10, &rc);
    TEST_ASSERT_EQUAL(PARSE_OK, rc);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, single.sgv_mgdl, multi[0].sgv_mgdl);
    TEST_ASSERT_EQUAL(single.date_sec, multi[0].date_sec);
}

static const char *MIXED_ENTRIES =
    "[{\"type\":\"cal\",\"date\":1700003000000},"
     "{\"sgv\":120,\"date\":1700002700000,\"direction\":\"Flat\"},"
     "{\"type\":\"cal\",\"date\":1700002400000},"
     "{\"sgv\":130,\"date\":1700002100000,\"direction\":\"Flat\"}]";

void test_multi_parse_skips_non_sgv(void) {
    SGVEntry entries[10];
    int rc;
    int count = parseSGVResponseMulti(MIXED_ENTRIES, strlen(MIXED_ENTRIES),
                                      entries, 10, &rc);
    TEST_ASSERT_EQUAL(PARSE_OK, rc);
    TEST_ASSERT_EQUAL(2, count);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 120.0f, entries[0].sgv_mgdl);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 130.0f, entries[1].sgv_mgdl);
}

void test_multi_parse_empty_array(void) {
    SGVEntry entries[10];
    int rc;
    int count = parseSGVResponseMulti("[]", 2, entries, 10, &rc);
    TEST_ASSERT_EQUAL(PARSE_ERR_EMPTY, rc);
    TEST_ASSERT_EQUAL(0, count);
}

void test_multi_parse_respects_max(void) {
    SGVEntry entries[2];
    int rc;
    int count = parseSGVResponseMulti(THREE_ENTRIES, strlen(THREE_ENTRIES),
                                      entries, 2, &rc);
    TEST_ASSERT_EQUAL(PARSE_OK, rc);
    TEST_ASSERT_EQUAL(2, count);
}

/* ── buildSparklineModel ──────────────────────────────────────── */

void test_sparkline_five_points(void) {
    float vals[] = {165.0f, 170.0f, 155.0f, 160.0f, 150.0f};
    SparklineModel m;
    buildSparklineModel(&m, vals, 5, 0, 0, 300, 100,
                        3.9f, 10.0f, 3.0f, 13.9f);
    TEST_ASSERT_EQUAL(5, m.count);
    // Newest (index 0) should be rightmost
    TEST_ASSERT_GREATER_THAN(m.points[1].x, m.points[0].x);
    // Highest value (170, index 1) should have lowest Y
    TEST_ASSERT_LESS_THAN(m.points[4].y, m.points[1].y);
}

void test_sparkline_all_same_value(void) {
    float vals[] = {100.0f, 100.0f, 100.0f};
    SparklineModel m;
    buildSparklineModel(&m, vals, 3, 0, 0, 300, 100,
                        3.9f, 10.0f, 3.0f, 13.9f);
    TEST_ASSERT_EQUAL(3, m.count);
    // All points at same Y (center-ish, due to min range padding)
    TEST_ASSERT_EQUAL(m.points[0].y, m.points[1].y);
    TEST_ASSERT_EQUAL(m.points[1].y, m.points[2].y);
}

void test_sparkline_too_few_points(void) {
    float vals[] = {100.0f};
    SparklineModel m;
    buildSparklineModel(&m, vals, 1, 0, 0, 300, 100,
                        3.9f, 10.0f, 3.0f, 13.9f);
    TEST_ASSERT_EQUAL(0, m.count);
}

void test_sparkline_color_coding(void) {
    // green=5.5mmol(99mg), yellow=3.5mmol(63mg), red=2.8mmol(50mg)
    float vals[] = {180.0f, 63.0f};  // green, yellow
    SparklineModel m;
    buildSparklineModel(&m, vals, 2, 0, 0, 300, 100,
                        3.9f, 10.0f, 3.0f, 13.9f);
    TEST_ASSERT_EQUAL(2, m.count);
    TEST_ASSERT_EQUAL(COLOR_GREEN, m.points[0].color);   // 180 mg = 10 mmol
    TEST_ASSERT_EQUAL(COLOR_YELLOW, m.points[1].color);  // 63 mg = 3.5 mmol
}

int main(void) {
    UNITY_BEGIN();
    // Multi-parse
    RUN_TEST(test_multi_parse_three_entries);
    RUN_TEST(test_multi_parse_first_matches_single);
    RUN_TEST(test_multi_parse_skips_non_sgv);
    RUN_TEST(test_multi_parse_empty_array);
    RUN_TEST(test_multi_parse_respects_max);
    // Sparkline model
    RUN_TEST(test_sparkline_five_points);
    RUN_TEST(test_sparkline_all_same_value);
    RUN_TEST(test_sparkline_too_few_points);
    RUN_TEST(test_sparkline_color_coding);
    return UNITY_END();
}

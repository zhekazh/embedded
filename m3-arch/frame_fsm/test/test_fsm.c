#include "unity.h"
#include "frame_fsm.h"

static frame_fsm_t f;
void setUp(void)    { fsm_init(&f); }
void tearDown(void) {}

static bool feed(uint8_t b, uint8_t *out) { return fsm_feed(&f, b, out); }

void test_clean_frame(void) {
    uint8_t out = 0;
    TEST_ASSERT_FALSE(feed(FSM_STX, &out));
    TEST_ASSERT_FALSE(feed('A', &out));
    TEST_ASSERT_TRUE (feed(FSM_ETX, &out));
    TEST_ASSERT_EQUAL_HEX8('A', out);
}
void test_noise_before_stx(void) {
    uint8_t out = 0;
    feed(0xFF, &out); feed(0x00, &out);        /* шум */
    feed(FSM_STX, &out); feed('B', &out);
    TEST_ASSERT_TRUE(feed(FSM_ETX, &out));
    TEST_ASSERT_EQUAL_HEX8('B', out);
}
void test_resync_on_embedded_stx(void) {
    uint8_t out = 0;
    feed(FSM_STX, &out); feed('C', &out);      /* кадр почався */
    feed(FSM_STX, &out);                       /* замість ETX — новий STX → ресинк */
    feed('D', &out);
    TEST_ASSERT_TRUE(feed(FSM_ETX, &out));
    TEST_ASSERT_EQUAL_HEX8('D', out);          /* 'C'-кадр відкинуто, 'D' прийнято */
}
void test_back_to_back(void) {
    uint8_t out = 0;
    feed(FSM_STX, &out); feed('X', &out);
    TEST_ASSERT_TRUE(feed(FSM_ETX, &out)); TEST_ASSERT_EQUAL_HEX8('X', out);
    feed(FSM_STX, &out); feed('Y', &out);
    TEST_ASSERT_TRUE(feed(FSM_ETX, &out)); TEST_ASSERT_EQUAL_HEX8('Y', out);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_clean_frame);
    RUN_TEST(test_noise_before_stx);
    RUN_TEST(test_resync_on_embedded_stx);
    RUN_TEST(test_back_to_back);
    return UNITY_END();
}

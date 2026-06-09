#include "unity.h"
#include "ringbuf.h"

static ringbuf_t rb;
static uint8_t   storage[4];   /* корисна місткість = size - 1 = 3 */

void setUp(void)    { rb_init(&rb, storage, sizeof storage); }
void tearDown(void) {}

void test_fresh_is_empty(void) {
    TEST_ASSERT_TRUE(rb_is_empty(&rb));
    TEST_ASSERT_FALSE(rb_is_full(&rb));
    TEST_ASSERT_EQUAL_INT(0, (int)rb_count(&rb));
}

void test_push_then_pop_roundtrip(void) {
    uint8_t out = 0;
    TEST_ASSERT_TRUE(rb_push(&rb, 0xAB));
    TEST_ASSERT_FALSE(rb_is_empty(&rb));
    TEST_ASSERT_EQUAL_INT(1, (int)rb_count(&rb));
    TEST_ASSERT_TRUE(rb_pop(&rb, &out));
    TEST_ASSERT_EQUAL_HEX8(0xAB, out);
    TEST_ASSERT_TRUE(rb_is_empty(&rb));
}

void test_fill_to_capacity_then_full(void) {
    TEST_ASSERT_TRUE(rb_push(&rb, 1));
    TEST_ASSERT_TRUE(rb_push(&rb, 2));
    TEST_ASSERT_TRUE(rb_push(&rb, 3));
    TEST_ASSERT_TRUE(rb_is_full(&rb));
    TEST_ASSERT_EQUAL_INT(3, (int)rb_count(&rb));
    TEST_ASSERT_FALSE(rb_push(&rb, 4));
    TEST_ASSERT_EQUAL_INT(3, (int)rb_count(&rb));
}

void test_pop_from_empty_fails(void) {
    uint8_t out = 0xFF;
    TEST_ASSERT_FALSE(rb_pop(&rb, &out));
    TEST_ASSERT_EQUAL_HEX8(0xFF, out);
}

void test_wraparound_keeps_fifo(void) {
    uint8_t out = 0;
    /* 10 обертів push/pop проганяють head і tail через стик size */
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_TRUE(rb_push(&rb, (uint8_t)i));
        TEST_ASSERT_TRUE(rb_pop(&rb, &out));
        TEST_ASSERT_EQUAL_HEX8((uint8_t)i, out);
    }
    TEST_ASSERT_TRUE(rb_is_empty(&rb));
}

void test_fifo_order_after_head_wraps(void) {
    uint8_t out = 0;
    TEST_ASSERT_TRUE(rb_push(&rb, 10));
    TEST_ASSERT_TRUE(rb_push(&rb, 20));
    TEST_ASSERT_TRUE(rb_pop(&rb, &out));   /* tail рушає вперед */
    TEST_ASSERT_EQUAL_HEX8(10, out);
    TEST_ASSERT_TRUE(rb_push(&rb, 30));
    TEST_ASSERT_TRUE(rb_push(&rb, 40));    /* head обертається за size */
    TEST_ASSERT_TRUE(rb_pop(&rb, &out)); TEST_ASSERT_EQUAL_HEX8(20, out);
    TEST_ASSERT_TRUE(rb_pop(&rb, &out)); TEST_ASSERT_EQUAL_HEX8(30, out);
    TEST_ASSERT_TRUE(rb_pop(&rb, &out)); TEST_ASSERT_EQUAL_HEX8(40, out);
    TEST_ASSERT_TRUE(rb_is_empty(&rb));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_fresh_is_empty);
    RUN_TEST(test_push_then_pop_roundtrip);
    RUN_TEST(test_fill_to_capacity_then_full);
    RUN_TEST(test_pop_from_empty_fails);
    RUN_TEST(test_wraparound_keeps_fifo);
    RUN_TEST(test_fifo_order_after_head_wraps);
    return UNITY_END();
}

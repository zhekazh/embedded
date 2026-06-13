#include "unity.h"
#include "frame_rx.h"
#include "frame.h"
#include "ringbuf.h"
#include <string.h>

#define MAXCAP 8
typedef struct { int count; uint8_t data[MAXCAP][FRAME_MAX_PAYLOAD]; size_t len[MAXCAP]; } cap_t;

static cap_t      cap;
static frame_rx_t rx;
static uint8_t    acc[300];

static void on_frame(const uint8_t *p, size_t n, void *ctx) {
    cap_t *c = (cap_t *)ctx;
    if (c->count < MAXCAP) {
        memcpy(c->data[c->count], p, n);
        c->len[c->count] = n;
        c->count++;
    }
}

void setUp(void) {
    memset(&cap, 0, sizeof cap);
    frame_rx_init(&rx, acc, sizeof acc, on_frame, &cap);
}
void tearDown(void) {}

static void feed(const uint8_t *b, size_t n) {
    for (size_t i = 0; i < n; i++) frame_rx_push(&rx, b[i]);
}

void test_two_valid_frames(void) {
    uint8_t s[128]; size_t sl = 0, fl = 0;
    const uint8_t p1[] = {0x10, 0x00, 0x20};        /* нуль усередині */
    const uint8_t p2[] = {0xAA, 0xBB, 0xCC, 0xDD};
    frame_build(p1, sizeof p1, s + sl, sizeof s - sl, &fl); sl += fl;
    frame_build(p2, sizeof p2, s + sl, sizeof s - sl, &fl); sl += fl;
    feed(s, sl);
    TEST_ASSERT_EQUAL_INT(2, cap.count);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(p1, cap.data[0], sizeof p1);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(p2, cap.data[1], sizeof p2);
}

void test_corrupted_frame_rejected(void) {
    uint8_t f[64]; size_t fl = 0;
    const uint8_t p[] = {0xAA, 0xBB, 0xCC, 0xDD};
    frame_build(p, sizeof p, f, sizeof f, &fl);
    f[1] ^= 0x01;
    feed(f, fl);
    TEST_ASSERT_EQUAL_INT(0, cap.count);
}

void test_garbage_rejected(void) {
    const uint8_t junk[] = {0xDE, 0xAD, 0xBE, 0x00};
    feed(junk, sizeof junk);
    TEST_ASSERT_EQUAL_INT(0, cap.count);
}

void test_empty_frames_ignored(void) {
    const uint8_t z[] = {0x00, 0x00, 0x00};
    feed(z, sizeof z);
    TEST_ASSERT_EQUAL_INT(0, cap.count);
}

void test_overflow_then_resync(void) {
    for (int i = 0; i < 400; i++) {
        frame_rx_push(&rx, 0x42);
    }
    frame_rx_push(&rx, 0x00);                       /* кінець бракованого */
    TEST_ASSERT_EQUAL_INT(0, cap.count);
    uint8_t f[64]; size_t fl = 0;
    const uint8_t p[] = {0x01, 0x02};
    frame_build(p, sizeof p, f, sizeof f, &fl);
    feed(f, fl);                                    /* після ресинку проходить */
    TEST_ASSERT_EQUAL_INT(1, cap.count);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(p, cap.data[0], sizeof p);
}

void test_through_ring_buffer(void) {
    uint8_t s[256]; size_t sl = 0, fl = 0;
    const uint8_t p1[] = {0x11, 0x22};
    const uint8_t p2[] = {0x00, 0x00, 0x99};
    frame_build(p1, sizeof p1, s + sl, sizeof s - sl, &fl); sl += fl;
    frame_build(p2, sizeof p2, s + sl, sizeof s - sl, &fl); sl += fl;

    uint8_t rbstore[16]; ringbuf_t ring;
    rb_init(&ring, rbstore, sizeof rbstore);
    for (size_t i = 0; i < sl; i++) {
        while (!rb_push(&ring, s[i])) {             /* повний -> дренуємо в парсер */
            uint8_t b; rb_pop(&ring, &b); frame_rx_push(&rx, b);
        }
    }
    uint8_t b;
    while (rb_pop(&ring, &b)) frame_rx_push(&rx, b); /* хвіст */

    TEST_ASSERT_EQUAL_INT(2, cap.count);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(p1, cap.data[0], sizeof p1);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(p2, cap.data[1], sizeof p2);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_two_valid_frames);
    RUN_TEST(test_corrupted_frame_rejected);
    RUN_TEST(test_garbage_rejected);
    RUN_TEST(test_empty_frames_ignored);
    RUN_TEST(test_overflow_then_resync);
    RUN_TEST(test_through_ring_buffer);
    return UNITY_END();
}

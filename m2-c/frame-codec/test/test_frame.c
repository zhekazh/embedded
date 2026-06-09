#include "unity.h"
#include "frame.h"

void setUp(void) {}
void tearDown(void) {}

static void rt(const uint8_t *p, size_t n) {
    uint8_t f[600]; size_t fl = 0;
    TEST_ASSERT_TRUE(frame_build(p, n, f, sizeof f, &fl));
    TEST_ASSERT_TRUE(fl >= 1 && f[fl - 1] == 0x00);   /* завершується роздільником */
    for (size_t i = 0; i + 1 < fl; i++) TEST_ASSERT_TRUE(f[i] != 0); /* нуль лише в кінці */
    uint8_t o[600]; size_t ol = 0;
    TEST_ASSERT_TRUE(frame_parse(f, fl - 1, o, sizeof o, &ol));      /* зрізаємо роздільник */
    TEST_ASSERT_EQUAL_INT((int)n, (int)ol);
    if (n > 0) TEST_ASSERT_EQUAL_UINT8_ARRAY(p, o, n);
}

void test_roundtrip_basic(void)       { const uint8_t p[] = {0x10,0x20,0x30,0x40}; rt(p, sizeof p); }
void test_roundtrip_with_zeros(void)  { const uint8_t z[] = {0,0,0}; const uint8_t m[] = {1,0,2,0,0,3}; rt(z,sizeof z); rt(m,sizeof m); }
void test_roundtrip_empty(void)       { const uint8_t x[1] = {0xAA}; rt(x, 0); }
void test_roundtrip_max_payload(void) { uint8_t b[FRAME_MAX_PAYLOAD]; for (size_t i=0;i<sizeof b;i++) b[i]=(uint8_t)(i+1); rt(b,sizeof b); }

void test_corruption_detected(void) {
    const uint8_t p[] = {0x10,0x20,0x30,0x40};
    uint8_t f[64]; size_t fl = 0;
    TEST_ASSERT_TRUE(frame_build(p, sizeof p, f, sizeof f, &fl));
    f[2] ^= 0x01;                                     /* перевертаємо біт у тілі кадру */
    uint8_t o[64]; size_t ol = 0;
    TEST_ASSERT_FALSE(frame_parse(f, fl - 1, o, sizeof o, &ol));  /* CRC ловить */
}

void test_truncated_rejected(void) {
    const uint8_t p[] = {0x10,0x20,0x30,0x40};
    uint8_t f[64]; size_t fl = 0;
    TEST_ASSERT_TRUE(frame_build(p, sizeof p, f, sizeof f, &fl));
    uint8_t o[64]; size_t ol = 0;
    TEST_ASSERT_FALSE(frame_parse(f, 2, o, sizeof o, &ol));       /* обрізаний */
}

void test_oversize_payload_rejected(void) {
    uint8_t b[FRAME_MAX_PAYLOAD + 1] = {0};
    uint8_t o[600]; size_t ol = 0;
    TEST_ASSERT_FALSE(frame_build(b, sizeof b, o, sizeof o, &ol));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_roundtrip_basic);
    RUN_TEST(test_roundtrip_with_zeros);
    RUN_TEST(test_roundtrip_empty);
    RUN_TEST(test_roundtrip_max_payload);
    RUN_TEST(test_corruption_detected);
    RUN_TEST(test_truncated_rejected);
    RUN_TEST(test_oversize_payload_rejected);
    return UNITY_END();
}

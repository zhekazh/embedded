#include "unity.h"
#include "cobs.h"

void setUp(void) {}
void tearDown(void) {}

/* encode -> перевірка «нема нулів» -> decode -> має збігтися з оригіналом */
static void roundtrip(const uint8_t *data, size_t len) {
    uint8_t enc[400], dec[400];
    size_t enc_len = 0, dec_len = 0;
    TEST_ASSERT_TRUE(cobs_encode(data, len, enc, sizeof enc, &enc_len));
    for (size_t i = 0; i < enc_len; i++) {
        TEST_ASSERT_TRUE(enc[i] != 0);          /* головна властивість COBS */
    }
    TEST_ASSERT_TRUE(cobs_decode(enc, enc_len, dec, sizeof dec, &dec_len));
    TEST_ASSERT_EQUAL_INT((int)len, (int)dec_len);
    if (len > 0) {
        TEST_ASSERT_EQUAL_UINT8_ARRAY(data, dec, len);
    }
}

void test_encode_known_vector(void) {
    const uint8_t in[]  = {0x11, 0x22, 0x00, 0x33};
    const uint8_t exp[] = {0x03, 0x11, 0x22, 0x02, 0x33};
    uint8_t out[16]; size_t out_len = 0;
    TEST_ASSERT_TRUE(cobs_encode(in, sizeof in, out, sizeof out, &out_len));
    TEST_ASSERT_EQUAL_INT((int)sizeof exp, (int)out_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp, out, sizeof exp);
}

void test_decode_known_vector(void) {
    const uint8_t in[]  = {0x03, 0x11, 0x22, 0x02, 0x33};
    const uint8_t exp[] = {0x11, 0x22, 0x00, 0x33};
    uint8_t out[16]; size_t out_len = 0;
    TEST_ASSERT_TRUE(cobs_decode(in, sizeof in, out, sizeof out, &out_len));
    TEST_ASSERT_EQUAL_INT((int)sizeof exp, (int)out_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp, out, sizeof exp);
}

void test_empty_input(void) {
    const uint8_t in[1] = {0xAA};            /* валідний вказівник, але len=0 */
    uint8_t out[4]; size_t out_len = 0;
    TEST_ASSERT_TRUE(cobs_encode(in, 0, out, sizeof out, &out_len));
    TEST_ASSERT_EQUAL_INT(1, (int)out_len);
    TEST_ASSERT_EQUAL_HEX8(0x01, out[0]);

    const uint8_t enc[] = {0x01};
    uint8_t dec[4]; size_t dec_len = 99;
    TEST_ASSERT_TRUE(cobs_decode(enc, sizeof enc, dec, sizeof dec, &dec_len));
    TEST_ASSERT_EQUAL_INT(0, (int)dec_len);
}

void test_trailing_zero(void) {
    const uint8_t in[]  = {0x11, 0x00};
    const uint8_t exp[] = {0x02, 0x11, 0x01};
    uint8_t out[16]; size_t out_len = 0;
    TEST_ASSERT_TRUE(cobs_encode(in, sizeof in, out, sizeof out, &out_len));
    TEST_ASSERT_EQUAL_INT((int)sizeof exp, (int)out_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp, out, sizeof exp);
    roundtrip(in, sizeof in);
}

void test_no_zeros(void) {
    const uint8_t in[] = {0x11, 0x22, 0x33};
    roundtrip(in, sizeof in);
}

void test_roundtrip_with_zeros(void) {
    const uint8_t a[] = {0x00};
    const uint8_t b[] = {0x00, 0x00, 0x00};
    const uint8_t c[] = {0x01, 0x00, 0x02, 0x00, 0x00, 0x03};
    roundtrip(a, sizeof a);
    roundtrip(b, sizeof b);
    roundtrip(c, sizeof c);
}

void test_long_run_no_zeros(void) {
    uint8_t in[300];
    for (size_t i = 0; i < sizeof in; i++) in[i] = (uint8_t)(i % 255 + 1); /* без нулів, >254 */
    roundtrip(in, sizeof in);                /* перетинає межу 0xFF-групи */
}

void test_encode_capacity_too_small(void) {
    const uint8_t in[] = {0x11, 0x22, 0x33};
    uint8_t out[2]; size_t out_len = 0;      /* треба 4, дамо лише 2 */
    TEST_ASSERT_FALSE(cobs_encode(in, sizeof in, out, sizeof out, &out_len));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_encode_known_vector);
    RUN_TEST(test_decode_known_vector);
    RUN_TEST(test_empty_input);
    RUN_TEST(test_trailing_zero);
    RUN_TEST(test_no_zeros);
    RUN_TEST(test_roundtrip_with_zeros);
    RUN_TEST(test_long_run_no_zeros);
    RUN_TEST(test_encode_capacity_too_small);
    return UNITY_END();
}

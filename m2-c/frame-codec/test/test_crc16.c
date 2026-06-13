#include "unity.h"
#include "crc16.h"

void setUp(void) {}
void tearDown(void) {}

void test_known_vector(void) {
    const uint8_t s[] = "123456789";          /* беремо 9 символів */
    TEST_ASSERT_EQUAL_HEX16(0x29B1, crc16(s, 9));
}

void test_empty_is_init(void) {
    const uint8_t x[1] = {0xAA};
    TEST_ASSERT_EQUAL_HEX16(0xFFFF, crc16(x, 0));
}

void test_single_bytes_differ(void) {
    const uint8_t a[] = {0x00};
    const uint8_t b[] = {0x01};
    TEST_ASSERT_EQUAL_HEX16(0xE1F0, crc16(a, 1));
    TEST_ASSERT_EQUAL_HEX16(0xF1D1, crc16(b, 1));
}

void test_single_bit_flip_detected(void) {
    const uint8_t d[]  = {0xDE, 0xAD, 0xBE, 0xEF};
    const uint8_t d2[] = {0xDE, 0xAD, 0xBF, 0xEF};   /* один біт інакше */
    uint16_t c1 = crc16(d, 4), c2 = crc16(d2, 4);
    TEST_ASSERT_EQUAL_HEX16(0x4097, c1);
    TEST_ASSERT_EQUAL_HEX16(0x73A6, c2);
    TEST_ASSERT_TRUE(c1 != c2);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_known_vector);
    RUN_TEST(test_empty_is_init);
    RUN_TEST(test_single_bytes_differ);
    RUN_TEST(test_single_bit_flip_detected);
    return UNITY_END();
}

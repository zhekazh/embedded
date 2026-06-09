#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

static void dump(const char *label, const uint8_t *p, size_t n) {
    printf("%-28s: ", label);
    for (size_t i = 0; i < n; i++) printf("%02x ", (unsigned)p[i]);
    printf("\n");
}

/* серіалізація через ЗНАЧЕННЯ (зсуви) -> порядок байтів фіксований на будь-якому хості */
static void put_u32_le(uint8_t *buf, uint32_t v) {
    buf[0] = (uint8_t)( v        & 0xFF);
    buf[1] = (uint8_t)((v >> 8)  & 0xFF);
    buf[2] = (uint8_t)((v >> 16) & 0xFF);
    buf[3] = (uint8_t)((v >> 24) & 0xFF);
}
static void put_u32_be(uint8_t *buf, uint32_t v) {
    buf[0] = (uint8_t)((v >> 24) & 0xFF);
    buf[1] = (uint8_t)((v >> 16) & 0xFF);
    buf[2] = (uint8_t)((v >> 8)  & 0xFF);
    buf[3] = (uint8_t)( v        & 0xFF);
}
static uint32_t get_u32_le(const uint8_t *buf) {
    return (uint32_t)buf[0]
         | (uint32_t)buf[1] << 8
         | (uint32_t)buf[2] << 16
         | (uint32_t)buf[3] << 24;
}

int main(void) {
    uint32_t x = 0x11223344u;
    dump("сирі байти в пам'яті (host)", (const uint8_t *)&x, sizeof x);
    uint8_t le[4], be[4];
    put_u32_le(le, x);
    put_u32_be(be, x);
    dump("put_u32_le (фіксовано)", le, 4);
    dump("put_u32_be (фіксовано)", be, 4);
    printf("get_u32_le(le) = 0x%08x\n", get_u32_le(le));
    return 0;
}

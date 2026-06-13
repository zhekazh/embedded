#ifndef CRC16_H
#define CRC16_H
#include <stddef.h>
#include <stdint.h>
/* CRC-16/CCITT-FALSE: poly 0x1021, init 0xFFFF, без рефлексії та фінального XOR.
 * Контрольне значення для ASCII "123456789" = 0x29B1. */
uint16_t crc16(const uint8_t *data, size_t len);
#endif

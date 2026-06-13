#ifndef FRAME_H
#define FRAME_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define FRAME_MAX_PAYLOAD 256

/* Кадр на лінії (між роздільниками 0x00):  COBS( payload || crc16_le(payload) )
 * frame_build додає завершальний роздільник 0x00.
 * frame_parse приймає ВИРІЗАНИЙ кадр без роздільника. */
bool frame_build(const uint8_t *payload, size_t len,
                 uint8_t *out, size_t out_cap, size_t *out_len);

bool frame_parse(const uint8_t *frame, size_t frame_len,
                 uint8_t *out, size_t out_cap, size_t *out_len);
#endif

#include "frame.h"
#include "cobs.h"
#include "crc16.h"

bool frame_build(const uint8_t *payload, size_t len,
                 uint8_t *out, size_t out_cap, size_t *out_len) {
    if (len > FRAME_MAX_PAYLOAD) return false;
    uint8_t tmp[FRAME_MAX_PAYLOAD + 2];
    for (size_t i = 0; i < len; i++) tmp[i] = payload[i];
    uint16_t crc = crc16(payload, len);
    tmp[len]     = (uint8_t)(crc & 0xFF);          /* CRC little-endian */
    tmp[len + 1] = (uint8_t)(crc >> 8);
    size_t enc_len = 0;
    if (!cobs_encode(tmp, len + 2, out, out_cap, &enc_len)) return false;
    if (enc_len >= out_cap) return false;          /* місце під роздільник */
    out[enc_len] = 0x00;
    *out_len = enc_len + 1;
    return true;
}

bool frame_parse(const uint8_t *frame, size_t frame_len,
                 uint8_t *out, size_t out_cap, size_t *out_len) {
    uint8_t tmp[FRAME_MAX_PAYLOAD + 2];
    size_t dec_len = 0;
    if (!cobs_decode(frame, frame_len, tmp, sizeof tmp, &dec_len)) return false;
    if (dec_len < 2) return false;                 /* мусить містити хоча б CRC */
    size_t payload_len = dec_len - 2;
    uint16_t got  = (uint16_t)(tmp[payload_len] | ((uint16_t)tmp[payload_len + 1] << 8));
    uint16_t want = crc16(tmp, payload_len);
    if (got != want) return false;                 /* пошкодження */
    if (payload_len > out_cap) return false;
    for (size_t i = 0; i < payload_len; i++) out[i] = tmp[i];
    *out_len = payload_len;
    return true;
}

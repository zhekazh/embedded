#include "cobs.h"

bool cobs_encode(const uint8_t *in, size_t in_len,
                 uint8_t *out, size_t out_cap, size_t *out_len) {
    if (out_cap < 1) return false;
    size_t write = 0;
    size_t code_idx = write++;   /* резервуємо слот під код-байт (індекс 0) */
    uint8_t code = 1;

    for (size_t read = 0; read < in_len; read++) {
        uint8_t b = in[read];
        if (b != 0) {
            if (write >= out_cap) return false;
            out[write++] = b;
            code++;
        }
        if (b == 0 || code == 0xFF) {       /* закриваємо групу: нуль або повна (254) */
            out[code_idx] = code;
            code = 1;
            if (write >= out_cap) return false;
            code_idx = write++;             /* резервуємо наступний код-байт */
        }
    }
    out[code_idx] = code;
    *out_len = write;
    return true;
}

bool cobs_decode(const uint8_t *in, size_t in_len,
                 uint8_t *out, size_t out_cap, size_t *out_len) {
    size_t read = 0, write = 0;
    while (read < in_len) {
        uint8_t code = in[read++];
        if (code == 0) return false;        /* нуль у COBS-потоці неможливий -> бите */
        for (uint8_t i = 1; i < code; i++) {
            if (read >= in_len) return false;   /* обрізаний/битий кадр */
            if (write >= out_cap) return false;
            out[write++] = in[read++];
        }
        if (code != 0xFF && read < in_len) {/* прихований нуль, крім 0xFF-групи й останньої */
            if (write >= out_cap) return false;
            out[write++] = 0;
        }
    }
    *out_len = write;
    return true;
}

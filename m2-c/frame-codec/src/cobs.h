#ifndef COBS_H
#define COBS_H

#include <stddef.h>   /* size_t  */
#include <stdint.h>   /* uint8_t */
#include <stdbool.h>  /* bool    */

/* COBS (Consistent Overhead Byte Stuffing).
 * Кодування прибирає з даних усі нульові байти, щоб 0x00 можна було
 * використати як роздільник кадрів. Сховище виходу дає викликач.
 *
 * Розмір out для encode: щонайбільше in_len + in_len/254 + 1 байтів.
 * Повертають false при замалому out_cap або (для decode) битому кодуванні.
 * Фактична довжина результату пишеться в *out_len. */
bool cobs_encode(const uint8_t *in, size_t in_len,
                 uint8_t *out, size_t out_cap, size_t *out_len);

bool cobs_decode(const uint8_t *in, size_t in_len,
                 uint8_t *out, size_t out_cap, size_t *out_len);

#endif /* COBS_H */

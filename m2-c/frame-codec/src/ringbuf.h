#ifndef RINGBUF_H
#define RINGBUF_H

#include <stddef.h>   /* size_t  */
#include <stdint.h>   /* uint8_t */
#include <stdbool.h>  /* bool    */

/* SPSC байтовий кільцевий буфер.
 * Сховище (storage) надає викликач; модуль ним лише керує, не володіє.
 * Корисна місткість = size - 1 (один слот жертвуємо на розрізнення повний/порожній).
 * Поля приватні за конвенцією: чіпай лише через rb_* функції. */
typedef struct {
    uint8_t *buf;    /* сховище викликача                   */
    size_t   size;   /* розмір storage у байтах             */
    size_t   head;   /* індекс запису; пише лише producer   */
    size_t   tail;   /* індекс читання; пише лише consumer  */
} ringbuf_t;

void   rb_init(ringbuf_t *rb, uint8_t *storage, size_t size);
bool   rb_push(ringbuf_t *rb, uint8_t byte);   /* false, якщо повний   */
bool   rb_pop (ringbuf_t *rb, uint8_t *out);   /* false, якщо порожній */
bool   rb_is_empty(const ringbuf_t *rb);
bool   rb_is_full (const ringbuf_t *rb);
size_t rb_count   (const ringbuf_t *rb);       /* байтів зараз у буфері */

#endif /* RINGBUF_H */

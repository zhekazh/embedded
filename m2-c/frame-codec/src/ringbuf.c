#include "ringbuf.h"

void rb_init(ringbuf_t *rb, uint8_t *storage, size_t size) {
    rb->buf  = storage;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
}

bool rb_is_empty(const ringbuf_t *rb) {
    return rb->head == rb->tail;
}

bool rb_is_full(const ringbuf_t *rb) {
    return (rb->head + 1) % rb->size == rb->tail;
}

size_t rb_count(const ringbuf_t *rb) {
    return (rb->head + rb->size - rb->tail) % rb->size;
}

bool rb_push(ringbuf_t *rb, uint8_t byte) {
    if (rb_is_full(rb)) {
        return false;
    }
    rb->buf[rb->head] = byte;
    rb->head = (rb->head + 1) % rb->size;
    return true;
}

bool rb_pop(ringbuf_t *rb, uint8_t *out) {
    if (rb_is_empty(rb)) {
        return false;
    }
    *out = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) % rb->size;
    return true;
}

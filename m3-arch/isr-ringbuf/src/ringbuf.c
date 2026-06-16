#include "ringbuf.h"

void rb_init(ringbuf_t *rb, uint8_t *storage, size_t size) {
    rb->buf  = storage;
    rb->size = size;
    atomic_store_explicit(&rb->head, 0, memory_order_relaxed);
    atomic_store_explicit(&rb->tail, 0, memory_order_relaxed);
}

bool rb_push(ringbuf_t *rb, uint8_t byte) {            /* producer (ISR) */
    size_t head = atomic_load_explicit(&rb->head, memory_order_relaxed); /* свій індекс */
    size_t next = (head + 1) % rb->size;
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire); /* прогрес consumer */
    if (next == tail) return false;                    /* повний */
    rb->buf[head] = byte;                              /* 1) дані */
    atomic_store_explicit(&rb->head, next, memory_order_release); /* 2) публікація */
    return true;
}

bool rb_pop(ringbuf_t *rb, uint8_t *out) {             /* consumer (main) */
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_relaxed); /* свій індекс */
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire); /* прогрес producer */
    if (head == tail) return false;                    /* порожній */
    *out = rb->buf[tail];                              /* 1) читаємо слот */
    atomic_store_explicit(&rb->tail, (tail + 1) % rb->size, memory_order_release); /* 2) звільняємо */
    return true;
}

bool rb_is_empty(const ringbuf_t *rb) {
    return atomic_load_explicit(&rb->head, memory_order_acquire)
        == atomic_load_explicit(&rb->tail, memory_order_acquire);
}
bool rb_is_full(const ringbuf_t *rb) {
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    return (head + 1) % rb->size == tail;
}
size_t rb_count(const ringbuf_t *rb) {
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    return (head + rb->size - tail) % rb->size;
}

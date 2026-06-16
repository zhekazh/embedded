// SPSC stress: один продюсер, один консюмер. Ганяти під ThreadSanitizer.
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "ringbuf.h"

#define N    1000000u   // байтів прогнати
#define CAP  8          // маленьке сховище → часті full/empty → максимум контенції

static ringbuf_t rb;
static uint8_t   storage[CAP];

static void *producer(void *arg) {
    (void)arg;
    for (uint32_t i = 0; i < N; i++) {
        while (!rb_push(&rb, (uint8_t)i)) { /* повний → spin */ }
    }
    return NULL;
}
static void *consumer(void *arg) {
    (void)arg;
    for (uint32_t i = 0; i < N; i++) {
        uint8_t out;
        while (!rb_pop(&rb, &out)) { /* порожній → spin */ }
        assert(out == (uint8_t)i);   // FIFO: і-тий байт == i (mod 256)
    }
    return NULL;
}
int main(void) {
    rb_init(&rb, storage, sizeof storage);
    pthread_t pt, ct;
    pthread_create(&ct, NULL, consumer, NULL);
    pthread_create(&pt, NULL, producer, NULL);
    pthread_join(pt, NULL);
    pthread_join(ct, NULL);
    assert(rb_is_empty(&rb));
    printf("TSAN SPSC: %u bytes, FIFO ok, buffer drained\n", N);
    return 0;
}

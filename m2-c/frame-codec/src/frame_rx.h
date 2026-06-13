#ifndef FRAME_RX_H
#define FRAME_RX_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Викликається на КОЖЕН валідний (CRC-перевірений) кадр.
 * payload дійсний лише під час виклику; ctx — користувацький контекст. */
typedef void (*frame_handler_t)(const uint8_t *payload, size_t len, void *ctx);

typedef struct {
    uint8_t        *acc;       /* буфер-акумулятор (сховище викликача) */
    size_t          acc_cap;
    size_t          acc_len;
    bool            overflow;  /* кадр не вліз -> бракуємо до роздільника */
    frame_handler_t handler;
    void           *ctx;
} frame_rx_t;

void frame_rx_init(frame_rx_t *rx, uint8_t *acc, size_t acc_cap,
                   frame_handler_t handler, void *ctx);
void frame_rx_push(frame_rx_t *rx, uint8_t byte);
#endif

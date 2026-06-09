#include "frame_rx.h"
#include "frame.h"

void frame_rx_init(frame_rx_t *rx, uint8_t *acc, size_t acc_cap,
                   frame_handler_t handler, void *ctx) {
    rx->acc = acc; rx->acc_cap = acc_cap; rx->acc_len = 0;
    rx->overflow = false; rx->handler = handler; rx->ctx = ctx;
}

void frame_rx_push(frame_rx_t *rx, uint8_t byte) {
    if (byte != 0x00) {
        if (rx->acc_len < rx->acc_cap) rx->acc[rx->acc_len++] = byte;
        else rx->overflow = true;              /* кадр бракований */
        return;
    }
    if (!rx->overflow && rx->acc_len > 0) {    /* межа кадру */
        uint8_t payload[FRAME_MAX_PAYLOAD];
        size_t plen = 0;
        if (frame_parse(rx->acc, rx->acc_len, payload, sizeof payload, &plen)) {
            if (rx->handler) rx->handler(payload, plen, rx->ctx);
        }                                      /* биті кадри тихо ігноруємо */
    }
    rx->acc_len = 0; rx->overflow = false;
}

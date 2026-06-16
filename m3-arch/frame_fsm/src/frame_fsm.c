#include "frame_fsm.h"

void fsm_init(frame_fsm_t *f) { f->state = FSM_WAIT_STX; f->payload = 0; }

bool fsm_feed(frame_fsm_t *f, uint8_t b, uint8_t *out) {
    switch (f->state) {
    case FSM_WAIT_STX:
        if (b == FSM_STX) f->state = FSM_WAIT_DATA;
        return false;                         /* шум до STX ігноруємо */

    case FSM_WAIT_DATA:
        f->payload = b;
        f->state = FSM_WAIT_ETX;
        return false;

    case FSM_WAIT_ETX:
        if (b == FSM_ETX) {                    /* валідний кадр */
            *out = f->payload;
            f->state = FSM_WAIT_STX;
            return true;
        }
        /* очікували ETX — ресинхронізація */
        f->state = (b == FSM_STX) ? FSM_WAIT_DATA : FSM_WAIT_STX;
        return false;
    }
    return false;                             /* недосяжно; -Wall любить повний шлях */
}

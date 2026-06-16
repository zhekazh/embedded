#ifndef FRAME_FSM_H
#define FRAME_FSM_H
#include <stdint.h>
#include <stdbool.h>

#define FSM_STX 0x02
#define FSM_ETX 0x03

typedef enum {
    FSM_WAIT_STX,    /* чекаємо початок кадру          */
    FSM_WAIT_DATA,   /* є STX, чекаємо байт даних      */
    FSM_WAIT_ETX,    /* є дані, чекаємо кінець кадру    */
} fsm_state_t;

typedef struct {
    fsm_state_t state;
    uint8_t     payload;
} frame_fsm_t;

void fsm_init(frame_fsm_t *f);
/* згодувати один байт; true = щойно завершився валідний кадр (*out = payload).
 * Не блокує й нічого не чекає — суто подієво. */
bool fsm_feed(frame_fsm_t *f, uint8_t b, uint8_t *out);
#endif

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "driver/gptimer.h"
#include "soc/gpio_reg.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_periph.h"
#include "ringbuf.h"
#include "frame_fsm.h"

#define LED       4
#define LED_MASK  (1UL << LED)

static ringbuf_t         rb;
static uint8_t           rb_storage[16];
static volatile uint32_t g_dropped = 0;

/* ISR-ПРОДЮСЕР: 1 байт/тік, кадр STX·літера·ETX (літери 'A'..'Z' не колізують зі STX/ETX) */
static bool IRAM_ATTR on_timer(gptimer_handle_t timer,
                               const gptimer_alarm_event_data_t *edata,
                               void *user_ctx)
{
    static int     phase  = 0;
    static uint8_t letter = 'A';
    uint8_t b;

    switch (phase) {
    case 0:  b = FSM_STX; break;
    case 1:  b = letter;  break;
    default: b = FSM_ETX;
             letter = (letter == 'Z') ? 'A' : letter + 1;
             break;
    }
    phase = (phase + 1) % 3;

    if (!rb_push(&rb, b)) g_dropped++;     /* rb_push у IRAM — cache-safe */
    return false;
}

void app_main(void)
{
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[LED], PIN_FUNC_GPIO);
    *(volatile uint32_t *)GPIO_ENABLE_W1TS_REG = LED_MASK;

    rb_init(&rb, rb_storage, sizeof rb_storage);

    frame_fsm_t fsm;
    fsm_init(&fsm);

    gptimer_handle_t timer = NULL;
    gptimer_config_t tcfg = { .clk_src = GPTIMER_CLK_SRC_DEFAULT,
                              .direction = GPTIMER_COUNT_UP, .resolution_hz = 1000000 };
    ESP_ERROR_CHECK(gptimer_new_timer(&tcfg, &timer));
    gptimer_event_callbacks_t cbs = { .on_alarm = on_timer };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer, &cbs, NULL));
    ESP_ERROR_CHECK(gptimer_enable(timer));
    gptimer_alarm_config_t acfg = { .alarm_count = 200000,  /* 200 мс/байт → кадр ~600 мс */
                                    .reload_count = 0, .flags.auto_reload_on_alarm = true };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(timer, &acfg));
    ESP_ERROR_CHECK(gptimer_start(timer));

    bool led_on = false;
    for (;;) {
        uint8_t b, payload;
        /* КОНСЮМЕР: забираємо все наявне й годуємо FSM — неблокуюче */
        while (rb_pop(&rb, &b)) {
            if (fsm_feed(&fsm, b, &payload)) {           /* кадр завершено */
                led_on = !led_on;                         /* LED = подія «кадр» */
                if (led_on) *(volatile uint32_t *)GPIO_OUT_W1TS_REG = LED_MASK;
                else        *(volatile uint32_t *)GPIO_OUT_W1TC_REG = LED_MASK;
                printf("frame: %c  (dropped: %u)\n", payload, (unsigned)g_dropped);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));   /* короткий yield; FSM ніколи не чекає на байт */
    }
}

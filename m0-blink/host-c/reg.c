// Біт-операції над «регістром» GPIO: set/clear/toggle одного біта.
// Детальний розбір (маски, 1u, вказівники, read-modify-write, volatile, порти STM32)
// — у README.md поруч.
#include <stdio.h>
#include <stdint.h>   // uint8_t = рівно 8 біт без знаку (на МК розмір типу важливий)
#include <assert.h>   // assert — наш міні-тест замість «звіряння очима»

// "Регістр" GPIO: 1 байт, біт N керує ніжкою N. На залізі — volatile uint8_t* (MMIO).
static uint8_t gpio_reg = 0;

void set_bit(uint8_t *reg, int pin)    { *reg |=  (1u << pin); }  // set:    OR з маскою
void clear_bit(uint8_t *reg, int pin)  { *reg &= ~(1u << pin); }  // clear:  AND з ~маскою
void toggle_bit(uint8_t *reg, int pin) { *reg ^=  (1u << pin); }  // toggle: XOR з маскою

// Друк байта у двійковому вигляді: від старшого біта (7) до молодшого (0).
void print_reg(uint8_t reg) {
    for (int i = 7; i >= 0; i--)
        putchar((reg & (1u << i)) ? '1' : '0');
    putchar('\n');
}

int main(void) {
    print_reg(gpio_reg);          // 00000000
    assert(gpio_reg == 0x00);
    set_bit(&gpio_reg, 2);        // digitalWrite(2, HIGH) → 00000100
    print_reg(gpio_reg);
    assert(gpio_reg == 0x04);
    set_bit(&gpio_reg, 5);        //                       → 00100100
    print_reg(gpio_reg);
    assert(gpio_reg == 0x24);
    clear_bit(&gpio_reg, 2);      // digitalWrite(2, LOW)  → 00100000
    print_reg(gpio_reg);
    assert(gpio_reg == 0x20);
    toggle_bit(&gpio_reg, 5);     //                       → 00000000
    print_reg(gpio_reg);
    assert(gpio_reg == 0x00);
    return 0;
}

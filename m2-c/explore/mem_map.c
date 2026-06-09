#include <stdio.h>
#include <stdlib.h>

int g_initialized = 42;      /* .data  — глобал із ненульовим значенням */
int g_zero;                  /* .bss   — глобал, нуль за замовчуванням   */

void some_function(void) { } /* .text  — код                            */

int main(void) {
    int  local   = 1;                    /* стек                         */
    int *heap    = malloc(sizeof(int));  /* купа                         */
    static int s_static = 7;             /* .data — статик у функції     */

    printf("код   .text   some_function : %p\n", (void *)some_function);
    printf("дані  .data   &g_initialized: %p\n", (void *)&g_initialized);
    printf("дані  .bss    &g_zero       : %p\n", (void *)&g_zero);
    printf("дані  .data   &s_static     : %p\n", (void *)&s_static);
    printf("купа  heap    heap          : %p\n", (void *)heap);
    printf("стек          &local        : %p\n", (void *)&local);

    free(heap);
    return 0;
}

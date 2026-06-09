#include <stdio.h>
int *g_ptr;
void stash(void) {
    int local = 777;
    g_ptr = &local;        /* зберігаємо адресу локалки в глобал */
}
int main(void) {
    stash();               /* кадр stash() щойно помер */
    printf("%d\n", *g_ptr);/* читаємо з мертвого стек-фрейму */
    return 0;
}

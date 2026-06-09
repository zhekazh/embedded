#include <stdio.h>
#include <stddef.h>   /* offsetof */
#include <stdint.h>

struct Bad {          /* погане впорядкування полів */
    uint8_t  a;       /* 1 байт                       */
    uint32_t b;       /* 4 байти, вирівнювання на 4    */
    uint8_t  c;       /* 1 байт                       */
};

struct Good {         /* ті самі поля, інший порядок  */
    uint32_t b;
    uint8_t  a;
    uint8_t  c;
};

int main(void) {
    printf("sizeof(uint8_t)=%zu  sizeof(uint32_t)=%zu\n",
           sizeof(uint8_t), sizeof(uint32_t));
    printf("\nstruct Bad : sizeof=%zu  (а даних усього 6 байтів)\n", sizeof(struct Bad));
    printf("  offsetof a=%zu  b=%zu  c=%zu\n",
           offsetof(struct Bad, a), offsetof(struct Bad, b), offsetof(struct Bad, c));
    printf("  _Alignof=%zu\n", _Alignof(struct Bad));
    printf("\nstruct Good: sizeof=%zu\n", sizeof(struct Good));
    printf("  offsetof b=%zu  a=%zu  c=%zu\n",
           offsetof(struct Good, b), offsetof(struct Good, a), offsetof(struct Good, c));
    printf("  _Alignof=%zu\n", _Alignof(struct Good));
    return 0;
}

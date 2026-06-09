#include <stdio.h>

void print_size(int arr[]) {     /* arr тут — НЕ масив, а вказівник */
    printf("у функції: sizeof(arr) = %zu  <- розмір ВКАЗІВНИКА, не масиву\n", sizeof arr);
}

int main(void) {
    int a[4] = {10, 20, 30, 40};

    /* 1. арифметика масштабується типом елемента */
    printf("&a[0] = %p\n", (void *)&a[0]);
    printf("&a[1] = %p\n", (void *)&a[1]);
    printf("&a[2] = %p\n", (void *)&a[2]);
    printf("крок між елементами = %ld байтів (sizeof(int) = %zu)\n\n",
           (long)((char *)&a[1] - (char *)&a[0]), sizeof(int));

    /* 2. a[i] — це буквально *(a + i) */
    printf("a[2]     = %d\n", a[2]);
    printf("*(a + 2) = %d\n", *(a + 2));
    printf("2[a]     = %d  <- легально й однаково\n\n", 2[a]);

    /* 3. масив vs вказівник: розмір */
    printf("у main:  sizeof(a) = %zu  (4 int = 16 байтів)\n", sizeof a);
    print_size(a);
    return 0;
}

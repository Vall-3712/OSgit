#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define STR_MAX 32

char* transfer(int a) {   // to binary
    char *temp = calloc(sizeof(char), STR_MAX);
    int sign = (a >= 0) ? 0 : 1;
    int b = (a > 0) ? a : -a;
    int i = 0;
    while (b != 0) {
        temp[i++] = '0' + (b % 2);
        b /= 2;
    }
    char *result = calloc(sizeof(char), i + sign);
    for (int j = 0; j < i; ++j) {
        result[j + sign] = temp[i - j - 1];
    }
    if (sign == 1) {
        result[0] = '-';
    }
    free(temp);
    return(result);

}

void sort(int *a, int n) {
    printf("\nThis is bubble sort\n");
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            if (a[j + 1] < a[j]) {
                int k = a[j];
                a[j] = a[j + 1];
                a[j + 1] = k;
            }
        }
    }
}
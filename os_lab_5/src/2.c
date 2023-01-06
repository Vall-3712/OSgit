#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define STR_MAX 32

char* transfer(int a) {   // to ternary
    char *temp = calloc(sizeof(char), STR_MAX);
    int sign = (a >= 0) ? 0 : 1;
    int b = (a > 0) ? a : -a;
    int i = 0;
    while (b != 0) {
        temp[i++] = '0' + (b % 3);
        b /= 3;
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

void quicksort(int* a, int f, int l) {
    int m = a[(f + l) / 2];
    int i = f, j = l;
    do {
        while(a[i] < m) {
            i++;
        }
        while(a[j] > m) {
            j--;
        }
        if (i <= j) {
            if (i < j) {
                int temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
            i++;
            j--;
        }
    } while(i <= j);
    if (i < l) {
        quicksort(a, i, l);
    }
    if (j > f) {
        quicksort(a, f, j);
    }
}

void sort(int *a, int n) {
    printf("\nThis is quicksort\n");
    quicksort(a, 0, n - 1);
}
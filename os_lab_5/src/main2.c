#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>

#define STR_MAX 32
#define MAX_SIZE 128

//Подсчёт количества простых чисел на отрезке [A, B] (A, B - натуральные)
//Перевод числа x из десятичной системы счисления в другую

int read_number(int *result) {
    char *buf = calloc(sizeof(char), STR_MAX);
    char c = 0;
    short i = 0;

    if (read(0, &c, 1) < 0) {
        exit(1);
    }
    if ((c == ' ') || (c == '\n') || (c == '\0'))
        return 2;
    while (c != ' ' && c != '\n') {
        buf[i++] = c;
        read(0, &c, 1);
    }
    *result = atoi(buf);
    free(buf);
    if (c == '\n') {
        return 1;
    }
    return 0;
}

int main() {
    int var = 1;
    void *handle = dlopen("./liblib1.so", RTLD_LAZY);
    char*(*transfer)(int) = dlsym(handle, "transfer");
    void(*sort)(int*, int) = dlsym(handle, "sort");

    printf("\nThis is var %d\n\n", var);

    int command;
    read_number(&command);
    
    while (command != -1) {
        switch (command) {
            case 0:
                if (var == 1) {
                    dlclose(handle);
                    var = 2;
                    handle = dlopen("./liblib2.so", RTLD_LAZY);
                    transfer = dlsym(handle, "transfer");
                    sort = dlsym(handle, "sort");
                }
                else {
                    dlclose(handle);
                    var = 1;
                    handle = dlopen("./liblib1.so", RTLD_LAZY);
                    transfer = dlsym(handle, "transfer");
                    sort = dlsym(handle, "sort");
                }
                printf("\nThis is var %d\n\n", var);
                break;
            case 1:
                int t;
                scanf("%d", &t);
                char *result = (*transfer)(t);
                printf("\n%s\n\n", result);
                break;
            case 2:
                int c, last;
                int i = 0;
                int *a = malloc(MAX_SIZE * sizeof(int));
                do {
                    last = read_number(&c);
                    if (last == 2) {
                        break;
                    }
                    a[i++] = c;
                } while (last == 0);
                a = realloc(a, i * sizeof(int));
                (*sort)(a, i);
                for (int j = 0; j < i; ++j) {
                    printf("%d ", a[j]);
                }
                printf("\n\n");
                break;
            default:
                printf("unknown command: %d\n", command);
                break;
        }
        read_number(&command);
    }
    dlclose(handle);
    return 0;
}
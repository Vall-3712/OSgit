#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define STR_MAX 32
#define MAX_SIZE 128

char* transfer(int);
void sort(int*, int);

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
    printf("Choose command \n 1 - is for changing the excision system \n 2 - is for sorting\n");
    
    int command;
    read_number(&command);
    
    while (command != -1) {
        switch (command) {
            case 1:
                int t;
                scanf("%d", &t);
                char *result = transfer(t);
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
                sort(a, i);
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
    return 0;
}
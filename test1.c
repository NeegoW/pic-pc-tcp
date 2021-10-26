#include <stdio.h>
#include "log.c"

int split(unsigned char dst[][8], char *str, const char *spl) {
    int n = 0;
    char *result = NULL;
    result = strtok(str, spl);
    while (result != NULL) {
        strcpy(dst[n++], result);
        result = strtok(NULL, spl);
    }
    return n;
}

int main(int argc, char *argv[]) {
    char str[] = "what is 123 your name?";
    char tmp[255];
    strcpy(tmp, str);
    unsigned char dst[4][8];
    int cnt = split(dst, tmp, " ");
    for (int i = 0; i < cnt; i++)
        puts(dst[i]);
    puts(str);
    return 0;
}

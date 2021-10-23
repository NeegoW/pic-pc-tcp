#include <stdio.h>
#include <windows.h>

int main() {
    FILE *fp = NULL;

    fp = fopen("../tmp/c2.txt", "w+");
    int j = 1;
    while (j <= 2000) {
        fprintf(fp, "1");
        if (j % 5 == 0) fprintf(fp, " ");
        if (j % 50 == 0) {
            fprintf(fp, " %d\n", j);
        }
        j++;
    }

    Sleep(2000);
    fprintf(fp, "\n一共%d\n", j - 1);
    fclose(fp);
    return 0;
}
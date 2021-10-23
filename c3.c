#include <stdio.h>
#include <signal.h>
#include <windows.h>

int stop_flag;

void on_sigint(int p_sig);

int main() {
    stop_flag = 0;
    signal(SIGINT, on_sigint);
    FILE *fp = NULL;

    fp = fopen("../tmp/c3.txt", "w+");
    int j = 1;
    while (j <= 100000) {
        if (stop_flag == 1) break;
        printf("1");
        fprintf(fp, "1");
        if (j % 5 == 0) {
            printf(" ");
            fprintf(fp, " ");
        }
        if (j % 50 == 0) {
            printf("%d\n", j);
            fprintf(fp, " %d\n", j);
        }
        j++;
        Sleep(1);
    }

    fprintf(fp, "\n一共%d\n", j - 1);
    fclose(fp);
    return 0;
}

void on_sigint(int p_sig) {
    stop_flag = 1;
//    printf("receives ctrl+c\n");
//    exit(0);
}
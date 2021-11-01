#include <stdio.h>
#include "log.c"
#include <conio.h>


void data2bin(void);

void bin2data(void);

int split(unsigned char dst[][8], char *str, const char *spl);

unsigned char rx[] = {0, 1, 1, 1, 0, 1, 1, 1,
                      0, 0, 1, 1, 0, 1, 1, 0,
                      1, 1, 0, 1, 0, 0, 1, 1,
                      1, 0, 0, 0, 1, 0, 0, 0};
const unsigned long long BUFSIZE = sizeof(rx) / sizeof(rx[0]);
unsigned char RxData[50];
unsigned int recvIdx = 0;
unsigned int idx = 0;
unsigned char shiftIdx = 6;
int sec = 0;

int main(int argc, char *argv[]) {
//    memset(RxData, 0, BUFSIZE);
//    int k, s = 0;
//    while (idx < BUFSIZE) {
//        data2bin();
//        Sleep(10);
//        if (++sec % 10 == 0)
//            printf("idx:%d\n", idx++);
//
//        if (kbhit()) {
//            k = _getch();
//            if (k == 27) {
//                break;
//            }
//        }
//    }
//
//    bin2data();
    int ackIdx = 0;
    int ACKSIZE = 10;
    int Ack;
    unsigned char AckData[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    while (1) {
        if (ackIdx == ACKSIZE) {
            ackIdx = 0;
            break;
        } else {
            Ack = AckData[ackIdx++];
        }
    }
    printf("ack:%d", Ack);

    return 0;
}

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

void data2bin(void) {
    unsigned char x = rx[idx];
    RxData[recvIdx] |= 0x80;
    RxData[recvIdx] |= x << shiftIdx;
    if (shiftIdx == 0) {
        shiftIdx = 6;
        recvIdx++;
    } else {
        shiftIdx--;
    }
}

void bin2data(void) {
    int j = 0;
    for (int i = 0; i < 50; i++) {
        if ((RxData[i] & 0x80) == 0x80) {
            for (int s = 1; s < 8; s++) {
                int m = 0x80 >> s;
                if ((RxData[i] & m) == m) {
                    printf("1");
                } else {
                    printf("0");
                }
                if (++j % 10 == 0) printf("  l:%d\n", j / 10);
            }
        }

    }
}
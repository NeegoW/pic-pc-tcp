/* @ raw : raw data 1111
 * @ rawLen : len(raw)
 * @ stx : 01110111 (default)
 * @ stxLen : len(stx)
 * @ etx : 10001000 (default)
 * @ etxLen : len(etx)
 * @ done : STX--DMC DATA--ETX
 * */
typedef struct _DMC_CODE {
    unsigned char *stx;
    unsigned char *etx;
    unsigned char *raw;
    unsigned char *done;
    int stxLen;
    int etxLen;
    long rawLen;
    long doneLen;
} DMC;

int dmc_encode(DMC *);

int dmc_decode(DMC *);

void dmc_print(DMC *);

/*
 * dmc[stxLen + 2n] = !dmc[stxLen + 2n - 1]
 * raw[n] == 0? dmc[stxLen + 2n + 1] = !dmc[stxLen + 2n] : dmc[stxLen + 2n + 1] = dmc[stxLen + 2n]
 * dmc[stx, dmc code, etx]
 **/
int dmc_encode(DMC *dmc) {
    for (int i = 0; i < dmc->stxLen; i++) {
        dmc->done[i] = dmc->stx[i];
    }

    for (int i = 0; i < dmc->rawLen; i++) {
        dmc->done[dmc->stxLen + 2 * i] = !dmc->done[dmc->stxLen + 2 * i - 1];
        if (dmc->raw[i] == 0) dmc->done[dmc->stxLen + 2 * i + 1] = !dmc->done[dmc->stxLen + 2 * i];
        else dmc->done[dmc->stxLen + 2 * i + 1] = dmc->done[dmc->stxLen + 2 * i];
    }

    int idx = dmc->stxLen + (dmc->rawLen * 2);
    for (int i = 0; i < dmc->etxLen; i++) {
        dmc->done[i + idx] = dmc->etx[i];
    }
    return 0;
}

/*
 * 11, 00 ---> 1
 * 10, 01 ---> 0
 * */
int dmc_decode(DMC *dmc) {
    for (int i = 0; i < dmc->rawLen; i++) {
        if (dmc->done[dmc->stxLen + 2 * i] == dmc->done[dmc->stxLen + 2 * i + 1]) dmc->raw[i] = 1;
        else dmc->raw[i] = 0;
    }
    return 0;
}


void dmc_print(DMC *dmc) {
    int j;
    printf("stx (len = %d):\n\r", dmc->stxLen);
    for (int i = 0; i < dmc->stxLen; i++) printf("%d", dmc->stx[i]);

    printf("\n\retx (len = %d):\n\r", dmc->etxLen);
    for (int i = 0; i < dmc->etxLen; i++) printf("%d", dmc->etx[i]);

    printf("\n\rraw (len = %ld):\n\r", dmc->rawLen);
    j = 1;
    for (int i = 0; i < dmc->rawLen; i++, j++) {
        printf("%d", dmc->raw[i]);
        if (j % 128 == 0) {
            printf("\n\r");
        }
    }

    printf("\n\rdone (len = %ld):\n\r", dmc->doneLen);
    j = 1;
    for (int i = 0; i < dmc->doneLen; i++, j++) {
        printf("%d", dmc->done[i]);
        if (j % 8 == 0) {
            printf("\n\r");
        }
    }
}
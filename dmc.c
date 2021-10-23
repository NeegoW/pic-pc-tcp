/* @ raw : raw data 1111
 * @ rawLen : len(raw)
 * @ stx : raw data 1111
 * @ stxLen : len(stx)
 * @ etx : raw data 1111
 * @ etxLen : len(etx)
 * @ done : STX--DMC DATA--ETX
 * STX : 01110111, ETX : 10001000
 * */
typedef struct _DMC {
    unsigned char *raw;
    int rawLen;
    char *stx;
    int stxLen;
    char *etx;
    int etxLen;
    unsigned char *done;
} DMC;

int dmc_encode(DMC);

int dmc_decode(DMC);


int dmc_encode(DMC dmc) {
    int idx = 0;
    for (int i = 0; i < dmc.stxLen; i++) {
        dmc.done[i] = dmc.stx[i];
        idx++;
    }

    for (int i = 0; i < dmc.rawLen; i++) {
        if (dmc.raw[i] == 0) {
            if (dmc.done[dmc.stxLen + 2 * i - 1] == 0) {
                dmc.done[dmc.stxLen + 2 * i] = 1;
            } else {
                dmc.done[dmc.stxLen + 2 * i] = 0;
            }
            dmc.done[dmc.stxLen + 2 * i + 1] = !dmc.done[dmc.stxLen + 2 * i];
        } else {
            if (dmc.done[dmc.stxLen + 2 * i - 1] == 0) {
                dmc.done[dmc.stxLen + 2 * i] = 1;
            } else {
                dmc.done[dmc.stxLen + 2 * i] = 0;
            }
            dmc.done[dmc.stxLen + 2 * i + 1] = dmc.done[dmc.stxLen + 2 * i];
        }
        idx += 2;
    }

    for (int i = 0; i < dmc.etxLen; i++) {
        dmc.done[idx] = dmc.etx[i];
        idx++;
    }
    return 0;
}

int dmc_decode(DMC dmc) {
    for (int i = 0; i < dmc.rawLen; i++) {
        if (dmc.done[dmc.stxLen + 2 * i] == dmc.done[dmc.stxLen + 2 * i + 1]) dmc.raw[i] = 1;
        else dmc.raw[i] = 0;
    }
    return 0;
}
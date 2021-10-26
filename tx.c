//////////////////////////////////////////////////////////////////////////////////
// タイマー初期化
// 速度を変更する場合 Init x,y
// 第１引数 3: x256 , 2: x64 , 1: x8 , 0: x1
// 第２引数 65536 (2^16)以下にすること
// G M K 1 m μ n
// CPU 16M
// クロック　分周　　　　　　    カウント
// 1/16M x 256 = 16us      x 62500 = 1000.0ms   [   1Hz]
// 1/16M x 256 = 16us      x 31250 =  500.0ms   [   2Hz]

// 1/16M x  64 =  4us      x 62500 =  250.0ms   [   4Hz]
// 1/16M x  64 =  4us      x 50000 =  200.0ms   [   5Hz]
// 1/16M x  64 =  4us      x 25000 =  100.0ms   [  10Hz]
// 1/16M x  64 =  4us      x 12500 =   50.0ms   [  20Hz]

// 1/16M x   8 =  0.5us    x 50000 =   25.0ms   [  40Hz]
// 1/16M x   8 =  0.5us    x 40000 =   20.0ms   [  50Hz]
// 1/16M x   8 =  0.5us    x 20000 =   10.0ms   [ 100Hz]
// 1/16M x   8 =  0.5us    x 10000 =    5.0ms   [ 200Hz]
// 1/16M x   8 =  0.5us    x  5000 =    2.5ms   [ 400Hz]

// 1/16M x   1 =  0.0625us x 32000 =    2.0ms   [ 500Hz]
// 1/16M x   1 =  0.0625us x 16000 =    1.0ms   [  1KHz]
// 1/16M x   1 =  0.0625us x  8000 =    500us   [  2KHz]
// 1/16M x   1 =  0.0625us x  4000 =    250us   [  4KHz]
// 1/16M x   1 =  0.0625us x  3200 =    200us   [  5KHz]
// 1/16M x   1 =  0.0625us x  1600 =    100us   [ 10KHz]
// 1/16M x   1 =  0.0625us x   800 =     50us   [ 20KHz]
// 1/16M x   1 =  0.0625us x   400 =     25us   [ 40KHz]
// 1/16M x   1 =  0.0625us x   320 =     20us   [ 50KHz]
// 1/16M x   1 =  0.0625us x   160 =     10us   [100KHz]
// 1/16M x   1 =  0.0625us x    80 =      5us   [200KHz]

typedef struct {
    char title[64];
    int tckps;
    int pr;
} PT;

PT pt[22] = {
        "1Hz", 3, 62500 - 1,    //0
        "2Hz", 3, 31250 - 1,    //1

        "4Hz", 2, 62500 - 1,    //2
        "5Hz", 2, 50000 - 1,    //3
        "10Hz", 2, 25000 - 1,   //4
        "20Hz", 2, 12500 - 1,   //5

        "40Hz", 1, 50000 - 1,   //6
        "50Hz", 1, 40000 - 1,   //7
        "100Hz", 1, 20000 - 1,  //8
        "200Hz", 1, 10000 - 1,  //9
        "400Hz", 1, 5000 - 1,   //10

        "500Hz", 0, 32000 - 1,  //11
        "1KHz", 0, 16000 - 1,   //12
        "2KHz", 0, 8000 - 1,    //13
        "4KHz", 0, 4000 - 1,    //14
        "5KHz", 0, 3200 - 1,    //15
        "10KHz", 0, 1600 - 1,   //16
        "20KHz", 0, 800 - 1,    //17
        "40KHz", 0, 400 - 1,    //18
        "50KHz", 0, 320 - 1,    //19
        "100KHz", 0, 160 - 1,   //20
        "200KHz", 0, 80 - 1,    //21
};

#include "log.c"
#include "USBconnect.c"
#include "prbs.c"
#include "dmc_code.c"


int main(int argc, char *argv[]) {
    int idx = 0;
    int times = 4;
    char distance[16];
    switch (argc) {
        case 3:
            strcpy(distance, argv[2]);
        case 2:
            idx = atoi(argv[1]);
        default:
            break;
    }

    //get random data
    unsigned char init[7] = {0, 0, 0, 0, 0, 0, 1};
    unsigned char prbs[128];
    prbs7(init, prbs);

    //initial DMC
    unsigned char stx[] = {0, 1, 1, 1, 0, 1, 1, 1};
    unsigned char etx[] = {1, 1, 1, 0, 1, 1, 1, 0};
    int stxLen = sizeof(stx) / sizeof(stx[0]);
    int etxLen = sizeof(etx) / sizeof(etx[0]);
    int baseLen = 128;
    long rawLen = baseLen * times;
    long doneLen = 2 * rawLen + stxLen + etxLen;
    unsigned char raw[rawLen];
    unsigned char done[doneLen];
    for (int i = 0; i < times; i++) {
        for (int j = 0; j < baseLen; j++) {
            raw[baseLen * i + j] = prbs[j];
        }
    }
    DMC tx = {stx, etx, raw, done, stxLen, etxLen, rawLen, doneLen};
    dmc_encode(&tx);
//    dmc_print(&tx);
//    exit(1);

    //initial USB
    USB usb;
    BOOL bRes;
    char buf[128];
    char fName[128];
    usb.VendorID = 0x04D8;
    usb.ProductID = 0x003F;

    bRes = USBConnect(&usb);
    if (bRes == FALSE) {
        printf("error:%s\n", usb.message);
    } else {
        printf("start\n");
        USBmemset(&usb);

        // Timer Init
        usb.SendBuf[0] = 0x00;
        sprintf(buf, "I %d,%d", pt[idx].tckps, pt[idx].pr);
        strcpy(&usb.SendBuf[1], buf);
        USBWrite(usb);

        // Tx Data Set
        for (int i = 0; i < doneLen; i++) {
            sprintf(buf, "D %d,%d", i, tx.done[i]);
            strcpy(&usb.SendBuf[1], buf);
            USBWrite(usb);
        }

        // Tx data number set
        sprintf(buf, "T %d", doneLen);
        strcpy(&usb.SendBuf[1], buf);
        USBWrite(usb);

        // Run
        usb.SendBuf[1] = 'R';
        USBWrite(usb);

        // waiting
        while (1) {
            usb.SendBuf[1] = 'S';
            USBWrite(usb);

            USBRead(&usb);
            int counter = (usb.RecvBuf[2] << 8) + usb.RecvBuf[3];
            printf("status: %d\n", counter);

            if (counter >= tx.doneLen) {
                break;
            }
        }

        log_set_name(fName, pt[idx].title, distance);
        puts(fName);
        log_w(fName, tx.done, tx.doneLen);

        USBDisConnect(usb);
    }
    return 0;
}
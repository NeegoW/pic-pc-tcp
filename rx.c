//////////////////////////////////////////////////////////////////////////////////
// タイマー初期化
// 速度を変更する場合 Init x,y
// 第１引数 3: x256 , 2: x64 , 1: x8 , 0: x1
// 第２引数 65536 (2^16)以下にすること
// G M K 1 m μ n
// CPU 16M
// クロック　分周　　　　　　    カウント
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
// 1/16M x   1 =  0.0625us x    40 =    2.5us   [400KHz]
// 1/16M x   1 =  0.0625us x    32 =      2us   [500KHz]
// 1/16M x   1 =  0.0625us x    16 =      1us   [  1MHz]
// 1/16M x   1 =  0.0625us x     8 =    0.5us   [  2MHz]

typedef struct {
    char title[64];
    int tckps;
    int pr;
} PT;

//Ttx * 10 = Trx;
PT pt[] = {
        "1Hz", 2, 25000 - 1,   //0
        "2Hz", 2, 12500 - 1,   //1

        "4Hz", 1, 50000 - 1,   //2
        "5Hz", 1, 40000 - 1,   //3
        "10Hz", 1, 20000 - 1,  //4
        "20Hz", 1, 10000 - 1,  //5
        "40Hz", 1, 5000 - 1,   //6

        "50Hz", 0, 32000 - 1,  //7
        "100Hz", 0, 16000 - 1,   //8
        "200Hz", 0, 8000 - 1,    //9
        "400Hz", 0, 4000 - 1,    //10
        "500Hz", 0, 3200 - 1,    //11
        "1KHz", 0, 1600 - 1,   //12
        "2KHz", 0, 800 - 1,    //13
        "4KHz", 0, 400 - 1,    //14
        "5KHz", 0, 320 - 1,    //15
        "10KHz", 0, 160 - 1,   //16
        "20KHz", 0, 80 - 1,    //17
        "40KHz", 0, 40 - 1,    //18
        "50KHz", 0, 32 - 1,    //19
        "100KHz", 0, 16 - 1,    //20
        "200KHz", 0, 8 - 1,    //21
};

#include <signal.h>
#include "log.c"
#include "dmc_code.c"
#include "USBconnect.c"
#include "prbs.c"

//#define DUMP


int stop_flag;

void on_sigint(int p_sig);

int main(int argc, char *argv[]) {
    stop_flag = 0;
    signal(SIGINT, on_sigint);

    int idx = 0;
    char distance[16];
    switch (argc) {
        case 3:
            strcpy(distance, argv[2]);
        case 2:
            idx = atoi(argv[1]);
        default:
            break;
    }

    USB usb;
    BOOL bRes;
    char buf[128];
    int i, j, k = 1;
    unsigned char tmp;

    usb.VendorID = 0x4D8;
    usb.ProductID = 0x3F;

    bRes = USBConnect(&usb);

    if (bRes == FALSE) {
        printf("error:%s\n", usb.message);
    } else {
        printf("start\n");
        USBmemset(&usb);

        // Timer Init
        usb.SendBuf[0] = 0x00;
#ifdef DUMP
        sprintf(buf, "I %d,%d", pt[idx].tckps, pt[idx].pr);
#else
        sprintf(buf, "I %d,%d", pt[idx].tckps, pt[idx].pr);
#endif
        strcpy(&usb.SendBuf[1], buf);
        USBWrite(usb);

        // Run
        usb.SendBuf[1] = 'R';
        USBWrite(usb);

#ifdef DUMP
        char fName[128] = ".\\rx_log\\";

        log_set_name(fName, pt[idx].title, distance);
        puts(fName);
        FILE *fp;
        fp = fopen(fName, "w");
#endif
        // waiting
        while (1) {
            usb.SendBuf[1] = 'G';
            USBWrite(usb);
            USBRead(&usb);

            if (stop_flag == 1) {
                break;
            }

            tmp = usb.RecvBuf[1];
            if ((tmp & 0x80) == 0x80) {
                int x = tmp & 0x1;
#ifdef DUMP
                fprintf(fp, tmp);
#else
                printf("%d", x);
#endif
                if (k++ % 64 == 0) {
#ifdef DUMP
                    fprintf(fp, "\n");
#else
                    printf("\n");
#endif
                }
            }
        }
        usb.SendBuf[1] = 'S';
        USBWrite(usb);

        //ACK DATA SET
        unsigned char ack[] = {1, 1, 0, 1, 0, 0, 1, 0};
        for (j = 0; j < 8; ++j) {
            sprintf(buf, "A %d,%d", j, ack[j]);
            strcpy(&usb.SendBuf[1], buf);
            USBWrite(usb);
        }

        usb.SendBuf[1] = 'C';
        USBWrite(usb);

        USBDisConnect(usb);
#ifdef DUMP
        fclose(fp);
#endif
    }
    return 0;
}

void on_sigint(int p_sig) {
    stop_flag = 1;
}
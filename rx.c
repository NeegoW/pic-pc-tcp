//////////////////////////////////////////////////////////////////////////////////
// タイマー初期化
// 速度を変更する場合 Init x,y
// 第１引数 3: x256 , 2: x64 , 1: x8 , 0: x1
// 第２引数 65536 (2^16)以下にすること
// G M K 1 m μ n
// CPU 16M
// クロック　分周　　　　　　    カウント
// 1/16M x  64 =  4us      x 62500 =  250.0ms   [   4Hz]

// 1/16M x   8 =  0.5us    x 50000 =   25.0ms   [  40Hz]
// 1/16M x   8 =  0.5us    x  5000 =    2.5ms   [ 400Hz]

// 1/16M x   1 =  0.0625us x  4000 =    250us   [  4KHz]
// 1/16M x   1 =  0.0625us x   400 =     25us   [ 40KHz]
// 1/16M x   1 =  0.0625us x    40 =    2.5us   [400KHz]
// 1/16M x   1 =  0.0625us x     4 =   0.25us   [  4MHz]

typedef struct {
    char title[64];
    int tckps;
    int pr;
} PT;

//Ttx * 4 = Trx;
PT pt[] = {
        "1Hz", 2, 62500 - 1,    //0
        "10Hz", 1, 50000 - 1,   //1
        "100Hz", 1, 5000 - 1,   //2
        "1KHz", 0, 4000 - 1,    //3
        "10KHz", 0, 400 - 1,    //4
        "100KHz", 0, 40 - 1,    //5
};

#include <signal.h>
#include "log.c"
#include "dmc_code.c"
#include "USBconnect.c"
#include "prbs.c"

//#define DUMP

int stop_flag = 0;

void on_sigint(int p_sig);

int main(int argc, char *argv[]) {
    signal(SIGINT, on_sigint);

    int idx = 0;
    char distance[16] = "test";
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
    int j, k = 1;

    usb.VendorID = 0x4D8;
    usb.ProductID = 0x3F;

    bRes = USBConnect(&usb);

    if (bRes == FALSE) {
        printf("error:%s\n", usb.message);
    } else {
        printf("start\n");
        USBmemset(&usb);

#ifdef DUMP
        char fName[128] = ".\\rx_log\\";

        log_set_name(fName, pt[idx].title, distance);
        puts(fName);
        FILE *fp;
        fp = fopen(fName, "w");
#endif

        // Timer Init
        usb.SendBuf[0] = 0x00;
        sprintf(buf, "I %d,%d", pt[idx].tckps, pt[idx].pr);
        strcpy(&usb.SendBuf[1], buf);
        USBWrite(usb);

        // Run
        usb.SendBuf[1] = 'R';
        USBWrite(usb);
        int h;
        unsigned char tmp;

        // waiting
        while (1) {
            usb.SendBuf[1] = 'G';
            USBWrite(usb);
            USBRead(&usb);

            if (stop_flag == 1) {
                break;
            }

            h = usb.RecvBuf[1];
            if (h == 0x80) {
                tmp = usb.RecvBuf[2];
#ifdef DUMP
                fprintf(fp, "%d", tmp);
#else
                printf("%d", tmp);
#endif
                if (k++ % 100 == 0) {
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
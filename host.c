#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>

#define MAX_CLIENT_NUMS 9
#define DEF_PORT 10086

#pragma comment(lib, "ws2_32.lib")

typedef struct client_list_node {
    SOCKET socket_client; //客户端的socket
    struct sockaddr_in c_sin; //用于存储已连接的客户端的socket基本信息
    int is_run; //标记这个节点的socket是否正在被使用
    HANDLE h; //为这个socket创建的线程 的句柄
} client_list_node_st, *client_list_node_t;

//客户端列表 0->TX, 1->RX, 2->ACK
static client_list_node_st client_list[MAX_CLIENT_NUMS] = {0};
//服务端（本地）的socket
static SOCKET socket_of_server;
//用于存储本地创建socket的基本信息
static struct sockaddr_in s_sin;

//当前连接服务器的客户端的个数
static int client_nums = 0;

static void analysis(char *data, int datal, client_list_node_t node_t);

//声明线程函数
DWORD WINAPI myfun1(LPVOID lpParameter);

int main(int argc, char *argv[]) {
    char tx_addr[16];
    char rx_addr[16];

    switch (argc) {
        case 3:
            strcpy(rx_addr, argv[2]);
        case 2:
            strcpy(tx_addr, argv[1]);
        default:
            break;
    }

    WORD socket_version = MAKEWORD(2, 2);
    WSADATA wsadata;
    if (WSAStartup(socket_version, &wsadata) != 0) {
        return 0;
    }

    socket_of_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //创建socket 并判断是否创建成功
    if (socket_of_server == INVALID_SOCKET) {
        printf("socket error\n");
        return 0;
    }

    s_sin.sin_family = AF_INET;  //定义协议族为IPV4
    s_sin.sin_port = htons(DEF_PORT);//规定端口号
    s_sin.sin_addr.S_un.S_addr = INADDR_ANY;
    /************************************************************************
    s_sin.sin_addr.S_un.S_addr = INADDR_ANY; 是在设定绑定在本机的哪个IP地址上，
    在哪个IP地址上进行监听。设定为INADDR_ANY代表0.0.0.0就是默认IP。
    正常个人编程时 这个地方无关紧要。但若真正应用时这个地方最好设置清楚。
    因为好多服务器是多个网卡，本机有多个IP。哪个网卡是连接服务所在局域网的就要
    设置为哪个。
    ************************************************************************/
    if (bind(socket_of_server, (LPSOCKADDR) &s_sin, sizeof(s_sin)) == SOCKET_ERROR)//绑定
    {
        printf("bind error\n");
    }

    if (listen(socket_of_server, 5) == SOCKET_ERROR)//监听
    {
        printf("listen error\n");
        return 0;
    }

    printf("serve running on PORT: %d, waiting...\n", DEF_PORT);
    while (1) {
        SOCKET socket_of_client;  //客户端（远程）的socket
        struct sockaddr_in c_sin; //用于存储已连接的客户端的socket基本信息
        int c_sin_len;         //函数accept的第三个参数，c_sin的大小。

        c_sin_len = sizeof(c_sin);

        socket_of_client = accept(socket_of_server, (SOCKADDR *) &c_sin, &c_sin_len);
        /************************************************************************
        没有新的连接是 程序不会一直在这里循环。此时accept会处于阻塞状态。
        直到有新的连接，或者出现异常。
        ************************************************************************/
        if (socket_of_client == INVALID_SOCKET) {
            printf("accept error\n");
            continue; //继续等待下一次连接
        } else {
            if (client_nums + 1 > MAX_CLIENT_NUMS) {
                send(socket_of_client, "disconnected due to overloaded\n", strlen("disconnected due to overloaded\n"),
                     0);
                printf("new request from IP：%s%d, disconnected due to overloaded\n", inet_ntoa(c_sin.sin_addr),
                       c_sin.sin_port);
                Sleep(1000);
                closesocket(socket_of_client);
                continue;
            } else {
                int j;
                if (strcmp(inet_ntoa(c_sin.sin_addr), tx_addr) == 0) j = 0;
                else if (strcmp(inet_ntoa(c_sin.sin_addr), rx_addr) == 0) j = 1;
                else {
                    char *msg = "disconnected due to wrong addr\n";
                    send(socket_of_client, msg, strlen(msg), 0);
                    printf("new request from IP：%s%d,disconnected due to wrong addr\n",
                           inet_ntoa(c_sin.sin_addr),
                           c_sin.sin_port);
                    Sleep(1000);
                    closesocket(socket_of_client);
                    continue;
                }

                if (client_list[j].is_run == 0) {
                    client_list[j].is_run = 1;
                    client_list[j].socket_client = socket_of_client;
                    memcpy(&(client_list[j].c_sin), &c_sin, sizeof(c_sin));
                    if (client_list[j].h) {
                        CloseHandle(client_list[j].h);
                    }
                    client_list[j].h = (HANDLE) _beginthreadex(
                            NULL,
                            0,
                            (unsigned int (*)(void *)) myfun1,
                            &(client_list[j]),
                            0,
                            NULL);
                    client_nums++;
                }
            }
        }
    }

    closesocket(socket_of_server);
    WSACleanup();
    return 0;
}


static void analysis(char *data, int datal, client_list_node_t node_t) {
    printf("client: %s:%d, recv:%s, len:%d\n", inet_ntoa(node_t->c_sin.sin_addr), node_t->c_sin.sin_port, data, datal);
    //在这里我们可以对已接收到的数据进行处理

    //一般情况下这里都是处理“粘包”的地方

    //解决粘包之后 将完整的数据发送给数据处理函数
    //FIXME
    if (strcmp(data, "rx stop") == 0) {
        //1->rx
        if (client_list[1].is_run == 1)
            send(client_list[1].socket_client, data, datal, 0);
    } else {
        //1->rx
        if (client_list[1].is_run == 1)
            send(client_list[1].socket_client, data, datal, 0);
        //0->tx
        if (client_list[0].is_run == 1)
            send(client_list[0].socket_client, data, datal, 0);
    }
}


DWORD WINAPI myfun1(LPVOID lpParameter) {
    char revData[256];//这个地方一定要酌情设置大小，这决定了每次能获取多少数据
    int ret;//recv函数的返回值 有三种状态每种状态的含义在下方有解释
    client_list_node_t node = (client_list_node_t) lpParameter;

    printf("new connect IP: %s:%d\n", inet_ntoa(node->c_sin.sin_addr), node->c_sin.sin_port);

    printf("max connects: %d, has connected: %d\n", MAX_CLIENT_NUMS, client_nums);

//    send(node->socket_client, "connect success\n", strlen("connect success\n"), 0);
    while (1) {
        //接收来自 这个客户端的消息
        ret = recv(node->socket_client, revData, 255, 0);
        /************************************************************************
        recv函数 的实质就是从socket的缓冲区里拷贝出数据
        返回值就是拷贝出字节数的大小。
        上面定义的载体（revData）大小是255，所以recv的第三个参数最大只能设置为255，
        如果设置为大于255的数值，当执行recv函数时恰好缓冲区的内容大于255，
        就会导致内存泄漏，导致ret值小于零，解除阻塞状态。因此这里最好将第三个参数
        设置为revData的大小，那么当缓冲区内的数据小于255的时候
        只需要执行一次recv就可以将缓冲区的内容都拷贝出来，但当缓冲区的数据大
        于255的时候，就要执行多次recv函数。当缓冲区内没有内容的时候，会处于阻塞
        状态，这个while函数会停在这里。直到新的数据进来或者出现异常。
        ************************************************************************/

        if (ret > 0) {
            revData[ret] = 0x00;//正常情况下不必这么做，这么做只是为了能按字串的形式输出它
            analysis(revData, ret, node);
        } else if (ret == 0) {
            //当ret == 0 说明客户端已断开连接。这里直接跳出循环去等待下次连接即可。
            printf("client disconnected IP: %s:%d\n", inet_ntoa(node->c_sin.sin_addr), node->c_sin.sin_port);

            closesocket(node->socket_client);
            break;
        } else {
            //当ret < 0 说明出现了异常 例如阻塞状态解除，或者读取数据时出现指针错误等。
            //所以这里要主动断开和客户端的链接，然后跳出循环去等待新的连接。
            printf("connect error IP: %s:%d\n", inet_ntoa(node->c_sin.sin_addr), node->c_sin.sin_port);
            closesocket(node->socket_client);
            break;
        }
    }
    node->is_run = 0;
    client_nums--;
    printf("max connects: %d, had connected: %d\n", MAX_CLIENT_NUMS, client_nums);
    _endthreadex(0);
}
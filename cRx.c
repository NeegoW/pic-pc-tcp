#include <stdio.h>
#include <winsock2.h>
#include <conio.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

#define HOST_ADDR "192.168.58.1"
#define DEF_PORT 10086

static SOCKET socket_client;         //本地创建的客户端socket
static struct sockaddr_in server_in; //用于存储服务器的基本信息
STARTUPINFO si;
PROCESS_INFORMATION pi;

int process_is_run = 0;
int is_connect = 0;

static void analysis(char *data, int datal);

DWORD WINAPI rec_thread();

DWORD WINAPI send_thread();

int run_tx(char *param);

void send_sigint();

int main(int argc, char **argv) {
    WORD socket_version;
    WSADATA wsadata;
    socket_version = MAKEWORD(2, 2);
    if (WSAStartup(socket_version, &wsadata) != 0) {
        printf("WSAStartup error!");
        system("pause");
        return 0;
    }

    socket_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_client == INVALID_SOCKET) {
        printf("invalid socket !");
        system("pause");
        return 0;
    }

    server_in.sin_family = AF_INET;    //IPV4协议族
    server_in.sin_port = htons(DEF_PORT);  //服务器的端口号
    server_in.sin_addr.S_un.S_addr = inet_addr(HOST_ADDR); //服务IP
    if (connect(socket_client, (struct sockaddr *) &server_in, sizeof(server_in)) == SOCKET_ERROR) {
        printf("connect error\n");
        system("pause");
        return 0;
    }

    is_connect = 1;
    printf("connect %s:%d\n", inet_ntoa(server_in.sin_addr), server_in.sin_port);

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    memset(&pi, 0, sizeof(pi));

    _beginthreadex(
            NULL,
            0,
            (unsigned int (*)(void *)) rec_thread,
            NULL,
            0,
            NULL);

    int key = _getch();
    while (key != 27);

    closesocket(socket_client);
    WSACleanup();
    return 0;
}

static void analysis(char *data, int datal) {
    printf("recv data:%s\ndatal:%d\n", data, datal);

    if (strcmp(data, "stop") == 0) {
        send_sigint();
    } else {
        run_tx(data);
    }
}

DWORD WINAPI rec_thread() {
    char recData[256];
    int ret;

    while (1) {
        ret = recv(socket_client, recData, 255, 0);
        if (ret > 0) {
            recData[ret] = 0x00;
            analysis(recData, ret);
        } else if (ret == 0) {
            //当ret == 0 说明服务器掉线。
            is_connect = 0;
            printf("lost connection , Ip = %s\n", inet_ntoa(server_in.sin_addr));
            break;
        } else {
            //当ret < 0 说明出现了异常 例如阻塞状态解除，或者读取数据时出现指针错误等。
            //所以要主动断开和客户端的链接。
            is_connect = 0;
            printf("something wrong of %s\n", inet_ntoa(server_in.sin_addr));
            closesocket(socket_client);
            break;
        }
    }

    _endthreadex(0);
}

int run_tx(char *param) {
    char cmdLine[256] = "c3.exe ";
    strcat(cmdLine, param);

    // Start the child process.
    if (!CreateProcess(NULL,   // No module name (use command line)
                       cmdLine,        // Command line
                       NULL,           // Process handle not inheritable
                       NULL,           // Thread handle not inheritable
                       FALSE,          // Set handle inheritance to FALSE
                       0,              // No creation flags
                       NULL,           // Use parent's environment block
                       NULL,           // Use parent's starting directory
                       &si,            // Pointer to STARTUPINFO structure
                       &pi)           // Pointer to PROCESS_INFORMATION structure
            ) {
        printf("CreateProcess failed (%lu).\n", GetLastError());
        return -1;
    }
    CloseHandle(pi.hThread);

    process_is_run = 1;
    puts("process running...");

    return 0;
}

void send_sigint() {
    AttachConsole(pi.dwProcessId);
    SetConsoleCtrlHandler(NULL, TRUE);

    if (GenerateConsoleCtrlEvent(CTRL_C_EVENT, pi.dwProcessId)) {
        memset(&si, 0, sizeof(STARTUPINFO));
        si.cb = sizeof(si);
        memset(&pi, 0, sizeof(PROCESS_INFORMATION));
    }

    Sleep(200);
    SetConsoleCtrlHandler(NULL, FALSE);

    process_is_run = 0;
}
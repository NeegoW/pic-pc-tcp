#include <windows.h>
#include <stdio.h>
#include <conio.h>

STARTUPINFO t_si;
PROCESS_INFORMATION t_pi;
int is_run = 0;

void KbdFunc(void);

int runC3(void);

void send_sigint();

int main(int argc, char *argv[]) {
    memset(&t_si, 0, sizeof(STARTUPINFO));
    memset(&t_pi, 0, sizeof(PROCESS_INFORMATION));
    KbdFunc();

//    system("pause");
}

void KbdFunc() {
    int KeyInfo;

    do {
        KeyInfo = _getch();
        if (KeyInfo == '1') {
            printf("is_run:%d\n", is_run);
            if (!is_run) {
                int rs;
                rs = runC3();
                if (rs != 0) perror("Error from spawnv");
                else {
                    is_run = 1;
                    puts("c3 running");
                }
            } else {
                puts("plz wait");
            }
        }
        if (KeyInfo == '2') {
            send_sigint();
        }
    } while (KeyInfo != 27);

    send_sigint();
}

void send_sigint() {
    AttachConsole(t_pi.dwProcessId);
    SetConsoleCtrlHandler(NULL, TRUE);

    if (GenerateConsoleCtrlEvent(CTRL_C_EVENT, t_pi.dwProcessId)) {
//        FreeConsole();
//        printf("father sends ctrl+c to child:%lu\n", t_pi.dwProcessId);
        memset(&t_si, 0, sizeof(STARTUPINFO));
        memset(&t_pi, 0, sizeof(PROCESS_INFORMATION));
    }

    Sleep(200);
    SetConsoleCtrlHandler(NULL, FALSE);

    is_run = 0;
}


int runC3() {
    t_si.cb = sizeof(STARTUPINFO);
    t_si.dwFlags = STARTF_USESHOWWINDOW;
    t_si.wShowWindow = SW_SHOW;
    CreateProcess(NULL, "c3", NULL, NULL, FALSE, 0, NULL, NULL, &t_si, &t_pi);
    CloseHandle(t_pi.hThread);

    printf("create son,pid:%lu\n", t_pi.dwProcessId);
    return 0;
}
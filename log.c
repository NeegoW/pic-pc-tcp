#include <time.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <windows.h>

int log_set_name(char *fName, char *type, char *freq);

void log_w(char *fName, unsigned char *arr, long size);

int log_set_name(char *fName, char *type, char *freq) {
    char parentDir[128] = "";

    // initial parentDir
    strcat(parentDir, fName); // cat baseDir
    if (access(parentDir, 0) == -1) mkdir(parentDir);
    sprintf(parentDir, "%s%s\\", parentDir, freq);// cat frequency
    if (access(parentDir, 0) == -1) mkdir(parentDir);
    time_t timep;
    struct tm *pTm;
    time(&timep);
    pTm = gmtime(&timep);
    sprintf(parentDir, "%s%d%02d%02d\\", parentDir, 1900 + pTm->tm_year, 1 + pTm->tm_mon, pTm->tm_mday);// cat nowDate
    if (access(parentDir, 0) == -1) mkdir(parentDir);
    sprintf(parentDir, "%s%s\\", parentDir, type);// cat type
    if (access(parentDir, 0) == -1) mkdir(parentDir);

    // find the last id of the fileName
    char search[128];
    const char fSuffix[5] = ".log";
    unsigned int id;
    const char delim[2] = ".";
    WIN32_FIND_DATA pFind;
    sprintf(search, "%s*%s", parentDir, fSuffix); // find *.log in parentDir
    HANDLE h = FindFirstFile(search, &pFind);
    if (h == INVALID_HANDLE_VALUE) {
        id = 1; // not file yet ,id = 1
    } else {
        do {
            id = atoi(strtok(pFind.cFileName, delim));
        } while (FindNextFile(h, &pFind));
        id++;
    }

    sprintf(fName, "%s%08d%s", parentDir, id, fSuffix);
    return 0;
}

void log_w(char *fName, unsigned char *arr, long size) {
    FILE *fp;
    int flag = 1;
    fp = fopen(fName, "w");
    if (fp == NULL) {
        flag = 0;
        printf("file can not open!\n\r");
        return;
    }
    for (int i = 0; i < size; i++) {
        if (fprintf(fp, "%d", arr[i]) != 1) {
            flag = 0;
            printf("fail to write!\n\r");
            break;
        }
    }
    fclose(fp);
    if (flag == 1) printf("success!\n\r");
}
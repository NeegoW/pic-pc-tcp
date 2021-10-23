void bin_file_w(char *);

void bin_file_r();

void bin_file_w(char *arr) {
    FILE *fp;
    fp = fopen("myFile.bin", "wb");
    if (fp == NULL) {
        printf("file can not open!\n");
        return;
    }
    for (int i = 0; i < 127; i++) {
        if (fwrite(&arr[i], sizeof(char), 1, fp) != 1) {
            printf("file write error");
        }
    }
    fclose(fp);
    printf(" \nWrite into file: done!\n");
}

void bin_file_r() {
    FILE *fp;
    char arr[127];
    fp = fopen("./myFile.bin", "r");
    fread(arr, sizeof(char), sizeof(arr), fp);
    fclose(fp);
    for (int i = 0; i < 127; i++) {
        if (i % 16 == 0) printf("\n");
        printf("%d", arr[i]);
    }
}
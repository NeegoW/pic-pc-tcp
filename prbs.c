int prbs7(unsigned char *, unsigned char *);

/* random sequence generation
 * @arr : initial value
 * @m : random sequence
 * */
int prbs7(unsigned char init[], unsigned char m[]) {
    unsigned char tmp;
    unsigned char arr[7];
    //clone the init
    for (int i = 0; i < 7; i++) {
        arr[i] = init[i];
    }
    //f(x) = x^7 + x^6 +1
    //tmp = c7 ^ c6 , c1 = tmp, initial value >>
    //output = c7
    for (int i = 0; i < 128; i++) {
        tmp = arr[5] ^ arr[6];
        arr[6] = arr[5];
        arr[5] = arr[4];
        arr[4] = arr[3];
        arr[3] = arr[2];
        arr[2] = arr[1];
        arr[1] = arr[0];
        arr[0] = tmp;
        m[i] = arr[6];
    }

//    for (int i = 0; i < 128; i++) {
//        printf("%d", m[i]);
//    }
    return 0;
}
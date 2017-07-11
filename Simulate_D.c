#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include"Utils.h"
#include"Simulate_D.h"

/* Map the RAM memory locations to 4 byte addresses in the RTU's memory space*/
unsigned long RamAddressMappingD(int arrayId) {

    long d0, d1, d2, d3;
    char buffer[9];
    char hex[10];
    unsigned long temp;
    memset(buffer, '\0', sizeof (buffer));

    /* Make sure to take care of the endianess of the platform*/
    d0 = final_hexD[arrayId + 0];
    d1 = final_hexD[arrayId + 1];
    d2 = final_hexD[arrayId + 2];
    d3 = final_hexD[arrayId + 3];

    sprintf(&hex, "%02x", d0);
    strupr(hex);
    strncpy(buffer, hex, 2);

    sprintf(&hex, "%02x", d1);
    strupr(hex);
    strncpy(buffer + 2, hex, 2);

    sprintf(&hex, "%02x", d2);
    strupr(hex);
    strncpy(buffer + 4, hex, 2);

    sprintf(&hex, "%02x", d3);
    strupr(hex);
    strncpy(buffer + 6, hex, 2);


    temp = hex2int(buffer, 8);
    //printf(" Index[%d] = %s\n", arrayId, strupr(buffer));
    //printf(" Value of r0 = %lu \n", temp);
    return temp;
}
void computeChecksumD(long long r1) {

    char a0[3] = "";
    char a1[3] = "";
    char a2[3] = "";
    char a3[3] = "";
    bzero(stringD, 10);
    int j;
    j = 0;
    sprintf(stringD, "%lX", r1);
    if (strlen(stringD) < 8) {
        //pad = 8-strlen(string);
        strcat(zeroD, stringD);
        bzero(stringD, 10);
        strcpy(stringD, zeroD);
    }
    strncpy(a0, stringD + j, 2);
    //a0[3] = '\0';
    j = j + 2;
    strncpy(a1, stringD + j, 2);
    //a1[3] = '\0';
    j = j + 2;
    strncpy(a2, stringD + j, 2);
   // a2[3] = '\0';
    j = j + 2;
    strncpy(a3, stringD + j, 2);
   // a3[3] = '\0';
    j = j + 2;



    strncat(checksumD, a3, 2);
    strncat(checksumD, a2, 2);
    strncat(checksumD, a1, 2);
    strncat(checksumD, a0, 2);


}
void SimulatorIOBoard16KB_D() {
    
    
    long d0, d1, d2, d3;
    unsigned char RAM[32640];

    char line[80];
    FILE *fp;
    int i, j, k, c;

    /* Read the file which contains the parsed memory dump*/
    fp = fopen("ram_log_io_D", "rt");
    i = 0;
    int store_index[30720*2];
    c = 0;
    long sum;
    char c0, c1, c2, c3;

    char b0[2] = "";
    char b1[2] = "";
    char b2[2] = "";
    char b3[2] = "";


    unsigned long r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0, r11 = 0, r12 = 0, r13 = 0, r14 = 0, r15 = 0;
    unsigned long mrs = 0;
    long loop;
    unsigned long tempRes;
    long temp = 0;
    int arrayId = 0;
    unsigned long memVal;

    char *b;
    memset(final_hexD, 0, sizeof(final_hexD));

    while (fgets(line, sizeof (line), fp)) {

        j = 0;
        if (strlen(line) > 2) {
            while (j < 32) {

                strncpy(b0, line + j, 2);
                j = j + 2;
                strncpy(b1, line + j, 2);
                j = j + 2;
                strncpy(b2, line + j, 2);
                j = j + 2;
                strncpy(b3, line + j, 2);
                j = j + 2;

                d0 = hex2int(b3, 2);
                d1 = hex2int(b2, 2);
                d2 = hex2int(b1, 2);
                d3 = hex2int(b0, 2);

                final_hexD[i++] = d0;
                final_hexD[i++] = d1;
                final_hexD[i++] = d2;
                final_hexD[i++] = d3;

            }
        }
    }
    
     /*
    printf("\n Total = %d ", i);
    for (i = 0; i < (RAM_SIZE); i++) {
        if (i % 25 == 0)
            printf("\n");
        printf(" Index [%d] = %d ", i, final_hexD[i]);
    }*/



    /* Attest logic */
    /* IMP : Staring random seed must be set here */
    r0 = 0x8;
    /* IMP : Starting values of the status register must be set here*/
    mrs = 0x60000029;
    for (r13 = 0x7800; r13 > 0; r13 = r13 - 0x8) {


        if (r13 == 0x0) goto end;

        /*   IMP: Starting PC value must be known and correctly set here
             PC needs to be reset after every loop since we go back to the same instruction
         */
        r15 = 0x10000068;

        // Update register r1 with the checksum piece
        r14 = r0 * r0;
        r14 = r14 | 0x5;
        r0 = r0 + r14;

        // 0-1 KB walk address
        r14 = r0 & 0x3FC;
        r14 = r14 + 0x10000000;
        r1 = r1 ^ r14;
        arrayId = r14 - 0x10000000;

        store_index[c++] = arrayId;
        if (arrayId < 0 || arrayId > RAM_SIZE) {
            printf("Invalid memory index %d... Exiting \n ",arrayId);
            printf("Loop no %02x",r13);
            exit(0);
        }
        memVal = RamAddressMappingD(arrayId);
        r14 = memVal;
        r0 = r0 ^ r14;
        r14 = mrs;

        //  r1 = r1+r0^r14+r15^r12+r15^r11+r13^r10+r9^r8+r7^r6+r5^r4+r3^r2
        tempRes = r1 + r0;
        mrs = ADDwithFlagUpdate(r1, r0, mrs);
        r1 = tempRes;
        r1 = r1 ^ r14;
        /* 
         * First PC access is 11 (B in hex) instructions after the first T-function instruction
         * Hence PC = PC + (4 * B) = PC + 2C = 0x40000054 + 0x2C = 0x40000080
         * Also the PC is an additional 8 bytes ahead on a ARM platform
         */
        r15 = r15 + 0x2C /*- 0x4 */;
        tempRes = r1 + r15;
        //mrs = ADDwithFlagUpdate(r1, r15, mrs);
        r1 = tempRes;
        r1 = r1 ^ r12;
        /* Two more instructions before accessing the PC again*/
        r15 = r15 + 0x6;
        tempRes = r1 + r15;
        //mrs = ADDwithFlagUpdate(r1, r15, mrs);
        r1 = tempRes;
        r1 = r1 ^ r11;
        tempRes = r1 + r13;
        //mrs = ADDwithFlagUpdate(r1, r13, mrs);
        r1 = tempRes;
        r1 = r1 ^ r10;
        tempRes = r1 + r9;
        mrs = ADDwithFlagUpdate(r1, r9, mrs);
        r1 = tempRes;
        r1 = r1 ^ r8;
        tempRes = r1 + r7;
        mrs = ADDwithFlagUpdate(r1, r7, mrs);
        r1 = tempRes;
        r1 = r1 ^ r6;
        tempRes = r1 + r5;
        mrs = ADDwithFlagUpdate(r1, r5, mrs);
        r1 = tempRes;
        r1 = r1 ^ r4;
        tempRes = r1 + r3;
        mrs = ADDwithFlagUpdate(r1, r3, mrs);
        r1 = tempRes;
        r1 = r1 ^ r2;
        temp = _rotr(r1, 1);
        r1 = temp;
        r0 = r0 ^ r1;

        // Update register r2 with checksum piece
        r14 = r0 * r0;
        r14 = r14 | 0x5;
        r0 = r0 + r14;

        //  16KB walk address from RAM
        r14 = r0 & 0x07F00;
        r14 = r14 + 0x10000000;
        r2 = r2 ^ r14;
        arrayId = r14 - 0x10000000;

        store_index[c++] = arrayId;
        if (arrayId < 0 || arrayId > RAM_SIZE) {
          printf("Invalid memory index %d... Exiting \n ",arrayId);
            printf("Loop no %02x",r13);
            exit(0);
        }
        memVal = RamAddressMappingD(arrayId);
        r14 = memVal;
        r0 = r0 ^ r14;
        r14 = mrs;

        // r2 = r2+r0^r14+r15^r12+r15^r11+r13^r10+r9^r8+r7^r6+r5^r4+r3^r1
        tempRes = r2 + r0;
        mrs = ADDwithFlagUpdate(r2, r0, mrs);
        r2 = tempRes;
        r2 = r2 ^ r14;
        /* 25 instructions since the last PC access */
        r15 = r15 + 0x56 /*- 0x52 */;
        tempRes = r2 + r15;
        //mrs = ADDwithFlagUpdate(r2, r15, mrs);
        r2 = tempRes;
        r2 = r2 ^ r12;
        /* 2 more instructions since the last PC access */
        r15 = r15 + 0x6;
        tempRes = r2 + r15;
        //mrs = ADDwithFlagUpdate(r2, r15, mrs);
        r2 = tempRes;
        r2 = r2 ^ r11;
        tempRes = r2 + r13;
        //mrs = ADDwithFlagUpdate(r2, r13, mrs);
        r2 = tempRes;
        r2 = r2 ^ r10;
        tempRes = r2 + r9;
        mrs = ADDwithFlagUpdate(r2, r9, mrs);
        r2 = tempRes;
        r2 = r2 ^ r8;
        tempRes = r2 + r7;
        mrs = ADDwithFlagUpdate(r2, r7, mrs);
        r2 = tempRes;
        r2 = r2 ^ r6;
        tempRes = r2 + r5;
        mrs = ADDwithFlagUpdate(r2, r5, mrs);
        r2 = tempRes;
        r2 = r2 ^ r4;
        tempRes = r2 + r3;
        mrs = ADDwithFlagUpdate(r2, r3, mrs);
        r2 = tempRes;
        r2 = r2 ^ r1;

        temp = _rotr(r2, 1);
        r2 = temp;
        r0 = r0 ^ r2;

        // Update register r3 with checksum piece	
        r14 = r0 * r0;
        r14 = r14 | 0x5;
        r0 = r0 + r14;

        // ; 1-2 KB walk address 
        r14 = r0 & 0x3FC;
        r14 = r14 + 0x10000000;
        r3 = r3 ^ r14;
        arrayId = r14 - 0x10000000 + 0x400;

        store_index[c++] = arrayId;

        if (arrayId < 0 || arrayId > RAM_SIZE) {
          printf("Invalid memory index %d... Exiting \n ",arrayId);
            printf("Loop no %02x",r13);
            exit(0);
        }
        memVal = RamAddressMappingD(arrayId);
        r14 = memVal;
        r0 = r0 ^ r14;
        r14 = mrs;

        // r3 = r3+r0^r14+r15^r12+r15^r11+r13^r10+r9^r8+r7^r6+r5^r4+r2^r1
        tempRes = r3 + r0;
        mrs = ADDwithFlagUpdate(r3, r0, mrs);
        r3 = tempRes;
        r3 = r3 ^ r14;
        /* 25 instructions since the last PC access */
        r15 = r15 + 0x56 /*- 0x4 */;
        tempRes = r3 + r15;
        //mrs = ADDwithFlagUpdate(r3, r15, mrs);
        r3 = tempRes;
        r3 = r3 ^ r12;
        /* 2 more instructions since the last PC access */
        r15 = r15 + 0x6;
        tempRes = r3 + r15;
        //mrs = ADDwithFlagUpdate(r3, r15, mrs);
        r3 = tempRes;
        r3 = r3 ^ r11;
        tempRes = r3 + r13;
        //mrs = ADDwithFlagUpdate(r3, r13, mrs);
        r3 = tempRes;
        r3 = r3 ^ r10;
        tempRes = r3 + r9;
        mrs = ADDwithFlagUpdate(r3, r9, mrs);
        r3 = tempRes;
        r3 = r3 ^ r8;
        tempRes = r3 + r7;
        mrs = ADDwithFlagUpdate(r3, r7, mrs);
        r3 = tempRes;
        r3 = r3 ^ r6;
        tempRes = r3 + r5;
        mrs = ADDwithFlagUpdate(r3, r5, mrs);
        r3 = tempRes;
        r3 = r3 ^ r4;
        tempRes = r3 + r2;
        mrs = ADDwithFlagUpdate(r3, r2, mrs);
        r3 = tempRes;
        r3 = r3 ^ r1;

        temp = _rotr(r3, 1);
        r3 = temp;
        r0 = r0 ^ r3;

        // ; Update register r4 with checksum piece
        r14 = r0 * r0;
        r14 = r14 | 0x5;
        r0 = r0 + r14;

        // ; 16KB walk address from RAM
        r14 = r0 & 0x07F00;
        r14 = r14 + 0x10000000;
        r4 = r4 ^ r14;
        arrayId = r14 - 0x10000000;

        store_index[c++] = arrayId;
        if (arrayId < 0 || arrayId > RAM_SIZE) {
           printf("Invalid memory index %d... Exiting \n ",arrayId);
            printf("Loop no %02x",r13);
            exit(0);
        }
        memVal = RamAddressMappingD(arrayId);
        r14 = memVal;
        r0 = r0 ^ r14;
        r14 = mrs;

        // r4 = r4+r0^r14+r15^r12+r15^r11+r13^r10+r9^r8+r7^r6+r5^r3+r2^r1
        tempRes = r4 + r0;
        mrs = ADDwithFlagUpdate(r4, r0, mrs);
        r4 = tempRes;
        r4 = r4 ^ r14;
        /* 25 instructions since the last PC access */
        r15 = r15 + 0x56/*- 0x4 */;
        tempRes = r4 + r15;
        //mrs = ADDwithFlagUpdate(r4, r15, mrs);
        r4 = tempRes;
        r4 = r4 ^ r12;
        /* 2 more instructions since the last PC access */
        r15 = r15 + 0x6;
        tempRes = r4 + r15;
        //mrs = ADDwithFlagUpdate(r4, r15, mrs);
        r4 = tempRes;
        r4 = r4 ^ r11;
        tempRes = r4 + r13;
        //mrs = ADDwithFlagUpdate(r4, r13, mrs);
        r4 = tempRes;
        r4 = r4 ^ r10;
        tempRes = r4 + r9;
        mrs = ADDwithFlagUpdate(r4, r9, mrs);
        r4 = tempRes;
        r4 = r4 ^ r8;
        tempRes = r4 + r7;
        mrs = ADDwithFlagUpdate(r4, r7, mrs);
        r4 = tempRes;
        r4 = r4 ^ r6;
        tempRes = r4 + r5;
        mrs = ADDwithFlagUpdate(r4, r5, mrs);
        r4 = tempRes;
        r4 = r4 ^ r3;
        tempRes = r4 + r2;
        mrs = ADDwithFlagUpdate(r4, r2, mrs);
        r4 = tempRes;
        r4 = r4 ^ r1;

        temp = _rotr(r4, 1);
        r4 = temp;
        r0 = r0 ^ r4;


        // Update register r5 with checksum piece
        r14 = r0 * r0;
        r14 = r14 | 0x5;
        r0 = r0 + r14;

        // 0-1 KB walk address
        r14 = r0 & 0x3FC;
        r14 = r14 + 0x10000000;
        r5 = r5 ^ r14;
        arrayId = r14 - 0x10000000;

        store_index[c++] = arrayId;
        if (arrayId < 0 || arrayId > RAM_SIZE) {
           printf("Invalid memory index %d... Exiting \n ",arrayId);
            printf("Loop no %02x",r13);
            exit(0);
        }
        memVal = RamAddressMappingD(arrayId);
        r14 = memVal;
        r0 = r0 ^ r14;
        r14 = mrs;

        // r5 = r5+r0^r14+r15^r12+r15^r11+r13^r10+r9^r8+r7^r6+r4^r3+r2^r1 
        tempRes = r5 + r0;
        mrs = ADDwithFlagUpdate(r5, r0, mrs);
        r5 = tempRes;
        r5 = r5 ^ r14;
        /* 25 instructions since the last PC access */
        r15 = r15 + 0x56/*- 0x4 */;
        tempRes = r5 + r15;
        //mrs = ADDwithFlagUpdate(r5, r15, mrs);
        r5 = tempRes;
        r5 = r5 ^ r12;
        /* 2 more instructions since the last PC access */
        r15 = r15 + 0x6;
        tempRes = r5 + r15;
        //mrs = ADDwithFlagUpdate(r5, r15, mrs);
        r5 = tempRes;
        r5 = r5 ^ r11;
        tempRes = r5 + r13;
        //mrs = ADDwithFlagUpdate(r5, r13, mrs);
        r5 = tempRes;
        r5 = r5 ^ r10;
        tempRes = r5 + r9;
        mrs = ADDwithFlagUpdate(r5, r9, mrs);
        r5 = tempRes;
        r5 = r5 ^ r8;
        tempRes = r5 + r7;
        mrs = ADDwithFlagUpdate(r5, r7, mrs);
        r5 = tempRes;
        r5 = r5 ^ r6;
        tempRes = r5 + r4;
        mrs = ADDwithFlagUpdate(r5, r4, mrs);
        r5 = tempRes;
        r5 = r5 ^ r3;
        tempRes = r5 + r2;
        mrs = ADDwithFlagUpdate(r5, r2, mrs);
        r5 = tempRes;
        r5 = r5 ^ r1;

        temp = _rotr(r5, 1);
        r5 = temp;
        r0 = r0 ^ r5;

        //  Update register r6 with checksum piece
        r14 = r0 * r0;
        r14 = r14 | 0x5;
        r0 = r0 + r14;

        // 16KB walk address from RAM
        r14 = r0 & 0x07F00;
        r14 = r14 + 0x10000000;
        r6 = r6 ^ r14;
        arrayId = r14 - 0x10000000;

        store_index[c++] = arrayId;
        if (arrayId < 0 || arrayId > RAM_SIZE) {
           printf("Invalid memory index %d... Exiting \n ",arrayId);
            printf("Loop no %02x",r13);
            exit(0);
        }
        memVal = RamAddressMappingD(arrayId);
        r14 = memVal;
        r0 = r0 ^ r14;
        r14 = mrs;

        //  r6 = r6+r0^r14+r15^r12+r15^r11+r13^r10+r9^r8+r7^r5+r4^r3+r2^r1
        tempRes = r6 + r0;
        mrs = ADDwithFlagUpdate(r6, r0, mrs);
        r6 = tempRes;
        r6 = r6 ^ r14;
        /* 25 instructions since the last PC access */
        r15 = r15 + 0x56/*- 0x4 */;
        tempRes = r6 + r15;
        //mrs = ADDwithFlagUpdate(r6, r15, mrs);
        r6 = tempRes;
        r6 = r6 ^ r12;
        /* 2 more instructions since the last PC access */
        r15 = r15 + 0x6;
        tempRes = r6 + r15;
        //mrs = ADDwithFlagUpdate(r6, r15, mrs);
        r6 = tempRes;
        r6 = r6 ^ r11;
        tempRes = r6 + r13;
        //mrs = ADDwithFlagUpdate(r6, r13, mrs);
        r6 = tempRes;
        r6 = r6 ^ r10;
        tempRes = r6 + r9;
        mrs = ADDwithFlagUpdate(r6, r9, mrs);
        r6 = tempRes;
        r6 = r6 ^ r8;
        tempRes = r6 + r7;
        mrs = ADDwithFlagUpdate(r6, r7, mrs);
        r6 = tempRes;
        r6 = r6 ^ r5;
        tempRes = r6 + r4;
        mrs = ADDwithFlagUpdate(r6, r4, mrs);
        r6 = tempRes;
        r6 = r6 ^ r3;
        tempRes = r6 + r2;
        mrs = ADDwithFlagUpdate(r6, r2, mrs);
        r6 = tempRes;
        r6 = r6 ^ r1;

        temp = _rotr(r6, 1);
        r6 = temp;
        r0 = r0 ^ r6;

        // Update register r7 with checksum piece
        r14 = r0 * r0;
        r14 = r14 | 0x5;
        r0 = r0 + r14;


        // 1-2 KB walk address
        r14 = r0 & 0x3FC;
        r14 = r14 + 0x10000000;
        r7 = r7 ^ r14;
        arrayId = r14 - 0x10000000 + 0x400;

        store_index[c++] = arrayId;

        if (arrayId < 0 || arrayId > RAM_SIZE) {
           printf("Invalid memory index %d... Exiting \n ",arrayId);
            printf("Loop no %02x",r13);
            exit(0);
        }
        memVal = RamAddressMappingD(arrayId);
        r14 = memVal;
        r0 = r0 ^ r14;
        r14 = mrs;

        // r7 = r7+r0^r14+r15^r12+r15^r11+r13^r10+r9^r8+r6^r5+r4^r3+r2^r1
        tempRes = r7 + r0;
        mrs = ADDwithFlagUpdate(r7, r0, mrs);
        r7 = tempRes;
        r7 = r7 ^ r14;
        /* 25 instructions since the last PC access */
        r15 = r15 + 0x56/*- 0x4 */;
        tempRes = r7 + r15;
        //mrs = ADDwithFlagUpdate(r7, r15, mrs);
        r7 = tempRes;
        r7 = r7 ^ r12;
        /* 2 more instructions since the last PC access */
        r15 = r15 + 0x6;
        tempRes = r7 + r15;
        //mrs = ADDwithFlagUpdate(r7, r15, mrs);
        r7 = tempRes;
        r7 = r7 ^ r11;
        tempRes = r7 + r13;
        //mrs = ADDwithFlagUpdate(r7, r13, mrs);
        r7 = tempRes;
        r7 = r7 ^ r10;
        tempRes = r7 + r9;
        mrs = ADDwithFlagUpdate(r7, r9, mrs);
        r7 = tempRes;
        r7 = r7 ^ r8;
        tempRes = r7 + r6;
        mrs = ADDwithFlagUpdate(r7, r6, mrs);
        r7 = tempRes;
        r7 = r7 ^ r5;
        tempRes = r7 + r4;
        mrs = ADDwithFlagUpdate(r7, r4, mrs);
        r7 = tempRes;
        r7 = r7 ^ r3;
        tempRes = r7 + r2;
        mrs = ADDwithFlagUpdate(r7, r2, mrs);
        r7 = tempRes;
        r7 = r7 ^ r1;

        temp = _rotr(r7, 1);
        r7 = temp;
        r0 = r0 ^ r7;

        // Update register r8 with checksum piece 
        r14 = r0 * r0;
        r14 = r14 | 0x5;
        r0 = r0 + r14;

        // 16KB walk address from RAM
        r14 = r0 & 0x07F00;
        r14 = r14 + 0x10000000;
        r8 = r8 ^ r14;
        arrayId = r14 - 0x10000000;
        //printf("ArrayIndex %d \n ",arrayId);
        store_index[c++] = arrayId;

        if (arrayId < 0 || arrayId > RAM_SIZE) {
           printf("Invalid memory index %d... Exiting \n ",arrayId);
            printf("Loop no %02x",r13);
            exit(0);
        }
        memVal = RamAddressMappingD(arrayId);
        r14 = memVal;
        r0 = r0 ^ r14;
        r14 = mrs;

        // ;  r8 = r8+r0^r14+r15^r12+r15^r11+r13^r10+r9^r7+r6^r5+r4^r3+r2^r1
        tempRes = r8 + r0;
        mrs = ADDwithFlagUpdate(r8, r0, mrs);
        r8 = tempRes;
        r8 = r8 ^ r14;
        /* 25 instructions since the last PC access */
        r15 = r15 + 0x56 + 0x2/*- 0x4 */;
        tempRes = r8 + r15;
        //mrs = ADDwithFlagUpdate(r8, r15, mrs);
        r8 = tempRes;
        r8 = r8 ^ r12;
        /* 2 more instructions since the last PC access */
        r15 = r15 + 0x6;
        tempRes = r8 + r15;
        //mrs = ADDwithFlagUpdate(r8, r15, mrs);
        r8 = tempRes;
        r8 = r8 ^ r11;
        tempRes = r8 + r13;
        //mrs = ADDwithFlagUpdate(r8, r13, mrs);
        r8 = tempRes;
        r8 = r8 ^ r10;
        tempRes = r8 + r9;
        mrs = ADDwithFlagUpdate(r8, r9, mrs);
        r8 = tempRes;
        r8 = r8 ^ r7;
        tempRes = r8 + r6;
        mrs = ADDwithFlagUpdate(r8, r6, mrs);
        r8 = tempRes;
        r8 = r8 ^ r5;
        tempRes = r8 + r4;
        mrs = ADDwithFlagUpdate(r8, r4, mrs);
        r8 = tempRes;
        r8 = r8 ^ r3;
        tempRes = r8 + r2;
        mrs = ADDwithFlagUpdate(r8, r2, mrs);
        r8 = tempRes;
        r8 = r8 ^ r1;

        temp = _rotr(r8, 1);
        r8 = temp;
        r0 = r0 ^ r8;

        // Update register r9 with checksum piece	
        r14 = r0 * r0;
        r14 = r14 | 0x5;
        r0 = r0 + r14;

        // 0-1 KB walk addresss
        r14 = r0 & 0x07F00;
        r14 = r14 + 0x10000000;
        r9 = r9 ^ r14;
        arrayId = r14 - 0x10000000;

        store_index[c++] = arrayId;

        if (arrayId < 0 || arrayId > RAM_SIZE) {
          printf("Invalid memory index %d... Exiting \n ",arrayId);
            printf("Loop no %02x",r13);
            exit(0);
        }
        memVal = RamAddressMappingD(arrayId);
        r14 = memVal;
        r0 = r0 ^ r14;
        r14 = mrs;

        // r9 = r9+r0^r14+r15^r12+r15^r11+r13^r10+r8^r7+r6^r5+r4^r3+r2^r1
        tempRes = r9 + r0;
        mrs = ADDwithFlagUpdate(r9, r0, mrs);
        r9 = tempRes;
        r9 = r9 ^ r14;
        /* 25 instructions since the last PC access */
        r15 = r15 + 0x56 + 0x8/*- 0x4 */;
        tempRes = r9 + r15;
        //mrs = ADDwithFlagUpdate(r9, r15, mrs);
        r9 = tempRes;
        r9 = r9 ^ r12;
        /* 2 more instructions since the last PC access */
        r15 = r15 + 0x6;
        tempRes = r9 + r15;
        //mrs = ADDwithFlagUpdate(r9, r15, mrs);
        r9 = tempRes;
        r9 = r9 ^ r11;
        tempRes = r9 + r13;
        //mrs = ADDwithFlagUpdate(r9, r13, mrs);
        r9 = tempRes;
        r9 = r9 ^ r10;
        tempRes = r9 + r8;
        mrs = ADDwithFlagUpdate(r9, r8, mrs);
        r9 = tempRes;
        r9 = r9 ^ r7;
        tempRes = r9 + r6;
        mrs = ADDwithFlagUpdate(r9, r6, mrs);
        r9 = tempRes;
        r9 = r9 ^ r5;
        tempRes = r9 + r4;
        mrs = ADDwithFlagUpdate(r9, r4, mrs);
        r9 = tempRes;
        r9 = r9 ^ r3;
        tempRes = r9 + r2;
        mrs = ADDwithFlagUpdate(r9, r2, mrs);
        r9 = tempRes;
        r9 = r9 ^ r1;

        temp = _rotr(r9, 1);
        r9 = temp;
        r0 = r0 ^ r9;

        // Update register r10 with checksum piece
        r14 = r0 * r0;
        r14 = r14 | 0x5;
        r0 = r0 + r14;

        // 16KB walk address from RAM
        r14 = r0 & 0x07F00;
        r14 = r14 + 0x10000000;
        r10 = r10 ^ r14;
        arrayId = r14 - 0x10000000;

        store_index[c++] = arrayId;
        if (arrayId < 0 || arrayId > RAM_SIZE) {
           printf("Invalid memory index %d... Exiting \n ",arrayId);
            printf("Loop no %02x",r13);
            exit(0);
        }
        memVal = RamAddressMappingD(arrayId);
        r14 = memVal;
        r0 = r0 ^ r14;
        r14 = mrs;

        // r10 = r10+r0^r14+r15^r12+r15^r11+r13^r9+r8^r7+r6^r5+r4^r3+r2^r1
        tempRes = r10 + r0;
        mrs = ADDwithFlagUpdate(r10, r0, mrs);
        r10 = tempRes;
        r10 = r10 ^ r14;
        /* 25 instructions since the last PC access */
        r15 = r15 + 0x56 + 0x8/*- 0x4 */;
        tempRes = r10 + r15;
        //mrs = ADDwithFlagUpdate(r10, r15, mrs);
        r10 = tempRes;
        r10 = r10 ^ r12;
        /* 2 more instructions since the last PC access */
        r15 = r15 + 0x6;
        tempRes = r10 + r15;
        //mrs = ADDwithFlagUpdate(r10, r15, mrs);
        r10 = tempRes;
        r10 = r10 ^ r11;
        tempRes = r10 + r13;
        //mrs = ADDwithFlagUpdate(r10, r13, mrs);
        r10 = tempRes;
        r10 = r10 ^ r9;
        tempRes = r10 + r8;
        mrs = ADDwithFlagUpdate(r10, r8, mrs);
        r10 = tempRes;
        r10 = r10 ^ r7;
        tempRes = r10 + r6;
        mrs = ADDwithFlagUpdate(r10, r6, mrs);
        r10 = tempRes;
        r10 = r10 ^ r5;
        tempRes = r10 + r4;
        mrs = ADDwithFlagUpdate(r10, r4, mrs);
        r10 = tempRes;
        r10 = r10 ^ r3;
        tempRes = r10 + r2;
        mrs = ADDwithFlagUpdate(r10, r2, mrs);
        r10 = tempRes;
        r10 = r10 ^ r1;

        temp = _rotr(r10, 1);
        r10 = temp;
        r0 = r0 ^ r10;

        // Update register r11 with the checksum piece
        r14 = r0 * r0;
        r14 = r14 | 0x5;
        r0 = r0 + r14;

        // 1-2 KB walk address
        r14 = r0 & 0x07F00;
        r14 = r14 + 0x10000000;
        r11 = r11 ^ r14;
        arrayId = r14 - 0x10000000 ;
        //printf("ArrayIndex %d \n ",arrayId);
        store_index[c++] = arrayId;

        if (arrayId < 0 || arrayId > RAM_SIZE) {
          printf("Invalid memory index %d... Exiting \n ",arrayId);
            printf("Loop no %02x",r13);
            exit(0);
        }
        memVal = RamAddressMappingD(arrayId);
        r14 = memVal;
        r0 = r0 ^ r14;
        r14 = mrs;

        // r11 = r11+r0^r14+r15^r12+r15^r10+r13^r9+r8^r7+r6^r5+r4^r3+r2^r1
        tempRes = r11 + r0;
        mrs = ADDwithFlagUpdate(r11, r0, mrs);
        r11 = tempRes;
        r11 = r11 ^ r14;
        /* 25 instructions since the last PC access */
        r15 = r15 + 0x56 + 0x8/*- 0x4 */;
        tempRes = r11 + r15;
        //mrs = ADDwithFlagUpdate(r11, r15, mrs);
        r11 = tempRes;
        r11 = r11 ^ r12;
        /* 2 more instructions since the last PC access */
        r15 = r15 + 0x6;
        tempRes = r11 + r15;
        //mrs = ADDwithFlagUpdate(r11, r15, mrs);
        r11 = tempRes;
        r11 = r11 ^ r10;
        tempRes = r11 + r13;
        //mrs = ADDwithFlagUpdate(r11, r13, mrs);
        r11 = tempRes;
        r11 = r11 ^ r9;
        tempRes = r11 + r8;
        mrs = ADDwithFlagUpdate(r11, r8, mrs);
        r11 = tempRes;
        r11 = r11 ^ r7;
        tempRes = r11 + r6;
        mrs = ADDwithFlagUpdate(r11, r6, mrs);
        r11 = tempRes;
        r11 = r11 ^ r5;
        tempRes = r11 + r4;
        mrs = ADDwithFlagUpdate(r11, r4, mrs);
        r11 = tempRes;
        r11 = r11 ^ r3;
        tempRes = r11 + r2;
        mrs = ADDwithFlagUpdate(r11, r2, mrs);
        r11 = tempRes;
        r11 = r11 ^ r1;

        temp = _rotr(r11, 1);
        r11 = temp;
        r0 = r0 ^ r11;

        // Update register r12 with the checksum piece
        r14 = r0 * r0;
        r14 = r14 | 0x5;
        r0 = r0 + r14;

        // ; 16KB walk address from RAM
        r14 = r0 & 0x07F00;
        r14 = r14 + 0x10000000;
        r12 = r12 ^ r14;
        arrayId = r14 - 0x10000000;

        store_index[c++] = arrayId;
        if (arrayId < 0 || arrayId > RAM_SIZE) {
           printf("Invalid memory index %d... Exiting \n ",arrayId);
            printf("Loop no %02x",r13);
            exit(0);
        }
        memVal = RamAddressMappingD(arrayId);
        r14 = memVal;
        r0 = r0 ^ r14;
        r14 = mrs;

        // r12 = r12+r0^r14+r15^r11+r15^r10+r13^r9+r8^r7+r6^r5+r4^r3+r2^r1
        tempRes = r12 + r0;
        mrs = ADDwithFlagUpdate(r12, r0, mrs);
        r12 = tempRes;
        r12 = r12 ^ r14;
        /* 25 instructions since the last PC access */
        r15 = r15 + 0x56 + 0x8/*- 0x4 */;
        tempRes = r12 + r15;
        //mrs = ADDwithFlagUpdate(r12, r15, mrs);
        r12 = tempRes;
        r12 = r12 ^ r11;
        /* 2 more instructions since the last PC access */
        r15 = r15 + 0x6;
        tempRes = r12 + r15;
        //mrs = ADDwithFlagUpdate(r12, r15, mrs);
        r12 = tempRes;
        r12 = r12 ^ r10;
        tempRes = r12 + r13;
        //mrs = ADDwithFlagUpdate(r12, r13, mrs);
        r12 = tempRes;
        r12 = r12 ^ r9;
        tempRes = r12 + r8;
        mrs = ADDwithFlagUpdate(r12, r8, mrs);
        r12 = tempRes;
        r12 = r12 ^ r7;
        tempRes = r12 + r6;
        mrs = ADDwithFlagUpdate(r12, r6, mrs);
        r12 = tempRes;
        r12 = r12 ^ r5;
        tempRes = r12 + r4;
        mrs = ADDwithFlagUpdate(r12, r4, mrs);
        r12 = tempRes;
        r12 = r12 ^ r3;
        tempRes = r12 + r2;
        mrs = ADDwithFlagUpdate(r12, r2, mrs);
        r12 = tempRes;
        r12 = r12 ^ r1;

        temp = _rotr(r12, 1);
        r12 = temp;
        r0 = r0 ^ r12;

        // SUB instruction to update the C,Z,N,O flags
        mrs = SUBwithFlagUpdate(r13, 0x8, mrs);
    }

end:

    /*
    for(i=0;i<c;i++){
        //printf("\t %d", store_index[i]);
        RamAddressMappingD(store_index[i]);
    }
    printf("\n Checksum piece r1 = %02x \n", r1);
    printf(" Checksum piece r2 = %02x \n", r2);
    printf(" Checksum piece r3 = %02x \n", r3);
    printf(" Checksum piece r4 = %02x \n", r4);
    printf(" Checksum piece r5 = %02x \n", r5);
    printf(" Checksum piece r6 = %02x \n", r6);
    printf(" Checksum piece r7 = %02x \n", r7);
    printf(" Checksum piece r8 = %02x \n", r8);
    printf(" Checksum piece r9 = %02x \n", r9);
    printf(" Checksum piece r10 = %02x \n", r10);
    printf(" Checksum piece r11 = %02x \n", r11);
    printf(" Checksum piece r12 = %02x \n", r12);
     * */
    bzero(checksumD, 48);

    computeChecksumD(r1);
    computeChecksumD(r2);
    computeChecksumD(r3);
    computeChecksumD(r4);
    computeChecksumD(r5);
    computeChecksumD(r6);
    computeChecksumD(r7);
    computeChecksumD(r8);
    computeChecksumD(r9);
    computeChecksumD(r10);
    computeChecksumD(r11);
    computeChecksumD(r12);
    printf("%s", KGRN);
    for (i = 0; checksumD[i] != '\0'; i++) {
        checksumD[i] = tolower(checksumD[i]);
    }
    printf("Checksum computed by simulator : \n");
    printf("%s", KBLU);
    printf("%s\n", checksumD);
     printf("\nChecksum computed by the Board and the Simulator match ! \n");
    printf("%s", KGRN);
    fclose(fp);

}/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


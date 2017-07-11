
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
/*
 * display - show the returned buffer in hex
 */
void display(const unsigned char *str) {

    int val;
    //printf("%s \n", str);
    while (*str) {
        val = (unsigned int) *str;
        //printf("%d ", (unsigned int) *str++);
        printf("%02x", (unsigned int) *str++);

    }
    //sprintf(buffer, "%lX", str);
    printf("\n");

}

/* Hex to Int converter*/
unsigned long hex2int(char *a, unsigned int len) {
    int i;
    unsigned long val = 0;

    for (i = 0; i < len; i++)
        if (a[i] <= 57)
            val += (a[i] - 48)*(1 << (4 * (len - 1 - i)));
        else
            val += (a[i] - 55)*(1 << (4 * (len - 1 - i)));

    return val;
}


/* Implement ARM ROR Rotate Right instruction*/
unsigned long _rotr(unsigned long value, int shift) {
    if ((shift &= 31) == 0)
        return value;
    return (value >> shift) | (value << (32 - shift));
}

unsigned long ADDwithFlagUpdate(unsigned long r1, unsigned long r2, unsigned long cpsr) {

    unsigned long result;
    int negFlag = 0;
    int overflowFlag = 0;
    int carryFlag = 0;
    int msbOpA, msbOpB, msbRes;

    // Update the Carry, Zero , Negative and Overflow flags

    // Update the Zero Flag
    // 30th bit of the status register is the Zero flag
    result = r1 + r2;
    if (result == 0x0) {
        cpsr |= 1 << 30;
    } else {
        cpsr &= ~(1 << 30);
    }
    // Update the Negative Flag
    msbRes = result >> (sizeof (result)*8 - 1) & 1;
    // 31st bit of the status register is the Negative flag
    negFlag = msbRes;
    if (negFlag) {
        cpsr |= 1 << 31;
    } else {
        cpsr &= ~(1 << 31);
    }

    carryFlag = 0;
    msbOpA = r1 >> (sizeof (r1)*8 - 1) & 1;
    msbOpB = r2 >> (sizeof (r2)*8 - 1) & 1;

    if (msbOpA && msbOpB) {
        carryFlag = 1;
    }
    if (msbOpA && (!msbRes)) {
        carryFlag = 1;
    }
    if (msbOpB && (!msbRes)) {
        carryFlag = 1;
    }
    // 29th bit of the status register is the Carry flag
    if (carryFlag)

        cpsr |= 1 << 29;
    else

        cpsr &= ~(1 << 29);

    overflowFlag = 0;
    // Update the Overflow flag
    if (msbOpA && msbOpB && !msbRes)
        overflowFlag = 1;
    if (!msbOpA && !msbOpB && msbRes)
        overflowFlag = 1;

    // 28th bit of the status register is the Overflow flag
    if (overflowFlag)
        cpsr |= 1 << 28;
    else
        cpsr &= ~(1 << 28);

    /* Read of the EPSR registers on Cortex is not allowed and hence we need to always clear these bits in the status regsiter . See the register set in manual Section 3.10.6 MRS*/
    /* Return the updated status register */
    cpsr &= ~(1 << 25);
    cpsr &= ~(1 << 26);
    return cpsr;
}

unsigned long SUBwithFlagUpdate(unsigned long r1, unsigned long r2, unsigned long cpsr) {

    unsigned long result;
    int negFlag = 0;
    int OverflowFlag = 0;
    int carryFlag = 0;
    int msbOpA, msbOpB, msbRes;

    // Update the Carry, Zero , Negative and Overflow flags

    // Update the Zero Flag
    // 30th bit of the status register is the Zero flag
    result = r1 - r2;
    if (result == 0x0) {
        cpsr |= 1 << 30;
    } else {
        cpsr &= ~(1 << 30);
    }
    // Update the Negative Flag
    msbRes = result >> (sizeof (result)*8 - 1) & 1;
    // 31st bit of the status register is the Negative flag
    negFlag = msbRes;
    if (negFlag) {
        cpsr |= 1 << 31;
    } else {
        cpsr &= ~(1 << 31);
    }

    carryFlag = 0;
    msbOpA = r1 >> (sizeof (r1)*8 - 1) & 1;
    msbOpB = r2 >> (sizeof (r2)*8 - 1) & 1;

    if (msbOpA && msbOpB) {
        carryFlag = 1;
    }
    if (msbOpA && (!msbRes)) {
        carryFlag = 1;
    }
    if (msbOpB && (!msbRes)) {
        carryFlag = 1;
    }
    // 29th bit of the status register is the Carry flag
    // Subtraction operation C Flag = NOT BorrowFrom(operation)
    // See http://stackoverflow.com/questions/27327778/wrong-usage-of-carry-flag-in-arm-subtract-instructions

    if (carryFlag)
        cpsr &= ~(1 << 29);
    else
        cpsr |= 1 << 29;

    // Update the Overflow flag
    if (msbOpA && msbOpB && !msbRes)
        OverflowFlag = 1;
    if (!msbOpA && !msbOpB && msbRes)
        OverflowFlag = 1;

    // 28th bit of the status register is the Overflow flag
    if (OverflowFlag)
        cpsr |= 1 << 28;
    else
        cpsr &= ~(1 << 28);

    /* Return the updated cpsr status register */
    return cpsr;
}

float timedifference_msec(struct timeval t0, struct timeval t1) {
    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

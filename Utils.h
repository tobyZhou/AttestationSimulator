/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Utils.h
 * Author: sumeet.j
 *
 * Created on 23 May, 2016, 1:35 PM
 */

#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C" {
#endif
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
void display(const unsigned char *str);
unsigned long hex2int(char *a, unsigned int len);
unsigned long _rotr(unsigned long value, int shift);
unsigned long ADDwithFlagUpdate(unsigned long r1, unsigned long r2, unsigned long cpsr);
unsigned long SUBwithFlagUpdate(unsigned long r1, unsigned long r2, unsigned long cpsr);
float timedifference_msec(struct timeval t0, struct timeval t1);
#ifdef __cplusplus
}
#endif

#endif /* UTILS_H */


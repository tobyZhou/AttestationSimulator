/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Simulate_MainBoard.h
 * Author: sumeet.j
 *
 * Created on 23 May, 2016, 7:11 PM
 */

#ifndef SIMULATE_MAINBOARD_H
#define SIMULATE_MAINBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#define RAM_SIZE 16320  // 0x3FC0
long final_hexM[RAM_SIZE];
char stringM[10];
char bufferM[48];
char checksumM[48];
char temp1[96];


#ifdef __cplusplus
}
#endif

#endif /* SIMULATE_MAINBOARD_H */


/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   hmac.h
 * Author: zhou.bin
 *
 * Created on 22 June, 2017, 4:27 PM
 */

#include <string.h>
#include "md5.h"

void lrad_hmac_md5(const unsigned char *text, int text_len,
	      const unsigned char *key, int key_len,
	      unsigned char *digest);



/**
  ******************************************************************************
  * @file    com_func.h
  * @company Shanghai Changxing Software Co.,Ltd.
  * @date    2016/1/7
  * @brief   common function head file.
  ******************************************************************************
  * @attention
  *
  * Copyright (C) 2016 Shanghai Changxing Software Co.,Ltd.
  * All rights reserved.
  *
  ******************************************************************************
  */

#ifndef COM_FUNC_H
#define COM_FUNC_H

#include "com_def.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define MAX(x, y)       (x > y ? x : y)
#define MIN(x, y)       (x < y ? x : y)

// big endian
unsigned short com_getbyte2_be(const unsigned char* buf);

unsigned int com_getbyte4_be(const unsigned char* buf);

unsigned long long int com_getbyte8_be(const unsigned char* buf);

void com_setbyte2_be(unsigned short src, unsigned char* buf);

void com_setbyte4_be(unsigned int src, unsigned char* buf);

void com_setbyte8_be(unsigned long long int src, unsigned char* buf);

// little endian
unsigned short com_getbyte2_le(const unsigned char* buf);

unsigned int com_getbyte4_le(const unsigned char* buf);

unsigned long long int com_getbyte8_le(const unsigned char* buf);

void com_setbyte2_le(unsigned short src, unsigned char* buf);

void com_setbyte4_le(unsigned int src, unsigned char* buf);

void com_setbyte8_le(unsigned long long int src, unsigned char* buf);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // COM_FUNC_H

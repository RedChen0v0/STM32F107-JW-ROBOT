#ifndef __USART_H
#define __USART_H

#include <stm32f10x.h>
#include <stdio.h>
//#include "string.h"       // C标准库头文件: 字符数组常用：strcpy()、strncpy()、strcmp()、strlen()、strnset()
//#include "stdarg.h"       // C标准库头文件，由standard（标准） arguments（参数）简化而来，主要目的为让函数能够接收可变参数

#define UART3_BAUD 115200
#define USART3_RX_BUFFER_SIZE  256

void USART3_Init(uint32_t baudrate);
void USART3_UartWrite(uint8_t *buf,uint8_t len);

#endif
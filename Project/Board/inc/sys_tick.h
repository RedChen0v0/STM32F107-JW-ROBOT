// coding UTF8


/**
* Copyright (c) 2021 Hefei Xiaobu Technologies Co., LTD. All rights reserved
*/


#ifndef __SYS_TICK_H
#define __SYS_TICK_H

#include <stdint.h>
#include "stm32f10x_tim.h"

#define SYS_FREQ  100000  //系统时钟每毫秒中断

void sys_tick_init(int freq);
void delay_ms(uint32_t time);
void Delay_ms(__IO uint32_t nTime);
void Delay_us(__IO u32 nTime);

#endif

// coding UTF8


/**
* Copyright (c) 2021 Hefei Xiaobu Technologies Co., LTD. All rights reserved
*/

#ifndef __TIMER_H
#define __TIMER_H

/*
接口的 定义 ，来自与 都问题的 建模/抽象

本文件 定义 timer 通用定时器的接口

本文件 定义 pwm 发生器 的接口

*/
#include <stdint.h>
#include "stm32f10x.h"


// 因为 timer 4 的 TIM_Prescaler 是9分频;
// 72000000/(9+1) = 7200000 分配之后，计数频率为 7.2 M

// 即 1s 中，就可以从 0 计数到  7200000 - 1 

// 频率 为 100 hz ， 则 是 1 s 内 出现 100 次 计数完成次数。
// 所以 重装载值 为 7200000/100 - 1
#define TIM_PRESCALER             9
#define TIM_AFTER_PRESCALER_FRENQ 7200000

#define TIM5_ARR    (TIM_AFTER_PRESCALER_FRENQ/1000) - 1
#define TIM5_PSC     TIM_PRESCALER

typedef enum
{
    bd_timer_no = 0,
    bd_timer_1 = 1,
    bd_timer_2,
    bd_timer_3,
    bd_timer_4,
    bd_timer_5,
    bd_timer_6
} bd_timer;


void  timer_init(bd_timer  timer, unsigned int timeout_ms );

void pwm_tim_init(bd_timer timer);
void pwm_tim_set_freq(bd_timer timer,uint32_t freq);

void tim_cmd_close(bd_timer  timer);
void tim_cmd_open(bd_timer  timer);

void TIM4_PWM_Init(u16 arr,u16 psc);
void TIM1_PWM_Init(u16 arr,u16 psc);
void Timer1_Init(u16 arr,u16 psc) ;
void TIM5_PWM_Init(u16 arr,u16 psc);
#endif

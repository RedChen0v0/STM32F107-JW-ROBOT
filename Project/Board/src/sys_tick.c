// coding UTF8
/**
* Copyright (c) 2021 Hefei Xiaobu Technologies Co., LTD. All rights reserved
*/

/*
CM3 内核 定时器 相关软件实现
*/

#include <stm32f10x.h>
#include <system_stm32f10x.h>
#include "sys_tick.h"

static __IO u32 TimingDelay;

/* freq = 1000    1ms中断一次
 * freq = 100000	10us中断一次
 * freq = 1000000 1us中断一次
 */
void sys_tick_init(int freq)
{
    uint32_t  ticks_between_2_irq = SystemCoreClock / (freq);
    SysTick_Config(ticks_between_2_irq);
}



/*us 定时器延时程序,10us为一个单位 */
void Delay_us(__IO u32 nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/*ms 定时器延时程序,1ms为一个单位 */
void Delay_ms(__IO u32 nTime)
{ 
  TimingDelay = 100*nTime;

  while(TimingDelay != 0);
}

/*ms 软件延时,1ms为一个单位 */
void delay_ms(uint32_t time)
{
    uint32_t i=0;
    while(time--)
    {
        i=12000;
        while(i--) ;
    }
}


/* 获取节拍程序，在 SysTick 中断函数 SysTick_Handler()调用	 */  
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

void SysTick_Handler(void)
{
    /* 进入中断 */
		TimingDelay_Decrement();	
}



#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f10x.h"
#include <stdbool.h>

#define TIM4_CLOCK_FREQ 7200000 //定时器4的时钟频率


#define MAX_SPEED	  2000	      //最大速度
#define MIN_SPEED   1000		    //最小速度


#define FSPR   			200         //电机单圈步数
#define MICRO_STEP  16		      //电机细分   
#define SPR  (FSPR*MICRO_STEP)  //电机旋转一圈脉冲数

#define PULSE_NUM SPR*2    //设置脉冲总数



typedef enum
{
    FORWARD = 0,  //电机正转
    BACKWARD      //电机反转
}Motor_Dir_e;

typedef enum
{
    ENA = 0,     //电机使能
    DIS          //电机失能
}Motor_En_e;

typedef enum {
	NO,
	YES
}flag_value_e;

typedef struct{
  Motor_Dir_e    Motor_Dir;     //电机方向
	Motor_En_e     Motor_En;      //电机使能
  uint16_t       Speed;         //电机速度      
  uint32_t       Pulse_Count;   //pwm脉冲计数
	uint32_t       Pulse_Max;     //脉冲总数
	uint8_t        Motor_Flag;    //电机是否到达位置
	flag_value_e   forward_limit_flag;       //前进限位标志
  flag_value_e   backward_limit_flag;     //后退限位标志
}Motor_t;


#endif
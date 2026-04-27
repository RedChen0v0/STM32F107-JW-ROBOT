#ifndef _SPEED_H
#define _SPEED_H

#include "stm32f10x.h"
#include <system_stm32f10x.h>

#define TIM5_CLOCK_FREQ 7200000 //定时器4的时钟频率

#define FSPR   			200         //电机单圈步数
#define MICRO_STEP  16		      //电机细分   
#define SPR  (FSPR*MICRO_STEP)  //电机旋转一圈脉冲数

#define PulseNum SPR*1  //设置脉冲总数为N圈
#define ACC_STEP_NUM    1000		//加速度需要的步数
#define MAX_SPEED	      2000	  //最大频率
#define MIN_SPEED       1000		//最小频率


typedef enum
{
    ENA = 0,     //电机使能
    DIS          //电机失能
}Motor_En_e;

typedef enum 
{
  SPEED_INCREASE=0, //加速阶段
  SPEED_STABLE, 	  //匀速阶段
  SPEED_DECREASE,   //减速阶段
	SPEED_STOP        //停止阶段
} motor_run_state_e;

typedef enum
{
    FORWARD = 0,  //电机正转
    BACKWARD      //电机反转
}Motor_Dir_e;

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

void CurveS_init(uint16_t *pbuff,uint32_t fre_max,uint32_t fre_min,int16_t len);
void SpeedAdjust(void);
void Motor_Start(uint32_t pulse_num);

#endif



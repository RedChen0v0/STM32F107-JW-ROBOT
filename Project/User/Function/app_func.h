/*************************************************************************
所需头文件
*************************************************************************/
#include <stdio.h>  
#include "usart.h"
#include "timer.h"
#include "gpio.h"
#include "sys_tick.h"
#include "oled.h"
#include "main.h"
#include "485.h"
/*************************************************************************
宏定义
*************************************************************************/
/*----------------------------------------------------------
OLED_LOGO                      定义OLED显示logo状态标志
OLED_DATA                      定义OLED显示data状态标志
OLED_MENU                      定义OLED显示menu状态标志
OLED_CTRL                      定义OLED显示ctrl状态标志
-----------------------------------------------------------*/
#define OLED_LOGO 0
#define OLED_DATA 1
#define OLED_MENU 2
#define OLED_CTRL 3
/*************************************************************************
全局变量
*************************************************************************/
/*----------------------------------------------------------
Motor                          外部定义电机控制结构体
USART3_RxFinished              外部定义USART3串口接收标识
USART3_RxBuffer                外部定义USART3串口数据存放数组
Move_End                       外部定义电机是否运动到尽头标识
RADAR_RX_DIST                  外部定义超声波数据存放变量
RADAR_RX_TEMP                  外部定义温度数据存放变量
Move_Stop                      电机是否停止状态标识
-----------------------------------------------------------*/
extern Motor_t Motor;
extern volatile uint8_t USART3_RxFinished;
extern uint8_t USART3_RxBuffer[USART3_RX_BUFFER_SIZE];
extern uint8_t Move_End;
extern uint16_t RADAR_RX_DIST;
extern uint16_t RADAR_RX_TEMP;

static uint8_t Move_Stop = 0;
/*************************************************************************
电机控制
*************************************************************************/
void Motor_Init(void);
void Motor_Start(Motor_t *Motor);
void MOTOR_Dir(Motor_Dir_e val);
void MOTOR_EN(Motor_En_e val);
void Motor_Stop(void);
void Motor_Resume(void);
void Motor_Check_Status(Motor_t *Motor);
void Motor_Set_Menual_Stop(void);
void Motor_Set_Menual_Run(void);
/*************************************************************************
OLED控制
*************************************************************************/
void print2OLEDln(const char* str);
void showLOGO();
void oledInitprint();
void printnum2OLEDln(const char* str,uint16_t num);
void showMENU();
void showFRAME();
void showDATA();
void showCTRL();
void showLOGO();
/*************************************************************************
LED控制
*************************************************************************/
void Led_1_on(void);
void Led_2_on(void);
void Led_1_off(void);
void Led_2_off(void);
/*************************************************************************
485总线操作//超声波//温度
*************************************************************************/
uint16_t get_distence(void);
uint16_t get_tempture(void);
/*************************************************************************
UART3串口操作
*************************************************************************/
uint8_t getUART3data(void);
void flashUART3(void);
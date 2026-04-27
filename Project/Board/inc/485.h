// coding UTF8

/**
* Copyright (c) 2021 Hefei Xiaobu Technologies Co., LTD. All rights reserved
*/

#pragma once

#include <stdint.h>

#define RS485_1 1
#define RS485_2 1

#define RADAR_ADDR     0x12   //定义超声波雷达地址
#define RADAR_DATA_LEN 0x07  //雷达接收数据长度

#define READ	0x03
#define WRITE	0X06

#define UART2_BAUD  9600
#define UART1_BAUD  9600

#define U1_RX_BUF_SIZE    8
#define U2_RX_BUF_SIZE    8

//UART1
#define USART1_RS485_TX_GPIO GPIOA
#define USART1_RS485_RX_GPIO GPIOA
#define USART1_RS485_EN_GPIO GPIOA

#define USART1_RS485_TX_PIN  GPIO_Pin_9
#define USART1_RS485_RX_PIN  GPIO_Pin_10
#define USART1_RS485_EN_PIN  GPIO_Pin_12


#define USART1_RS485_RX_EN        GPIO_ResetBits(USART1_RS485_EN_GPIO,USART1_RS485_EN_PIN)
#define USART1_RS485_TX_EN        GPIO_SetBits(USART1_RS485_EN_GPIO,USART1_RS485_EN_PIN)

//UART2
#define USART2_RS485_TX_GPIO GPIOD
#define USART2_RS485_RX_GPIO GPIOD
#define USART2_RS485_EN_GPIO GPIOD

#define USART2_RS485_TX_PIN  GPIO_Pin_5
#define USART2_RS485_RX_PIN  GPIO_Pin_6
#define USART2_RS485_EN_PIN  GPIO_Pin_7

#define USART2_RS485_RX_EN        GPIO_ResetBits(USART2_RS485_EN_GPIO,USART2_RS485_EN_PIN)
#define USART2_RS485_TX_EN        GPIO_SetBits(USART2_RS485_EN_GPIO,USART2_RS485_EN_PIN)




typedef struct 
{
	uint8_t   USART1_RX_FLAG; 
	uint8_t   USART1_Count;
	uint8_t   USART1_Busy;
	uint8_t   USART1_Err;
	uint8_t   USART1ReceivedBuffer[U1_RX_BUF_SIZE]; // 接收到数据的缓存
}xUSATR_TypeDef;


extern uint16_t RADA_RxData;           //超声波返回的距离值
extern xUSATR_TypeDef  xUSART;  

void USART1_RS485_Init(uint32_t bound);
void USART2_RS485_Init(uint32_t bound);

void USART1_RS485_UartWrite(uint8_t *buf,uint8_t len);
void USART2_RS485_UartWrite(uint8_t *buf,uint8_t len);
void USART1_RS485_RW_Opr(uint8_t uAddr,uint8_t uCmd,uint16_t uReg,uint16_t uData);
void Uart1_RxData(void);

void usart1_irqhandle_irq(uint8_t res);

uint16_t get_radar_dist_mm(void);
uint16_t get_radar_temp_celsius_times10(void);
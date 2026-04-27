// coding UTF8

/**
* Copyright (c) 2021 Hefei Xiaobu Technologies Co., LTD. All rights reserved
*/


/*
* 485通信 软件实现
*485 通信初始化，接收485通信的数据和发送数据
*/

#include "stm32f10x.h"
#include "gpio.h"
#include "485.h"
#include "sys_tick.h"
//#include "485_server.h"
//#include "sys_config_base.h"

uint16_t RADAR_RX_DIST;
uint16_t RADAR_RX_TEMP;

static uint16_t last_ask_register_addr;

// CRC 高位字节值表
static const uint8_t s_CRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;
// CRC 低位字节值表
const uint8_t s_CRCLo[] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
	0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
	0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
	0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
	0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
	0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
	0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
	0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
	0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
	0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
	0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
	0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
	0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
	0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
	0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
	0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
	0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
	0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
	0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
	0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
	0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

xUSATR_TypeDef xUSART ={

	.USART1_RX_FLAG = 0,
	.USART1_Count = 0,
	.USART1_Busy = 0,
	.USART1_Err = 0,
};

uint16_t RADA_RxData;  

/*
*********************************************************************************************************
*	函 数 名: CRC16_Modbus
*	功能说明: 计算CRC。 用于Modbus协议。
*	形    参: _pBuf : 参与校验的数据
*			  _		usLen : 数据长度
*	返 回 值: 16位整数值。 对于Modbus ，此结果高字节先传送，低字节后传送。
*
*   所有可能的CRC值都被预装在两个数组当中，当计算报文内容时可以简单的索引即可；
*   一个数组包含有16位CRC域的所有256个可能的高位字节，另一个数组含有低位字节的值；
*   这种索引访问CRC的方式提供了比对报文缓冲区的每一个新字符都计算新的CRC更快的方法；
*********************************************************************************************************
*/
uint16_t CRC16_Modbus(uint8_t *_pBuf, uint16_t _usLen)
{
	uint8_t ucCRCHi = 0xFF; /* 高CRC字节初始化 */
	uint8_t ucCRCLo = 0xFF; /* 低CRC 字节初始化 */
	uint16_t usIndex;  /* CRC循环中的索引 */

    while (_usLen--)
    {
		usIndex = ucCRCHi ^ *_pBuf++; /* 计算CRC */
		ucCRCHi = ucCRCLo ^ s_CRCHi[usIndex];
		ucCRCLo = s_CRCLo[usIndex];
    }
    return ((uint16_t)ucCRCHi << 8 | ucCRCLo);
}


//==================================================================================================
//
// 下面是 静态 函数
//
//==================================================================================================

//==================================================================================================
//
// 下面是 接口函数 实现
//
//==================================================================================================

/******************************************************************************
 * 函  数： USART1_RS485_Configuration
 * 功  能： 设置串口3发送与接收引脚的模式
 * 参  数： 无
 * 返回值： 无
 ******************************************************************************/  
static void USART1_RS485_GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = USART1_RS485_EN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(USART1_RS485_EN_GPIO,&GPIO_InitStructure);
		// GPIO_TX引脚配置
    GPIO_InitStructure.GPIO_Pin   = USART1_RS485_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;                // TX引脚，配置为复用推挽工作模式
    GPIO_Init(USART1_RS485_RX_GPIO, &GPIO_InitStructure);
    // GPIO_RX引脚配置
    GPIO_InitStructure.GPIO_Pin   = USART1_RS485_RX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING ;         // RX引脚，配置为浮空输入工作模式
    GPIO_Init(USART1_RS485_RX_GPIO, &GPIO_InitStructure);
	
}

/******************************************************************************
 * 函  数： USART1_RS485_RCC_Configuration
 * 功  能： 开启GPIOC,串口1时钟
 * 参  数： 无
 * 返回值： 无
 ******************************************************************************/ 
static void USART1_RS485_RCC_Configuration(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);   	//	使能GPIOA时钟\AFIO功能时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);	                        //	使能USART1时钟
}



void USART1_RS485_Init(uint32_t bound)
{
    USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;

	USART1_RS485_RCC_Configuration();
	USART1_RS485_GPIO_Configuration();
	
	// 中断配置
    NVIC_InitStructure .NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure .NVIC_IRQChannelPreemptionPriority=2;       // 抢占优先级
    NVIC_InitStructure .NVIC_IRQChannelSubPriority = 0;             // 子优先级
    NVIC_InitStructure .NVIC_IRQChannelCmd = ENABLE;                // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);  
	
    //USART1 初始化设置
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);

    USART_ClearFlag(USART1, USART_FLAG_TC);
    USART1_RS485_RX_EN;
}

void USART1_RS485_UartWrite(uint8_t *buf,uint8_t len)
{
    uint8_t i=0;
    USART1_RS485_TX_EN;
    delay_ms(3);
    for(i=0; i<len; i++)
    {
      //USART_ClearFlag(USART1,USART_FLAG_TC);   //清除TC位
			USART_SendData(USART1,buf[i]);
			while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
				//USART1->DR = buf[i];
        //while((USART1->SR&0X40)==0);
    }
		delay_ms(3);
    USART1_RS485_RX_EN;
}

/******************************************************************************
 * 函  数： USART2_RS485_Configuration
 * 功  能： 设置串口3发送与接收引脚的模式
 * 参  数： 无
 * 返回值： 无
 ******************************************************************************/  
static void USART2_RS485_GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = USART2_RS485_EN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(USART2_RS485_EN_GPIO,&GPIO_InitStructure);
	// GPIO_TX引脚配置
    GPIO_InitStructure.GPIO_Pin   = USART2_RS485_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;                // TX引脚，配置为复用推挽工作模式
    GPIO_Init(USART2_RS485_RX_GPIO, &GPIO_InitStructure);
    // GPIO_RX引脚配置
    GPIO_InitStructure.GPIO_Pin   = USART2_RS485_RX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING ;    // RX引脚，配置为浮空输入工作模式
    GPIO_Init(USART2_RS485_RX_GPIO, &GPIO_InitStructure);
	
}

/******************************************************************************
 * 函  数： USART2_RS485_RCC_Configuration
 * 功  能： 开启GPIOC,串口2时钟
 * 参  数： 无
 * 返回值： 无
 ******************************************************************************/ 
static void USART2_RS485_RCC_Configuration(void)
{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);   //使能GPIOD时钟\AFIO功能时钟
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);	                        // 使能USART2时钟
}

void USART2_RS485_Init(uint32_t bound)
{
    USART_InitTypeDef USART_InitStructure;
		USART2_RS485_RCC_Configuration();
		USART2_RS485_GPIO_Configuration();
		
		GPIO_PinRemapConfig( GPIO_Remap_USART2,  ENABLE);

    USART_InitStructure.USART_BaudRate = bound;//波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
//    USART_InitStructure.USART_Mode = /*USART_Mode_Rx  | */ USART_Mode_Tx;    //收发模式
    USART_InitStructure.USART_Mode = USART_Mode_Rx  |  USART_Mode_Tx;    //收发模式
    USART_Init(USART2, &USART_InitStructure);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启接受中断
    USART_Cmd(USART2, ENABLE);  //使能串口2

    USART_ClearFlag(USART2, USART_FLAG_TC);

    USART2_RS485_RX_EN;

}

void USART2_RS485_UartWrite(uint8_t *buf,uint8_t len)
{
    uint8_t i=0;
    USART2_RS485_TX_EN;
    delay_ms(3);
    for(i=0; i<len; i++)
    {
      USART_SendData(USART2,buf[i]);
			while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
			
			//USART2->DR = buf[i];
        //while((USART2->SR&0X40)==0);
    }
		delay_ms(3);
    USART2_RS485_RX_EN;
}


/****************************************************************************************************
 * 函数名称：void RS485_RW_Opr(u8 ucAddr,u8 ucCmd,u16 ucReg,u16 uiDate)
 * 入口参数：u8 ucAddr,u8 ucCmd,u16 ucReg,u16 uiDate
 * 			 ucAddr：从机地址
 *			 ucCmd ：功能码 03->读	06->写
 *			 ucReg ：寄存器地址
 *			 uiDate：写操作即是发送的数据 读操作即是读取数据个数
 * 返回  值：无
 * 功能说明：485读写操作函数
 ***************************************************************************************************/   
void USART1_RS485_RW_Opr(uint8_t uAddr,uint8_t uCmd,uint16_t uReg,uint16_t uData)
{
	unsigned int crc;
	unsigned char crcl;
	unsigned char crch;	
	unsigned char uBuf[8];
	
	last_ask_register_addr = uReg;
	
	uBuf[0] = uAddr;				            /* 从机地址 */
	uBuf[1] = uCmd;				              /* 命令 06 写 03 读 */
	uBuf[2] = uReg >> 8;			          /* 寄存器地址高位 */
	uBuf[3] = uReg;				              /* 寄存器地址低位 */
	uBuf[4] = uData >> 8;			          /* 数据高8位 */
	uBuf[5] = uData;				            /* 数据低8位 */
	crc      = CRC16_Modbus(uBuf,6);    /* 计算CRC校验值 */
	crch     = crc >> 8;    		        /* crc高位 */
	crcl     = crc &  0xFF;			        /* crc低位 */
	uBuf[6] = crch;				              /* 校验高8位 */
	uBuf[7] = crcl;				              /* 校验低8位 */
	
	USART1_RS485_UartWrite(uBuf,8);		  /* 发送数据 */
	xUSART.USART1_Busy = 1;					    /* 发送完成后 忙标志置位 */
}


void Uart1_RxData(void)
{
  unsigned char i;
	
	unsigned int crc;
	unsigned char crch,crcl;
	unsigned char len;
	unsigned char buf[RADAR_DATA_LEN];

	
	//if (xUSART.USART1_RX_FLAG)            								//帧接收完成标志，即接收到一帧新数据
	{
		xUSART.USART1_RX_FLAG = 0;           							//帧接收完成标志清零
		
		for(i = 0;i < RADAR_DATA_LEN; i++)  		//拷贝接收到的数据到接收指针中
		{
			buf[i] = xUSART.USART1ReceivedBuffer[i+1];  	//将数据复制到buf中
		}
	
		crc  = CRC16_Modbus(buf,RADAR_DATA_LEN-2);       				//计算CRC校验值，除去CRC校验值
		crch = crc >> 8;    									//crc高位
		crcl = crc & 0xFF;										//crc低位

		if((buf[RADAR_DATA_LEN-2] == crch) && (buf[RADAR_DATA_LEN-1] == crcl))  		//判断CRC校验是否正确
		{
			if (buf[1] == 0x03)									//0x03 读命令
			{
				if(buf[2] == 0x02)  		//寄存器地址支持0x0000~0x0005
				{
				
					/* 通过上面的验证判断后 在这里可直接获取数据 保存在ReadDateVal中 */
					RADA_RxData = (buf[3]<<8) | buf[4]; //雷达数据
				}
			}									
		}
		else	/* 如果校验值错误，执行错误处理函数 这里用串口1打印一条提示信息 也可自定义共它处理函数 */
			xUSART.USART1_Err = 1;
	}
}


static uint8_t usart1_rx_cnt=0; 
static uint8_t usart1_rx_buf[RADAR_DATA_LEN];

void USART1_IRQHandler(void)
{
	uint8_t ch;
	
  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收到数据
  {
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		ch = USART_ReceiveData(USART1);

		usart1_irqhandle_irq(ch);
  }
}


void usart1_irqhandle_irq(uint8_t res)
{
	uint16_t crc ;
	uint8_t crch,crcl;
	
	
	if(usart1_rx_cnt == 0)
	{
        if(RADAR_ADDR == res)
		{
            usart1_rx_buf[usart1_rx_cnt] = res;
            usart1_rx_cnt++;
        }
    }
    else if(usart1_rx_cnt <= RADAR_DATA_LEN)
	{
        usart1_rx_buf[usart1_rx_cnt] = res;
        usart1_rx_cnt++;
        
        if(RADAR_DATA_LEN == usart1_rx_cnt )
		{
;        //进行数据验证
			crc = CRC16_Modbus(usart1_rx_buf,(usart1_rx_cnt - 2));
			crch = crc >> 8;    									//crc高位
			crcl = crc & 0xFF;										//crc低位
				
			if((usart1_rx_buf[usart1_rx_cnt - 2] == crch) && (usart1_rx_buf[usart1_rx_cnt - 1]  == crcl))  		//判断CRC校验是否正确
			{
#if 1
				if(0x0100 == last_ask_register_addr || 0x0101 == last_ask_register_addr){
						RADAR_RX_DIST = (usart1_rx_buf[3]<<8) | usart1_rx_buf[4]; //雷达距离数据
				}else if(0x0102 == last_ask_register_addr){
						RADAR_RX_TEMP = (usart1_rx_buf[3]<<8) | usart1_rx_buf[4]; //雷达温度数据
				}
#endif
				xUSART.USART1_RX_FLAG = 1;
				usart1_rx_cnt = 0;			
			}
		}
	}
}

uint16_t get_radar_dist_mm(void){
		return RADAR_RX_DIST;
}

uint16_t get_radar_temp_celsius_times10(void){
		return RADAR_RX_TEMP;
}

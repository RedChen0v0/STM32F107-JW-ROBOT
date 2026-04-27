#include "usart.h"
uint8_t USART3_RxBuffer[USART3_RX_BUFFER_SIZE]; 
volatile uint16_t USART3_RxCount = 0;
volatile uint8_t USART3_RxFinished = 0;

uint8_t uart3_cmd;

static void USART_RCC_Configuration(void)
{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);   //使能GPIOC时钟\AFIO功能时钟
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);	                        // 使能USART3时钟
}

static void USART3_GPIO_Configuration(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
	    // GPIO_TX引脚配置
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;                // TX引脚
    GPIO_Init (GPIOC, &GPIO_InitStructure);
    // GPIO_RX引脚配置
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING  ;        // RX引脚 
		
		//将USART3的TXD和RXD重映射到PC10和PC11
		GPIO_PinRemapConfig(GPIO_PartialRemap_USART3,ENABLE);           //部分重映像
		GPIO_Init (GPIOC, &GPIO_InitStructure); 
}

void DMA_USART3_RX_Config(void) {
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_InitTypeDef DMA_InitStructure;

    DMA_DeInit(DMA1_Channel3);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART3->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART3_RxBuffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = USART3_RX_BUFFER_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;

    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);

    DMA_Cmd(DMA1_Channel3, ENABLE);
}

void USART3_Enable_DMA_RX(void) {
    USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
}

void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_IDLE) != RESET) {
        volatile uint16_t temp = USART3->SR; 
        temp = USART3->DR; 
        (void)temp; 
        DMA_Cmd(DMA1_Channel3, DISABLE);
        USART3_RxCount = USART3_RX_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel3);
        USART3_RxFinished = 1;
    }
}

//需要在target选项中选择使用 microLIB

void USART3_Init(uint32_t baudrate)
{   
    USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef  NVIC_InitStructure;	
    //USART 时钟设置
		USART_RCC_Configuration();
    //USART 初始化设置
    USART_DeInit(USART3); 
    USART_InitStructure.USART_BaudRate   = baudrate;                // 串口波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;     // 字长为8位数据格式
    USART_InitStructure.USART_StopBits   = USART_StopBits_1;        // 一个停止位
    USART_InitStructure.USART_Parity     = USART_Parity_No;         // 无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // 使能收、发模式
    USART_Init(USART3, &USART_InitStructure);                       // 初始化串口
			// 中断配置
    NVIC_InitStructure .NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure .NVIC_IRQChannelPreemptionPriority=1;       // 抢占优先级
    NVIC_InitStructure .NVIC_IRQChannelSubPriority = 0;             // 子优先级
    NVIC_InitStructure .NVIC_IRQChannelCmd = ENABLE;                // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure); 

    USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
    USART_Cmd(USART3, ENABLE);                                      // 使能串口, 开始工作  
    USART_ClearFlag(USART3, USART_FLAG_TC);
		
		//USART GPIO设置
		USART3_GPIO_Configuration();
		
		DMA_USART3_RX_Config();
		USART3_Enable_DMA_RX();
}

/******************************************************************************
 * 功  能： printf函数支持代码
 *         加入以下代码, 使用printf函数时, 不再需要选择use MicroLIB     
 * 参  数： 
 * 返回值：
 * 备  注： 
 ******************************************************************************/      
#pragma import(__use_no_semihosting)                
struct __FILE       { int handle; };         // 标准库需要的支持函数
FILE __stdout;                               // FILE 在stdio.h文件
void _sys_exit(int x) {    x = x; }          // 定义_sys_exit()以避免使用半主机模式

int fputc(int ch, FILE *f)                   // 重定向fputc函数，使printf的输出，由fputc输出到UART
{                                   
	USART_SendData(USART3,(uint8_t)ch);
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);

	return ch;
}

void USART3_UartWrite(uint8_t *buf, uint8_t len)
{
    uint8_t i=0;
    for(i=0; i<len; i++)
    {
      USART_SendData(USART3,buf[i]);
			while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    }
}
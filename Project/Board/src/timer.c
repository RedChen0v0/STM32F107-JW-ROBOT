#include "stm32f10x.h"
#include "timer.h"
#include "usart.h"
#include "main.h"
#include "stm32f10x_exti.h"

extern Motor_t Motor;

/******************************************************************************
 * 函  数： USART_RCC_Configuration
 * 功  能： 开启GPIOC,串口3时钟
 * 参  数： 无
 * 返回值： 无
 ******************************************************************************/ 
static void PWM_RCC_Configuration(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //使能GPIOA时钟，GPIOA挂载在APB2时钟下，在STM32中使用IO口前都要使能对应时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);  //使能通用定时器5时钟
}

/******************************************************************************
 * 函  数： USART3_GPIO_Configuration
 * 功  能： 设置串口3发送与接收引脚的模式
 * 参  数： 无
 * 返回值： 无
 ******************************************************************************/  
static void PWM_GPIO_Configuration(void)
{
		//PWM输出引脚
	GPIO_InitTypeDef GPIO_InitStructure; //定义一个引脚初始化的结构体
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;//引脚PA0
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP; //复用推挽输出模式，定时器功能为PB9引脚复用功能
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz; //定义该引脚输出速度为50MHZ
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化引脚PA0
}

void TIM5_PWM_Init(u16 arr,u16 psc)
{  

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStrue; //定义一个定时中断的结构体	
	TIM_OCInitTypeDef TIM_OCInitTypeStrue; //定义一个PWM输出的结构体
	NVIC_InitTypeDef NVIC_InitStructure;
	
	PWM_RCC_Configuration();
	PWM_GPIO_Configuration();
	
	//定时器结构体初始化
	TIM_TimeBaseInitStrue.TIM_Period = arr; //计数模式为向上计数时，定时器从0开始计数，计数超过到arr时触发定时中断服务函数
	TIM_TimeBaseInitStrue.TIM_Prescaler =  psc; //预分频系数，决定每一个计数的时长
	TIM_TimeBaseInitStrue.TIM_CounterMode = TIM_CounterMode_Up; //计数模式：向上计数
	TIM_TimeBaseInitStrue.TIM_ClockDivision = TIM_CKD_DIV1; //一般不使用，默认TIM_CKD_DIV1
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseInitStrue); //根据TIM_TimeBaseInitStrue的参数初始化定时器TIM4
	
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE );  //使能指定的TIM5中断,允许更新中断
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;   //TIM5中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
	
	
	TIM_OCStructInit(&TIM_OCInitTypeStrue);                    //设置缺省值
	TIM_OCInitTypeStrue.TIM_OCMode = TIM_OCMode_PWM1; //PWM模式1，当定时器计数小于TIM_Pulse时，定时器对应IO输出有效电平
	TIM_OCInitTypeStrue.TIM_OCPolarity = TIM_OCNPolarity_High; //输出有效电平为高电平
	TIM_OCInitTypeStrue.TIM_OutputState = TIM_OutputState_Enable; //使能PWM输出
	TIM_OCInitTypeStrue.TIM_Pulse =0; //arr/2  设置待装入捕获比较寄存器的脉冲值
	//TIM_OCInitTypeStrue.TIM_Pulse = 500; //设置待装入捕获比较寄存器的脉冲值
	TIM_OC1Init(TIM5, &TIM_OCInitTypeStrue); //根TIM_OCInitTypeStrue参数初始化定时器3通道4
 
	TIM_OC1PreloadConfig(TIM5, TIM_OCPreload_Enable); //CH4预装载 失能后改变TIM_Pulse(即PWM)的值立刻生效，使能则下个周期生效
	//TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Disable); //CH4预装载 失能后改变TIM_Pulse(即PWM)的值立刻生效，使能则下个周期生效
	
	TIM_ARRPreloadConfig(TIM5, ENABLE); //TIM4预装载使能，使能后改变arr立即生效，不使能则下个周期生效
	
	//TIM_Cmd(TIM4, ENABLE); //使能定时器TIM4
	TIM_Cmd(TIM5, DISABLE); //使能定时器TIM4
	

}


void TIM1_PWM_Init(u16 arr,u16 psc)
{  

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStrue; //定义一个定时中断的结构体	
	TIM_OCInitTypeDef TIM_OCInitTypeStrue; //定义一个PWM输出的结构体
	NVIC_InitTypeDef NVIC_InitStructure;
	
	PWM_RCC_Configuration();
	PWM_GPIO_Configuration();
	
	//定时器结构体初始化
	TIM_TimeBaseInitStrue.TIM_Period = arr; //计数模式为向上计数时，定时器从0开始计数，计数超过到arr时触发定时中断服务函数
	TIM_TimeBaseInitStrue.TIM_Prescaler =  psc; //预分频系数，决定每一个计数的时长
	TIM_TimeBaseInitStrue.TIM_CounterMode = TIM_CounterMode_Up; //计数模式：向上计数
	TIM_TimeBaseInitStrue.TIM_ClockDivision = TIM_CKD_DIV1; //一般不使用，默认TIM_CKD_DIV1
	TIM_TimeBaseInitStrue.TIM_RepetitionCounter = 0;//重复计数设置
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStrue); //根据TIM_TimeBaseInitStrue的参数初始化定时器TIM4
	

	TIM_ClearFlag(TIM1,TIM_FLAG_Update);
 // TIM_ITConfig(TIM1,TIM_IT_Update|TIM_IT_CC4,ENABLE );  //使能指定的TIM1中断,允许更新中断
	TIM_ITConfig(TIM1,TIM_IT_Update|TIM_IT_Trigger,ENABLE );  //使能指定的TIM1中断,允许更新中断
	  
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;   //TIM1中断   
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
	

	
	TIM_OCStructInit(&TIM_OCInitTypeStrue);                    //设置缺省值
	TIM_OCInitTypeStrue.TIM_OCMode = TIM_OCMode_PWM1; //PWM模式1，当定时器计数小于TIM_Pulse时，定时器对应IO输出有效电平
	TIM_OCInitTypeStrue.TIM_OCPolarity = TIM_OCNPolarity_High; //输出有效电平为高电平
	TIM_OCInitTypeStrue.TIM_OutputState = TIM_OutputState_Enable; //使能PWM输出
	TIM_OCInitTypeStrue.TIM_Pulse =0; //arr/2  设置待装入捕获比较寄存器的脉冲值

	 
	TIM_OCInitTypeStrue.TIM_OCNIdleState = TIM_OCIdleState_Reset; 
	TIM_OCInitTypeStrue.TIM_OCIdleState  = TIM_OCIdleState_Set; 
	 
	TIM_OC4Init(TIM1, &TIM_OCInitTypeStrue); //根TIM_OCInitTypeStrue参数初始化定时器1通道4
 
 
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable); //CH4预装载 失能后改变TIM_Pulse(即PWM)的值立刻生效，使能则下个周期生效效	
	TIM_ARRPreloadConfig(TIM1, ENABLE); //TIM4预装载使能，使能后改变arr立即生效，不使能则下个周期生效
	
	TIM_Cmd(TIM1, DISABLE); //使能定时器TIM1
	TIM_CtrlPWMOutputs(TIM1, ENABLE);  //高级定时器专属--MOE主输出使能

}


void TIM4_IRQHandler(void)   //TIM4中断
{
		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) {
					Motor.Pulse_Count++;
			    if(Motor.Pulse_Count >= Motor.Pulse_Max )
					{
						Motor.Motor_Flag = 1;
						TIM_Cmd(TIM4, DISABLE); //关闭定时器TIM4			
					}
										
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update );  //清除TIMx更新中断标志 
    }
}

void TIM5_IRQHandler(void)   //TIM5中断
{
		if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) {
					Motor.Pulse_Count++;
			    if(Motor.Pulse_Count >= Motor.Pulse_Max )
					{
						Motor.Motor_Flag = 1;
						//TIM_Cmd(TIM5, DISABLE); //关闭定时器TIM5		
					}
										
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update );  //清除TIMx更新中断标志 
    }
}

//void TIM1_UP_IRQHandler(void)   //TIM1中断
void TIM1_UP_IRQHandler(void)  
{
		if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) {
					Motor.Pulse_Count++;
			    if(Motor.Pulse_Count >= Motor.Pulse_Max )
					{
						Motor.Motor_Flag = 1;
						TIM_Cmd(TIM1, DISABLE); //关闭定时器TIM1	
					}
										
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update );  //清除TIMx更新中断标志 
    }
}

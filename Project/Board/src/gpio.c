// coding UTF8


/**
* Copyright (c) 2021 Hefei Xiaobu Technologies Co., LTD. All rights reserved
*/

/*
log:

20230301:   注释pb0,1;pe6引脚初始化（新板子没有使用这3个引脚）
            添加adc引脚初始化程序：pa3,4,5;pb0,1;pc2
*/
 

/**
 GPIO 软件实现
 此部分 实现 xbo mkzgd 板子上GPIO 功能
 
 对于板子上面，已经确定输出，输入的 GPIO 引脚，在此.C 文件的 init/config 函数中已经完成了配置。
*/


#include "stm32f10x.h"
#include "gpio.h"
#include "usart.h"
#include "main.h"

extern Motor_t Motor;
//==================================================================================================
//
// 下面是 静态变量
//
//==================================================================================================


static gpio_callback_irq  def_irq_callback = 0;

static uint16_t pulse_cout = 0;

uint8_t Move_End = 0; 
uint8_t send_flag = 0;
//==================================================================================================
//
// 下面是 静态 函数
//
//==================================================================================================

/*
NVIC配置，中断优先级 ，在硬件初始化（gpio）中调用
*/
static void NVIC_Configuration(void) //优先级配置原则限位及避障中断优先级>脉冲统计中断优先级>网络接收中断优先级
{
    NVIC_InitTypeDef   NVIC_InitStructure;
    /* Set the Vector Table base location at 0x08000000 */
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
   // NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x7800);
    /* 2 bit for pre-emption priority, 2 bits for subpriority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
/*
    这里放的必须配置的中断优先级
    定时器2，3，4，的中断优先级在中断中配置；
*/
    /* Enable the Ethernet global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#if 0
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;        
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            
    NVIC_Init(&NVIC_InitStructure);
    
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif 
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE ;    
    NVIC_Init(&NVIC_InitStructure); 
#if 0
    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE ;    
    NVIC_Init(&NVIC_InitStructure);
    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE ;    
    NVIC_Init(&NVIC_InitStructure);
 #endif 
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE ;    
    NVIC_Init(&NVIC_InitStructure);
}

/*******************************************GPIO初始化函数************************************/
/*
返回值：
   成功，返回 对应的 指针
   失败，返回 0
*/
static GPIO_TypeDef* to_impl_group(bd_gpio_group group)
{
    GPIO_TypeDef* GPIOx = 0;

    if(group >= bd_gpio_group_a  && group <= bd_gpio_group_g){
        GPIOx = (GPIO_TypeDef*)(GPIOA_BASE + 0x0400 * (group - 1) );
    }
    return GPIOx;
}


/*
返回值：
     成功  对应的  pin 值
     失败  返回    0
*/

static uint16_t to_impl_pin(bd_gpio_pin pin)
{
    int pin_int = pin;
    uint16_t ret_pin = 0;
    if(( pin >= bd_gpio_pin_0 ) && (pin <= bd_gpio_pin_15 )){
        ret_pin = (uint16_t)(0x0001 << (pin_int - 1) );
    }
    return ret_pin;

}

static GPIOMode_TypeDef to_impl_mode(bd_gpio_mode mode)
{
    GPIOMode_TypeDef mode_mode = GPIO_Mode_IN_FLOATING;

    switch(mode){
        case bd_gpio_mode_ain:
            mode_mode = GPIO_Mode_AIN;
        break;
        case bd_gpio_mode_in_floating:
            mode_mode = GPIO_Mode_IN_FLOATING;
        break;
        case bd_gpio_mode_ipd:
            mode_mode = GPIO_Mode_IPD;
        break;
        case bd_gpio_mode_ipu:
            mode_mode = GPIO_Mode_IPU;
        break;
        case bd_gpio_mode_out_od:
            mode_mode = GPIO_Mode_Out_OD;
        break;
        case bd_gpio_mode_out_pp:
            mode_mode = GPIO_Mode_Out_PP;
        break;
        case bd_gpio_mode_af_od:
            mode_mode = GPIO_Mode_AF_OD;
        break;
        case bd_gpio_mode_af_pp:
            mode_mode = GPIO_Mode_AF_PP;
        break;
    }
    return mode_mode;

}

static GPIOSpeed_TypeDef to_impl_speed(bd_gpio_speed speed)
{
    GPIOSpeed_TypeDef speed_speed = GPIO_Speed_2MHz;
    switch(speed){
        case bd_gpio_speed_10mhz:
            speed_speed = GPIO_Speed_10MHz;
        break;
        case bd_gpio_speed_2mhz:
            speed_speed = GPIO_Speed_2MHz;
        break;
        case bd_gpio_speed_50mhz:
            speed_speed = GPIO_Speed_50MHz;
        break;
    }
    return speed_speed;
}
/*******************************************GPIO初始化函数************************************/

/*******************************************中断初始化函数************************************/
static uint32_t to_impl_exti_line(bd_exti_line line)
{
    int line_int = line;
    uint32_t ret_line = 0;
    if(( line >= bd_exti_line_0 ) && (line <= bd_exti_line_19 )){
        ret_line = (uint32_t)(0x0001 << line_int);
    }
    return ret_line;
}

static EXTIMode_TypeDef to_impl_exti_mode(bd_exti_mode mode)
{
    EXTIMode_TypeDef mode_mode = EXTI_Mode_Interrupt;

    switch(mode){
        case bd_exti_mode_interrupt:
            mode_mode = EXTI_Mode_Interrupt;
        break;
        case bd_exti_mode_event:
            mode_mode = EXTI_Mode_Event;
        break;

    }
    return mode_mode;

}

static EXTITrigger_TypeDef to_impl_exti_trigger(bd_exti_trigger trigger)
{
    EXTITrigger_TypeDef trigger_trigger = EXTI_Trigger_Rising;

    switch(trigger){
        case bd_exti_trigger_rising:
            trigger_trigger = EXTI_Trigger_Rising;
        break;
        case bd_exti_trigger_falling:
            trigger_trigger = EXTI_Trigger_Falling;
        break;
        case bd_exti_trigger_rising_falling:
            trigger_trigger = EXTI_Trigger_Rising_Falling;
        break;

    }
    return trigger_trigger;

}
/*******************************************终端初始化函数************************************/

//==================================================================================================
//
// 下面是 导出的接口函数
//
//==================================================================================================
void set_gpio_config(bd_gpio_group group,  bd_gpio_pin pin, bd_gpio_mode mode ,bd_gpio_speed speed)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_TypeDef* GPIOx = to_impl_group(group);
    if(GPIOx == 0)
    {
        return ;
    }
    uint16_t pinx = to_impl_pin(pin);
    if(pinx == 0)
    {
        return ;
    }
    GPIOMode_TypeDef modex = to_impl_mode(mode);
    GPIOSpeed_TypeDef speedx = to_impl_speed(speed);

    // todo 转化后的返回值检查，

    GPIO_InitStructure.GPIO_Pin = pinx;
    GPIO_InitStructure.GPIO_Speed = speedx;
    GPIO_InitStructure.GPIO_Mode = modex;    
    GPIO_Init(GPIOx, &GPIO_InitStructure);
}

/*
设置单独GPIO口的值 0 低电平   1 高电平
*/
void set_gpio_value(bd_gpio_group group, bd_gpio_pin pin, char val )
{
    GPIO_TypeDef* GPIOx = to_impl_group(group);
    uint16_t GPIO_Pinx = to_impl_pin(pin);
    if(val)
    {
        GPIO_SetBits( GPIOx,  GPIO_Pinx);
    }
    else
    {
        GPIO_ResetBits( GPIOx,  GPIO_Pinx);
    }
}


char get_gpio_output_value(bd_gpio_group group, bd_gpio_pin pin )
{
    GPIO_TypeDef* GPIOx = to_impl_group(group);
    uint16_t GPIO_Pinx = to_impl_pin(pin);

    return GPIO_ReadOutputDataBit( GPIOx,  GPIO_Pinx);
}

char get_gpio_input_value(bd_gpio_group group, bd_gpio_pin pin )
{
    GPIO_TypeDef* GPIOx = to_impl_group(group);
    uint16_t GPIO_Pinx = to_impl_pin(pin);

    return GPIO_ReadInputDataBit( GPIOx,  GPIO_Pinx);
}

void set_exti_config(bd_exti_line line ,bd_exti_mode mode ,bd_exti_trigger tigger)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    uint32_t linex = to_impl_exti_line(line);
    if(0 == linex)
    {
        return ;
    }
    EXTIMode_TypeDef modex = to_impl_exti_mode(mode);
    EXTITrigger_TypeDef tifferx = to_impl_exti_trigger(tigger);
    
    EXTI_InitStructure.EXTI_Line = linex; 
    EXTI_InitStructure.EXTI_Mode = modex;
    EXTI_InitStructure.EXTI_Trigger = tifferx;
    EXTI_InitStructure.EXTI_LineCmd    = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}


/*
 GPIO 软件实现
 此部分 实现 xbo mkzgd 板子上GPIO 功能
 对于板子上面，已经确定输出，输入的 GPIO 引脚，在此.C 文件的 init/config 函数中已经完成了配置。
*/
void gpio_config(void)
{
    //stm32的ETH外设挂载在AHB1总线上，位于RCC_AHB1ENR的bit25-bit27
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx |RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_USART1 
                            |RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);
    
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2,  ENABLE);
    //管脚重映射
    GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);
    GPIO_PinRemapConfig( GPIO_Remap_USART2,  ENABLE);
    
    NVIC_Configuration();



/*
    电机输出配置：   
                PE12  ->  电机DIR
                PB11  ->  电机ENA
                Pa0  ->  电机PULSE
	            
*/
    set_gpio_config(bd_gpio_group_e,bd_gpio_pin_12,bd_gpio_mode_out_pp,bd_gpio_speed_50mhz);
    set_gpio_config(bd_gpio_group_b,bd_gpio_pin_11,bd_gpio_mode_out_pp,bd_gpio_speed_50mhz);
    set_gpio_config(bd_gpio_group_a,bd_gpio_pin_0,bd_gpio_mode_out_pp,bd_gpio_speed_50mhz);
/*
    LED配置：   
                PE3  ->  LED2
                PE4  ->  LED3	            
*/	
    set_gpio_config(bd_gpio_group_e,bd_gpio_pin_3,bd_gpio_mode_out_pp,bd_gpio_speed_50mhz);
    set_gpio_config(bd_gpio_group_e,bd_gpio_pin_4,bd_gpio_mode_out_pp,bd_gpio_speed_50mhz);
		set_gpio_value(bd_gpio_group_e,bd_gpio_pin_4,1);
		set_gpio_value(bd_gpio_group_e,bd_gpio_pin_3,1);
		
		
		
		/*
输入中断配置：      
                    PC13 ->  水平限位接近开关
                    PC14 ->  水平限位接近开关

*/		
	set_gpio_config(bd_gpio_group_c,bd_gpio_pin_13,bd_gpio_mode_ipu,bd_gpio_speed_50mhz);
    GPIO_EXTILineConfig( GPIO_PortSourceGPIOC,  GPIO_PinSource13);
    set_exti_config(bd_exti_line_13, bd_exti_mode_interrupt,bd_exti_trigger_falling);
    
    set_gpio_config(bd_gpio_group_c,bd_gpio_pin_14,bd_gpio_mode_ipu,bd_gpio_speed_50mhz);
    GPIO_EXTILineConfig( GPIO_PortSourceGPIOC,  GPIO_PinSource14);
    set_exti_config(bd_exti_line_14, bd_exti_mode_interrupt,bd_exti_trigger_falling);
}


void exti_forward_handle(Motor_t * motor_control_obj)
{
	if( (0 == get_gpio_input_value(FORWARD_LIMIT_GROUP,FORWARD_LIMIT_PIN))&&
			(0 == motor_control_obj->forward_limit_flag))
	{
			motor_control_obj->forward_limit_flag = 1;
		  MOTOR_EN(1);
	}
	if( (1 == motor_control_obj->forward_limit_flag ) && (1 == get_gpio_input_value(FORWARD_LIMIT_GROUP,FORWARD_LIMIT_PIN)) )
	{
			motor_control_obj->forward_limit_flag = 0;
	}
}

 void exti_backward_handle(Motor_t * motor_control_obj)
 {
	 if( (0 == get_gpio_input_value(BACKWARD_LIMIT_GROUP,BACKWARD_LIMIT_PIN) ) &&
			(0 == motor_control_obj->backward_limit_flag ) )
	{
			motor_control_obj->backward_limit_flag = 1;
		  MOTOR_EN(1);
	}
	if( (1 == motor_control_obj->backward_limit_flag ) && 
			(1 == get_gpio_input_value(BACKWARD_LIMIT_GROUP,BACKWARD_LIMIT_PIN) ) )
	{
			motor_control_obj->backward_limit_flag = 0;	
	}
 }

void EXTI15_10_IRQHandler()
{
    if(EXTI_GetITStatus(EXTI_Line13)!= RESET){
				MOTOR_EN(1);        // 使能电机，停止电机运行
				Move_End = 1; 
				//exti_forward_handle(&Motor);
				send_flag = 0;
        EXTI_ClearITPendingBit(EXTI_Line13);
    }
    
    if(EXTI_GetITStatus(EXTI_Line14)!= RESET){
				MOTOR_EN(1);
				Move_End = 1; 
				//exti_backward_handle(&Motor);
				send_flag = 0;
        EXTI_ClearITPendingBit(EXTI_Line14);
    }
}



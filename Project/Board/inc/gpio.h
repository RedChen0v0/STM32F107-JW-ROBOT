// coding UTF8


/**
* Copyright (c) 2021 Hefei Xiaobu Technologies Co., LTD. All rights reserved
*/
#ifndef _GPIO_H_
#define _GPIO_H_

//使用那个定义那个为1
#define RS485_1 1
#define RS485_2 1

typedef enum
{
    bd_gpio_group_no = 0,
    bd_gpio_group_a = 1,
    bd_gpio_group_b ,
    bd_gpio_group_c ,
    bd_gpio_group_d ,
    bd_gpio_group_e ,
    bd_gpio_group_g
  
} bd_gpio_group;


typedef enum
{
    bd_gpio_pin_no = 0,
    bd_gpio_pin_0 = 1,
    bd_gpio_pin_1,
    bd_gpio_pin_2,
    bd_gpio_pin_3,
    bd_gpio_pin_4,
    bd_gpio_pin_5,
    bd_gpio_pin_6,
    bd_gpio_pin_7,
    bd_gpio_pin_8,
    bd_gpio_pin_9,
    bd_gpio_pin_10,
    bd_gpio_pin_11,
    bd_gpio_pin_12,
    bd_gpio_pin_13,
    bd_gpio_pin_14,
    bd_gpio_pin_15
} bd_gpio_pin;


typedef enum
{
    bd_gpio_mode_ain = 0,
    bd_gpio_mode_in_floating,
    bd_gpio_mode_ipd,
    bd_gpio_mode_ipu,
    bd_gpio_mode_out_od,
    bd_gpio_mode_out_pp,
    bd_gpio_mode_af_od,
    bd_gpio_mode_af_pp
    
} bd_gpio_mode;

typedef enum
{
    bd_gpio_speed_10mhz = 1,
    bd_gpio_speed_2mhz,
    bd_gpio_speed_50mhz
} bd_gpio_speed;



typedef enum
{
    bd_exti_line_0 = 0,
    bd_exti_line_1,
    bd_exti_line_2,
    bd_exti_line_3,
    bd_exti_line_4,
    bd_exti_line_5,
    bd_exti_line_6,
    bd_exti_line_7,
    bd_exti_line_8,
    bd_exti_line_9,
    bd_exti_line_10,
    bd_exti_line_11,
    bd_exti_line_12,
    bd_exti_line_13,
    bd_exti_line_14,
    bd_exti_line_15,
    bd_exti_line_16,
    bd_exti_line_17,
    bd_exti_line_18,
    bd_exti_line_19
} bd_exti_line;

typedef enum
{
    bd_exti_mode_interrupt = 1,
    bd_exti_mode_event,

} bd_exti_mode;

typedef enum
{
    bd_exti_trigger_rising = 1,
    bd_exti_trigger_falling,
    bd_exti_trigger_rising_falling

} bd_exti_trigger;

#define FORWARD_LIMIT_PIN     bd_gpio_pin_13   //前进限位管脚
#define BACKWARD_LIMIT_PIN    bd_gpio_pin_14  //后退限位管脚

#define FORWARD_LIMIT_GROUP   bd_gpio_group_c
#define BACKWARD_LIMIT_GROUP  bd_gpio_group_c


void gpio_config(void);

void set_gpio_config(bd_gpio_group group,  bd_gpio_pin pin, bd_gpio_mode mode ,bd_gpio_speed speed);

void set_gpio_value(bd_gpio_group group, bd_gpio_pin pin, char val);

char get_gpio_output_value(bd_gpio_group group, bd_gpio_pin pin );
char get_gpio_input_value(bd_gpio_group group, bd_gpio_pin pin );

void set_exti_config(bd_exti_line line ,bd_exti_mode mode ,bd_exti_trigger tigger);


typedef void (*gpio_callback_irq)( bd_gpio_group group,  bd_gpio_pin pin,  int type );
void register_gpio_irq_callback(gpio_callback_irq func);

#endif

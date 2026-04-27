/**
* Copyright (c) 2021 Hefei Xiaobu Technologies Co., LTD. All rights reserved
*/
#include "stm32f10x.h"
#include <system_stm32f10x.h>
#include <stdio.h> 
#include <stdint.h>
#include "app_func.h"

Motor_t Motor;


int main(void)
{
    SystemInit();                       // 初始化系统时钟
    gpio_config();                      // 初始化GPIO
    sys_tick_init(SYS_FREQ);            // 初始化系统滴答定时器，计数初始化
    USART3_Init(UART3_BAUD);            // 初始化串口3，波特率115200
    USART1_RS485_Init(UART1_BAUD);      // 初始化485usart1，波特率9600
    MOTOR_EN(ENA);                      // 使能电机引脚
    OLED_Init();                        // OLED显示初始化
    OLED_Clear();                       // OLED显示清屏
    Motor_Init();                       // 电机控制结构体初始化
/*--------------------------------------------------------------------------
    以上为全部外设初始化代码
----------------------------------------------------------------------------*/
    printf("system init completed\n");  // 系统初始化完成
    oledInitprint();                    // OLED显示初始化完成
    showLOGO();                         // OLED显示LOGO

    uint16_t dist = 0;                  // 超声波测距数据
    uint16_t temp = 0;                  // 温度数据
    uint16_t fmeCnt = 1;                // 电机运动状态显示计数
    uint8_t showcmd = 0;                // OLED显示内容控制, 0显示LOGO，1显示数据，2显示菜单
    uint8_t oldshowcmd  = showcmd;      // OLED上一次显示的类型
    delay_ms(500);                      // 等待500ms，确保OLED显示初始化完成
    Motor_Start(&Motor);                // 电机开始工作


    while(1)
    {
        Motor_Check_Status(&Motor);     // 校验电机状态，超声波检测停止
        
        if(getUART3data()){             // 界面隔离：用户必须跳转到MENU界面输入对应的操作才能跳到数据或LOGO界面，不能直接跳到数据或LOGO界面
            /*----------------------------------------------------------
            LOGO界面：输入任意值（包括"LOGO", "CTRL"等）进入MENU
            -----------------------------------------------------------*/
            if (showcmd == OLED_LOGO) {
                showcmd = OLED_MENU;
            }
            /*----------------------------------------------------------
            MENU界面：输入DATA进入数据界面，输入LOGO进入LOGO界面，输入CTRL进入CTRL界面
            -----------------------------------------------------------*/
            else if (showcmd == OLED_MENU) {
                if (strcmp("DATA", USART3_RxBuffer) == 0) {
                    showcmd = OLED_DATA;
                }
                else if (strcmp("LOGO", USART3_RxBuffer) == 0) {
                    showcmd = OLED_LOGO;
                }
                else if (strcmp("CTRL", USART3_RxBuffer) == 0) {
                    showcmd = OLED_CTRL;
                }
                // 其他输入在MENU下被视为无效输入
            }
            /*----------------------------------------------------------
            CTRL界面：输入STOP停止电机，输入RUN恢复电机，操作后停留在CTRL，输入MENU返回MENU
            -----------------------------------------------------------*/
            else if (showcmd == OLED_CTRL) {
                if (strcmp("STOP", USART3_RxBuffer) == 0) {
                    Motor_Stop();
                    Motor_Set_Menual_Stop();
                    // 操作后不返回MENU，停留在CTRL界面
                }
                else if (strcmp("RUN", USART3_RxBuffer) == 0) {
                    Motor_Resume();
                    Motor_Set_Menual_Run();
                    // 操作后不返回MENU，停留在CTRL界面
                }
                else if (strcmp("MENU", USART3_RxBuffer) == 0) {
                    showcmd = OLED_MENU;
                }
                // 其他输入在CTRL下被被视为无效输入
            }
            /*----------------------------------------------------------
            DATA界面：输入MENU返回MENU
            -----------------------------------------------------------*/
            else if (showcmd == OLED_DATA ) {
                // 只有输入"MENU"才能返回MENU
                if (strcmp("MENU", USART3_RxBuffer) == 0) {
                    showcmd = OLED_MENU;
                }
                // 其他输入被忽略
            }

            flashUART3(); // 刷新接收的缓冲区USART3_RxBuffer
        }
        
        if(fmeCnt <= 5){
            Led_1_on();
            Led_2_off();
        }else{
            Led_1_off();
            Led_2_on();
        }

        if(fmeCnt % 2 == 0){
            dist = get_distence();              // 100ms
        }else{
            temp = get_tempture();              // 100ms
        }

/*--------------------------------------------------------------------------
    OLED显示控制，接收串口指令切换显示内容
----------------------------------------------------------------------------*/
        switch (showcmd)    // OLED显示内容控制
        {
        case OLED_LOGO:
            if(oldshowcmd != showcmd){
                showLOGO();
            }
            oldshowcmd = showcmd;
            break;
        case OLED_DATA:
            if(oldshowcmd != showcmd){          // 只有当显示内容切换时才刷新显示，避免重复刷新导致闪烁
                showFRAME();                    // OLED显示菜单框架
            }
            printnum2OLEDln("dist", dist);      // OLED显示距离数据
            printnum2OLEDln("temp", temp);      // OLED显示温度数据
            oldshowcmd = showcmd;
            break;
        case OLED_MENU:
            if(oldshowcmd != showcmd){
                showMENU();
            }
            oldshowcmd = showcmd;
            break;
        case OLED_CTRL:
            if(oldshowcmd != showcmd){
                showCTRL();
            }
            oldshowcmd = showcmd;
            break;
        default: 
            break;
        }

        fmeCnt++;
        if(fmeCnt > 10){
            fmeCnt = fmeCnt % 11 + 1;
        }

    }
}

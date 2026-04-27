/**
* Copyright (c) 2021 Hefei Xiaobu Technologies Co., LTD. All rights reserved
*/
#include "stm32f10x.h"
#include <system_stm32f10x.h>
#include <stdio.h> 
#include <stdint.h>
#include "app_func.h"

Motor_t Motor;

enum Point_Dist{
    Point1_Dist = 300,
    Point2_Dist = 400,
    Point3_Dist = 500,
};

enum Point_Num{
    Point1 = 0,
    Point2 = 1,
    Point3 = 2,
};

float Point1_temp[100];
float Point2_temp[100];
float Point3_temp[100];

uint8_t Point1_times = 0;
uint8_t Point2_times = 0;
uint8_t Point3_times = 0;

extern uint8_t led_flag; 
extern uint8_t send_flag;

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
    uint32_t Point_Dist = Point1_Dist;  // 默认巡检点1位置
    uint8_t Point_Num = Point1;         // 当前巡检点编号
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
            MENU界面：输入DATA进入数据界面，输入LOGO进入LOGO界面，输入CTRL进入CTRL界面，输入POINT进入POINT界面
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
                else if (strcmp("POINT", USART3_RxBuffer) == 0) {
                    showcmd = OLED_POINT;
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
            POINT界面：输入POINT1,POINT2,POINT3选择巡检点，输入MENU返回MENU
            -----------------------------------------------------------*/          
            else if (showcmd == OLED_POINT) {
                if (strcmp("POINT1", USART3_RxBuffer) == 0) {
                    Point_Num = Point1;
                    Point_Dist = Point1_Dist;
                    // 操作后不返回MENU，停留在POINT界面
                }
                else if (strcmp("POINT2", USART3_RxBuffer) == 0) {
                    Point_Num = Point2;
                    Point_Dist = Point2_Dist;
                    // 操作后不返回MENU，停留在POINT界面
                }
                else if (strcmp("POINT3", USART3_RxBuffer) == 0) {
                    Point_Num = Point3;
                    Point_Dist = Point3_Dist;
                    // 操作后不返回MENU，停留在POINT界面
                }
                else if (strcmp("MENU", USART3_RxBuffer) == 0) {
                    showcmd = OLED_MENU;
                }
                // 其他输入在POINT下被被视为无效输入
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
        
        if(led_flag == 1){
            if(fmeCnt <= 5){
                Led_1_on();
                Led_2_off();
            }else{
                Led_1_off();
                Led_2_on();
            }
        }
        else{
            Led_1_off();
            Led_2_off();
        }


        if(fmeCnt % 2 == 0){
            dist = get_distence();              // 100ms
        }else{
            temp = get_tempture();              // 100ms
        }


        if(send_flag == 0){
            if(dist > Point_Dist-70 && dist < Point_Dist+70){ 
                Motor_Speed(MIN_SPEED);
            }
            if(dist > Point_Dist-15 && dist < Point_Dist+15){
                Motor_Stop(); 
                Motor_Set_Menual_Stop();
        
                showFRAME(); 
                printnum2OLEDln("dist", dist);
                printnum2OLEDln("temp", temp);
        
                // printf("temp: %.1f C\r\n", (float)temp / 10);
                // 根据当前 Point_Num 存入对应数组，并更新对应计数器
                if (Point_Num == Point1 && Point1_times < 100) {
                    Point1_temp[Point1_times] = (float)temp / 10;
                    Point1_times++;
                }
                else if (Point_Num == Point2 && Point2_times < 100) {
                    Point2_temp[Point2_times] = (float)temp / 10;
                    Point2_times++;
                }
                else if (Point_Num == Point3 && Point3_times < 100) {
                    Point3_temp[Point3_times] = (float)temp / 10;
                    Point3_times++;
                }
        
                // 打印当前巡检点的全部历史温度值
                if (Point_Num == Point1) {
                    for (int i = 0; i < Point1_times; i++) {
                        printf("Point1_times%d_temp: %.1f C\r\n", i + 1, Point1_temp[i]);
                    }
                }
                else if (Point_Num == Point2) {
                    for (int i = 0; i < Point2_times; i++) {
                        printf("Point2_times%d_temp: %.1f C\r\n", i + 1, Point2_temp[i]);
                    }
                }
                else if (Point_Num == Point3) {
                    for (int i = 0; i < Point3_times; i++) {
                        printf("Point3_times%d_temp: %.1f C\r\n", i + 1, Point3_temp[i]);
                    }
                }
                Motor_Speed(MAX_SPEED);
                delay_ms(500); 
                showLOGO();
                showcmd = OLED_LOGO;    // 同步状态
                oldshowcmd = OLED_LOGO; // 避免下次重复刷屏
                send_flag = 1;
                Motor_Set_Menual_Run();
            }
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
        case OLED_POINT:
            if(oldshowcmd != showcmd){
                showPOINT();
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

#include "app_func.h"
#include "bmp.h"
/*************************************************************************
电机控制
*************************************************************************/
/*----------------------------------------------------------
stopThre         超声波检测停止距离阈值
resume_flag      超声波检测消抖
-----------------------------------------------------------*/
uint8_t stopThre = 48;
static uint8_t resume_flag = 0;
uint8_t menual_stop = 0;    // 把停止标志位再细分了一种情况手动停止还是避障停止
uint8_t led_flag =  0;      // LED状态标志位，1表示检测到障碍物，0表示没有检测到障碍物
/*----------------------------------------------------------
函数描述： 设置电机运动方向
输入参数：
类型： Motor_Dir_e {FORWARD 向前、 BACKWARD 向后}
返回值： void
-----------------------------------------------------------*/
void MOTOR_Dir(Motor_Dir_e val)  //电机方向
{
    set_gpio_value(bd_gpio_group_e,bd_gpio_pin_12,val);
}
/*----------------------------------------------------------
函数描述： 设置电机使能
输入参数： 类型： Motor_En_e {ENA 使能、 DIS 停止}
返回值： void
-----------------------------------------------------------*/
void MOTOR_EN(Motor_En_e val)  //电机使能 PB11引脚
{ 
    set_gpio_value(bd_gpio_group_b,bd_gpio_pin_11,val);
}
/*----------------------------------------------------------
函数描述： 初始化电机
输入参数： 类型： void
返回值： void
-----------------------------------------------------------*/
void Motor_Init(void)
{
    if(get_gpio_input_value(bd_gpio_group_c, bd_gpio_pin_14) == 1){
        Motor.Motor_Dir = FORWARD;
    } else {
        Motor.Motor_Dir = BACKWARD;
    }
    Motor.Motor_En = ENA;
    Motor.Pulse_Count = 0;
    Motor.Speed = MAX_SPEED;
    Motor.Pulse_Max = PULSE_NUM;
    Motor.Motor_Flag = 0; 
    TIM5_PWM_Init(TIM5_ARR, TIM5_PSC);
}
/*----------------------------------------------------------
函数描述： 设置电机速度
输入参数： 类型： uint16_t speed（速度值）
返回值： void
-----------------------------------------------------------*/
void Motor_Speed(uint16_t speed)
{
    TIM_SetCompare1(TIM5, (TIM_AFTER_PRESCALER_FRENQ>>1)/speed);//  设置占空比为50%
    TIM_SetAutoreload(TIM5,TIM_AFTER_PRESCALER_FRENQ/speed - 1);//  设置自动重装载值为速度值的倒数减1
}
/*----------------------------------------------------------
函数描述： 电机开始运行
输入参数： 类型： Motor_t *Motor（电机控制结构体）
返回值： void
-----------------------------------------------------------*/
void Motor_Start(Motor_t *Motor)
{
    MOTOR_Dir(Motor->Motor_Dir);
    MOTOR_EN(Motor->Motor_En);
    Motor_Speed(Motor->Speed);
    TIM_Cmd(TIM5, ENABLE);
    Move_Stop = 0;
}
/*----------------------------------------------------------
函数描述： 设置电机方向
输入参数：
    类型： Motor_Dir_e {FORWARD 向前、 BACKWARD 向后}
    类型： Motor_t *Motor（电机控制结构体）
返回值： void
-----------------------------------------------------------*/
void Motot_Set_Dir(Motor_Dir_e val, Motor_t *Motor){
    Motor->Motor_Dir = val;
}
/*----------------------------------------------------------
函数描述： 反转电机方向
输入参数：类型： Motor_t *Motor（电机控制结构体）
返回值： void
-----------------------------------------------------------*/
void Motor_Change_Dir(Motor_t *Motor)
{
    if(Motor->Motor_Dir == FORWARD){
        Motot_Set_Dir(BACKWARD, Motor);
    }else{
        Motot_Set_Dir(FORWARD, Motor);
    }
}
/*----------------------------------------------------------
函数描述： 停止电机运行
输入参数：类型： void
返回值： void
-----------------------------------------------------------*/
void Motor_Stop(){
    TIM_Cmd(TIM5, DISABLE);
    Move_Stop = 1;
}
/*----------------------------------------------------------
函数描述： 恢复电机运行
输入参数：类型： void
返回值： void
-----------------------------------------------------------*/
void Motor_Resume(){
    TIM_Cmd(TIM5, ENABLE);
    Move_Stop = 0;
}
/*----------------------------------------------------------
函数描述： 串口设置手动停止
输入参数：类型： void
返回值： void
-----------------------------------------------------------*/
void Motor_Set_Menual_Stop(){
    menual_stop = 1;
}
/*----------------------------------------------------------
函数描述： 串口设置手动重启
输入参数：类型： void
返回值： void
-----------------------------------------------------------*/
void Motor_Set_Menual_Run(){
    menual_stop = 0;
}
/*----------------------------------------------------------
函数描述： 校验电机状态，超声波检测停止
输入参数：类型： void
返回值： void
-----------------------------------------------------------*/
void Motor_Check_Status(Motor_t *Motor)
{
//     if(Move_End == 1){
//     // 检查限位开关是否已释放，只有释放后才允许换向
//     uint8_t forward_limit_released = (get_gpio_input_value(FORWARD_LIMIT_GROUP, FORWARD_LIMIT_PIN) == 1);
//     uint8_t backward_limit_released = (get_gpio_input_value(BACKWARD_LIMIT_GROUP, BACKWARD_LIMIT_PIN) == 1);
    
//     // 只有当触发换向的限位开关释放后才执行换向
//     uint8_t can_change_dir = 0;
    
//     // 判断是哪个限位开关触发的换向
//     if(Motor->Motor_Dir == FORWARD && backward_limit_released){
//         can_change_dir = 1;  // 之前是前进，检测后退限位是否释放
//     }
//     else if(Motor->Motor_Dir == BACKWARD && forward_limit_released){
//         can_change_dir = 1;  // 之前是后退，检测前进限位是否释放
//     }
    
//     if(can_change_dir){
//         Move_End = 0;
//         Motor_Change_Dir(Motor);    // 改变电机方向
//         Motor->Motor_En = ENA;      // 电机状态改为使能
//         Motor_Start(Motor);         // 重新启动电机
//     }
// }
    
    if(Move_End == 1){
        Move_End = 0;
        Motor_Change_Dir(Motor);    // 改变电机方向
        Motor->Motor_En = ENA;      // 电机状态改为使能
        Motor_Start(Motor);         // 重新启动电机
    }
    // if(get_gpio_input_value(bd_gpio_group_c, bd_gpio_pin_14) == 0){
    //     Motor->Motor_Dir = BACKWARD;
    // } 
    // if(get_gpio_input_value(bd_gpio_group_c, bd_gpio_pin_13) == 0){
    //     Motor->Motor_Dir = FORWARD;
    // }
    if(RADAR_RX_DIST != 0 && RADAR_RX_DIST <= stopThre){    // 加!0的原因：超声波还没有检测障碍物的时候，接收数据为0的情况
        Motor_Stop();                                       // 立刻停止电机，无延迟
        resume_flag = 0;                                    // 防止跳变、跳变、有障碍物、跳变的情况
        led_flag = 1;                                       // 打开LED灯，表示检测到障碍物
    }else if(Move_Stop == 1 && menual_stop == 0){           // 电机停止，检测三次确认没有障碍物(消抖)后恢复运行
        resume_flag++;                                      // 加&& menual_stop == 0是为了确保不是"手动停止"产生的停止，把两种情况隔离了
        if(resume_flag > 2){
            resume_flag = 0;
            Motor_Resume();
            led_flag = 0;                                   // 关闭LED灯，表示没有检测到障碍物
        }
    }
}
/*************************************************************************
OLED控制
*************************************************************************/
/*----------------------------------------------------------
uint8_t linenum      希望oled显示内容所在的行数
char showstr[17]     存放oled输出内容，一行16个非中文字符
char shownumstr[17]  用来拼接需要显示数据的内容
uint8_t showstatus   标识是否处于显示logo的状态 0 不是 1 是
-----------------------------------------------------------*/
static uint8_t linenum = 0;
char showstr[17];
char shownumstr[17];
static uint8_t showstatus = 0;
/*----------------------------------------------------------
函数描述： 输送到oled显示字符，再次调用会自动显示换行
输入参数：类型： char* str （要显示的字符，只显示16个，超过内容不显示）
返回值： void
-----------------------------------------------------------*/
void print2OLEDln(const char* str)
{
    if(showstatus == 1){
        OLED_Clear();
        showstatus = 0;
    } else {
        for(int i = 0; i < 16; i++){
            showstr[i] = ' ';
        }
    }
    OLED_ShowString(0, linenum * 2, showstr, 16);
    memcpy(showstr, str, 16);
    OLED_ShowString(0, linenum * 2, showstr, 16);
    linenum++;
    linenum = linenum % 4;
}
/*----------------------------------------------------------
函数描述： 输送到oled显示字符，后接数据，再次调用会自动显示换行，
          居住中显示，只显示在第二第三行
输入参数：
    类型： char* str （要显示的字符串）
    类型： uint16_t num （要显示的数字，与字符串一共只显示16个，超过内容不显示）
返回值： void
-----------------------------------------------------------*/
void printnum2OLEDln(const char* str, uint16_t num)
{
    if(linenum == 0 || linenum == 3){
        linenum = 1;
    }
    if(showstatus == 1){
        OLED_Clear();
        showstatus = 0;
    } else {
        for(int i = 0; i < 16; i++){
            shownumstr[i] = ' ';
        }
    }
    OLED_ShowString(0, linenum * 2, shownumstr, 16);
    memset(shownumstr, 0, 16);
    sprintf(shownumstr,"%s:%d", str, num);
    OLED_ShowString(0, linenum * 2, shownumstr, 16);
    linenum++;
    linenum = linenum % 4;
}
/*----------------------------------------------------------
函数描述： 显示LOGO：青鸟
输入参数：类型： void
返回值： void
-----------------------------------------------------------*/
void showLOGO()
{
    OLED_Clear();
    OLED_DrawBMP(0,0,128,8,BMP2);
    showstatus = 1;
}
/*----------------------------------------------------------
函数描述： 显示菜单框架
输入参数：类型： void
返回值： void
-----------------------------------------------------------*/
void showFRAME()
{
    OLED_Clear();   // 清屏
    showstatus = 0; // 展示LOGO的状态标志位清零
    linenum = 0;    // 设置第一行
    OLED_ShowString(0, linenum * 2, "******MENU******", 16);
    linenum = 3;    // 设置最后一行
    OLED_ShowString(0, linenum * 2, "****************", 16);
}
/*----------------------------------------------------------
函数描述： 显示主菜单
输入参数：类型： void
返回值： void
-----------------------------------------------------------*/
void showMENU()
{
    showFRAME();
    linenum = 1;
    OLED_ShowString(0, linenum * 2, "*LOGO", 16);   // 为什么是 linenum*2：字符占两行，用第一行的y轴坐标
    OLED_ShowString(87, linenum * 2, "*CRTL", 16);  // 为什么是16，一个字符像素点：宽8，高2，8X2=16，整个屏幕最大坐标(7,127)
    linenum = 2;
    OLED_ShowString(0, linenum * 2, "*DATA", 16);
    OLED_ShowString(79, linenum * 2, "*POINT", 16);
    linenum = 0;
}
/*----------------------------------------------------------
函数描述： 显示控制菜单
输入参数：类型： void
返回值： void
-----------------------------------------------------------*/
void showCTRL()
{
    showFRAME();
    linenum = 1;
    OLED_ShowString(0, linenum * 2, "*STOP", 16);
    OLED_ShowString(87, linenum * 2, "*TURN", 16);
    linenum = 2;
    OLED_ShowString(0, linenum * 2, "*RUN", 16);
    linenum = 0;
}
/*----------------------------------------------------------
函数描述： 显示巡检点菜单
输入参数：类型： void
返回值： void
-----------------------------------------------------------*/
void showPOINT()
{
    showFRAME();
    linenum = 1;
    OLED_ShowString(0, linenum * 2, "*POINT1", 16);
    OLED_ShowString(71, linenum * 2, "*POINT2", 16);
    linenum = 2;
    OLED_ShowString(0, linenum * 2, "*POINT3", 16);
    linenum = 0;
}
/*----------------------------------------------------------
函数描述： 显示初始化内容
输入参数：类型： void
返回值： void
-----------------------------------------------------------*/
void oledInitprint()
{
    print2OLEDln("system init");
    delay_ms(1000);
    print2OLEDln("status update");
    delay_ms(1000);
    print2OLEDln("running...");
    delay_ms(1000);
    print2OLEDln("success");
    delay_ms(1000);
    OLED_Clear();
    print2OLEDln("showing logo");
    delay_ms(2000);
    showLOGO();
}
/*************************************************************************
LED控制
*************************************************************************/
/*----------------------------------------------------------
函数描述： 打开LED1
输入参数：类型： void
返回值： void
-----------------------------------------------------------*/
void Led_1_on(void){
    set_gpio_value(bd_gpio_group_e, bd_gpio_pin_3, 0);
}
/*----------------------------------------------------------
函数描述： 打开LED2
输入参数：类型： void
返回值： void
-----------------------------------------------------------*/
void Led_2_on(void){
    set_gpio_value(bd_gpio_group_e, bd_gpio_pin_4, 0);
}
/*----------------------------------------------------------
函数描述： 关闭LED1
输入参数：类型： void
返回值： void
-----------------------------------------------------------*/
void Led_1_off(void){
    set_gpio_value(bd_gpio_group_e, bd_gpio_pin_3, 1);
}
/*----------------------------------------------------------
函数描述： 关闭LED2
输入参数：类型： void
返回值： void
-----------------------------------------------------------*/
void Led_2_off(void){
    set_gpio_value(bd_gpio_group_e, bd_gpio_pin_4, 1);
}
/*************************************************************************
485总线操作//超声波//温度
*************************************************************************/
/*----------------------------------------------------------
函数描述： 获取超声波数据
输入参数：类型： void
返回值： uint16_t
-----------------------------------------------------------*/
uint16_t get_distence(){
    uint16_t dist = 0;
    if(!xUSART.USART1_Busy)
    {
        USART1_RS485_RW_Opr(RADAR_ADDR,READ,0x0101,0x0001); // read distance data
    }
    delay_ms(100);  // 外部中断足够时间接收数据，并完成标志设置
    if(xUSART.USART1_RX_FLAG)   // wait for data
    {
        xUSART.USART1_RX_FLAG = 0;
        xUSART.USART1_Busy = 0;
        dist = RADAR_RX_DIST;
    }
    return dist;
}
/*----------------------------------------------------------
函数描述： 获取温度数据
输入参数：类型： void
返回值： uint16_t
-----------------------------------------------------------*/
uint16_t get_tempture(){
    uint16_t temp;
    if(!xUSART.USART1_Busy)
    {
        USART1_RS485_RW_Opr(RADAR_ADDR,READ,0x0102,0x0001); // read temperature data
    }
    delay_ms(100);
    if(xUSART.USART1_RX_FLAG)
    {
        xUSART.USART1_RX_FLAG = 0;
        xUSART.USART1_Busy = 0;
        temp = RADAR_RX_TEMP;
    }
    return temp;
}
/*************************************************************************
UART3串口操作
*************************************************************************/
/*----------------------------------------------------------
函数描述： 获取USART3串口接收数据，并于串口回传显示
输入参数：类型： void
返回值： uint8_t 有接收数据 1 无接收数据 0，数据存放位置USART3_RxBuffer
-----------------------------------------------------------*/
uint8_t getUART3data(void){
    if(USART3_RxFinished == 1){
        USART3_RxFinished = 0; 
        printf("uart3_rcv = %s\r\n", USART3_RxBuffer);
        DMA_SetCurrDataCounter(DMA1_Channel3, USART3_RX_BUFFER_SIZE);// 刷新DMA数据传输计数器
        DMA_Cmd(DMA1_Channel3, ENABLE);// DMA重新使能，开始接收数据
        return 1;
    }
    return 0;
}
/*----------------------------------------------------------
函数描述： 刷新USART3串口接收存储数组
输入参数：类型： void
返回值： void
-----------------------------------------------------------*/
void flashUART3(void){
    memset(USART3_RxBuffer, 0, USART3_RX_BUFFER_SIZE);
}

uint8_t chr2num(char c){
}
/*************************************************************************
w5500网络操作
*************************************************************************/
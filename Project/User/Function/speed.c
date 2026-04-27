#include "speed.h"
#include "stdio.h"
#include "math.h"
//#include "data.h"

Motor_t Motor;

//extern volatile uint16_t AccStep[ACC_STEP_NUM];
uint16_t AccStep[ACC_STEP_NUM]={0};


//功能：S加速曲线初始化
//参数1 *pbuff          计算出的定时器的周期
//参数2 fre_max        最大频率 Hz
//参数3 fre_min        最小频率 Hz
//参数4 len            加速需要的脉冲数
void CurveS_init(uint16_t *pbuff,uint32_t fre_max,uint32_t fre_min,int16_t len)
{
	int16_t i;
	uint16_t flexible =4;
	float delt = fre_max-fre_min;
	float deno ;
	float melo ;
	float fre;

	for(i=0; i<len; i++)
	{
		melo = flexible* (i-len/2) / (len/2);
		deno = 1.0f / (1 + expf(-melo));  //
		fre = delt * deno + fre_min;
		pbuff[i] = (unsigned short)(TIM5_CLOCK_FREQ / fre);
		printf("pbuff[%d]:%d\r\n,",i,pbuff[i]);
	}

}
//速度调节函数
volatile uint8_t RUN_FLAG = 0;
void SpeedAdjust(void)
{
  switch(Motor.Status)
  {
    /*加速*/            
    case SPEED_INCREASE:
      if(Motor.ACC_STEP_Count < Motor.ACC_Count_Max)
      {
				TIM5->ARR = AccStep[Motor.ACC_STEP_Count];         //计算下一个PWM的周期
				TIM5->CCR1 = AccStep[Motor.ACC_STEP_Count]/2;      //占空比50%
				Motor.ACC_STEP_Count++;                            //加速次数
      }
      else
      {
        Motor.Status = SPEED_STABLE; 
        Motor.ACC_STEP_Count--;
      }
    break;
    /*匀速*/  
    case SPEED_STABLE:
      if(Motor.Pulse_Count >= (Motor.Pulse_Max - Motor.ACC_STEP_Count))
      {    
        Motor.Status = SPEED_DECREASE;
      }
    break;
    /*减速*/  
    case SPEED_DECREASE:
      if(Motor.ACC_STEP_Count >= 0)
      {
				TIM5->ARR = AccStep[Motor.ACC_STEP_Count];  //计算下一个PWM的周期
				TIM5->CCR1 = AccStep[Motor.ACC_STEP_Count]/2;//占空比50%
        Motor.ACC_STEP_Count--;  
      }      
     break;
			
			/*停止*/  
		case SPEED_STOP:
			if(Motor.ACC_STEP_Count >= 1)
			{

				TIM5->ARR = AccStep[Motor.ACC_STEP_Count];  //计算下一个PWM的周期
				TIM5->CCR1 = AccStep[Motor.ACC_STEP_Count]/2;//占空比50%
        Motor.ACC_STEP_Count--;  
			}
			else{TIM_Cmd(TIM5, DISABLE);}
    
		default :
    break;
  }
}

//PWM--需要输出的脉冲个数
void Motor_Start(uint32_t pulse_num)
{      
	Motor.Pulse_Count = 0;
	Motor.Pulse_Max = pulse_num;
	Motor.ACC_STEP_Count = 0;
	Motor.Status = SPEED_INCREASE;
	Motor.ACC_Count_Max = ACC_STEP_NUM;
	//初始化加速曲线
	CurveS_init(AccStep,MAX_SPEED,MIN_SPEED,Motor.ACC_Count_Max);
	TIM5->ARR = AccStep[0];
	TIM5->CCR1 = AccStep[0]/2;

	TIM_Cmd(TIM5, ENABLE);
}


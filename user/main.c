#include "stm32f10x.h"
#include "bat_test.h"
#include "app_bat.h"
#include "app_button.h"
#include "app_pwm.h"
#include "pwm_test.h"
#include "encoder_test.h"
#include "mpu6050_test.h"
#include "app_encoder.h"
#include "app_motor.h"
#include "delay.h"
#include "app_mpu6050.h"
#include "app_control.h"

int main(void)	
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); // 配置中断优先级分组为0

//	MPU6050_EularAngleTest();
//	MPU6050_Test();
//	Encoder_T_Method_Test();
//	Encoder_M_Method_Test();
//	Bat_Test();
//	PWM_Test();
//	Encoder_Test();

	App_Button_Init(); //按钮，用于控制电机的启停
	App_Encoder_Init(); //编码器，用于测量电机的转速
	App_PWM_Init(); //PWM，用于控制电机的转速，TB6612FNG，双H桥
	App_Bat_Init(); //电池电压监控
	App_Motor_Init(); //电机调速系统(PID控制)
	App_MPU6050_Init(); //MPU6050，用于测量平衡车的倾斜角度和角速度
	App_Control_Init(); //平衡车控制系统

	while(1)
	{
		App_Button_Proc();
		App_Bat_Proc();
		App_Motor_Proc();
		App_MPU6050_Proc();
		App_Control_Proc();
	}
}

//
//
//


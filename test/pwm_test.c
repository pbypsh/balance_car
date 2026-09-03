#include "pwm_test.h"
#include "app_pwm.h"
#include "delay.h"

//
// @简介：PWM测试
//       先使能电机
//       让电机正转30%2秒
//       让电机正转60%2秒
//       让电机正转90%2秒
//       关闭电机
//
void PWM_Test(void)
{
    App_PWM_Init(); // 初始化TB6612

    App_PWM_Cmd(1); // 使能电机

    App_PWM_Set_L(30); // 左电机占空比为30%
    App_PWM_Set_R(30); // 右电机占空比为30%

    Delay(2000); // 延时2秒

    App_PWM_Set_L(60); // 左电机占空比为60%
    App_PWM_Set_R(60); // 右电机占空比为60% 

    Delay(2000); // 延时2秒 

    App_PWM_Set_L(90); // 左电机占空比为90%
    App_PWM_Set_R(90); // 右电机占空比为90% 

    Delay(2000); // 延时2秒  

    App_PWM_Cmd(0); // 关闭电机

    while(1)
    {
    App_PWM_Cmd(1); // 使能电机
    App_PWM_Set_L(30.0f); // 左电机正转30%
    App_PWM_Set_R(60.0f); // 右电机正转60%
    App_PWM_Set_L(90.0f); // 左电机正转90%
    App_PWM_Set_R(0.0f); // 右电机正转0%
    App_PWM_Cmd(0); // 关闭电机
    }
}

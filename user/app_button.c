#include "app_button.h"
#include "button.h"
#include "app_motor.h"
#include "app_control.h"    

static Button_TypeDef userKey; //用户按键

static void OnUserKey_ClickCb(uint8_t clicks);

//
// @简介：初始化按键驱动
//
void App_Button_Init(void)
{
    Button_InitTypeDef Button_InitStruct = {0};

    Button_InitStruct.GPIOx = GPIOA;
    Button_InitStruct.GPIO_Pin = GPIO_Pin_11; 

    My_Button_Init(&userKey, &Button_InitStruct);

    My_Button_SetClickCb(&userKey, OnUserKey_ClickCb);
}

void App_Button_Proc(void)
{
    My_Button_Proc(&userKey);
}

static uint8_t pwm_on = 0; // 0 - 电机休眠，1 - 电机活动
//
// @简介：用户按键点击事件回调函数
//
static void OnUserKey_ClickCb(uint8_t clicks)
{
    if (clicks == 1) // 单击事件处理
    {
        App_Control_Reset();

        // 翻转电机状态
        if (pwm_on == 0)
        {
            pwm_on = 1; // 电机活动
        }
        else
        {
            pwm_on = 0; // 电机休眠
        }

        App_Motor_Cmd(pwm_on); // 控制电机状态
       }
}

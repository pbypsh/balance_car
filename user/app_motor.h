#ifndef APP_MOTOR_H
#define APP_MOTOR_H

#include "stm32f10x.h"

void App_Motor_Init(void);
void App_Motor_Proc(void);
void App_Motor_SetOmega_L(float omega);
void App_Motor_SetOmega_R(float omega);
void App_Motor_Cmd(uint8_t on);

#endif

#ifndef PID_H
#define PID_H

#include "stm32f10x.h"

typedef struct
{
    float Kp; //比例系数
    float Ki; //积分系数
    float Kd; //微分系数
    float SP; //设定值
    
    uint64_t t_k_1; // t[k-1]，上一次运行PID的时间
    float err_k_1; // err[k-1]，上一次运行PID的误差
    float err_int_k_1; // err_int[k-1]，上一次运行PID的积分值

    float UpperLimit; // UpperLimit，上限值
    float LowerLimit; // LowerLimit，下限值
} PID_TypeDef;

void PID_Init(PID_TypeDef *PID, float Kp, float Ki, float Kd); // 初始化PID
void PID_ChangeSP(PID_TypeDef *PID, float SP); // 改变设定值
float PID_Compute(PID_TypeDef *PID, float FB); // 计算PID输出
void PID_LimitConfig(PID_TypeDef *PID, float Upper, float Lower); // 配置PID的上下限值
void PID_Reset(PID_TypeDef *PID); // 复位PID控制器


#endif

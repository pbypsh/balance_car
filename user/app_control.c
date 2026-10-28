#include "app_control.h"
#include "pid.h"
#include "task.h"
#include "app_mpu6050.h"
#include "qmath.h"
#include "app_motor.h"
#include "app_encoder.h"

static PID_TypeDef pid_velocity; // 速度环的PID控制器
static PID_TypeDef pid_theta; // θ环的PID控制器
static PID_TypeDef pid_theta_dot; // θ点环的PID控制器
static PID_TypeDef pid_turn; // 转向环

static const float g = 9.8; // 重力加速度,单位是m/s^2
static const float lp = 0.062; // 平衡车轮胎中心到电池中心的距离,单位是m
static const float rw = 0.032; // 平衡车轮胎半径,单位是m

//
// @简介：负责初始化平衡车控制系统
//
void App_Control_Init(void) 
{
    PID_Init(&pid_velocity, 10.0f, 10.0f, 0.0f); // 初始化速度环PID
    PID_LimitConfig(&pid_velocity, +0.5 * g, -0.5 * g); // 将速度环PID的输出范围限制在+-0.5g 之间

    PID_Init(&pid_theta, 4.0f, 0.0f, 0.0f); // 初始化θ环PID
    PID_LimitConfig(&pid_theta, +12.57f, -12.57f); // 将θ环PID的输出范围限制在+-4PI rad/s 之间

    PID_Init(&pid_theta_dot, 10.0f, 10.0f, 0.0f); // 初始化θ点环PID
    PID_LimitConfig(&pid_theta_dot, +125.7f, -125.7f); // 将θ点环PID的输出范围限制在+-40PI rad/s 之间
}

static float omega_ref = 0.0f; // 速度环的参考值,单位是m/s
static uint64_t last_time = 0; // 上一次循环的时间,单位是毫秒级

//
// @简介：平衡车控制系统的进程函数，在while循环中调用
//
void App_Control_Proc(void)
{
    PERIODIC (5) // 每5ms执行一次

    uint64_t now = GetUs();
    float deltaT = (now - last_time) * 1.0e-6f;

    // #1. 改变速度环的设定值
//    PID_ChangeSP(&pid_velocity, 0.0f);
    
    // #2. 读取传感器的值
    float omega = 0.5 * (App_Encoder_GetSpeed_L() + App_Encoder_GetSpeed_R());
    float theta = App_MPU6050_GetPitch() * 0.0174533; // 将角度转换为弧度，单位是rad
    float theta_dot = App_MPU6050_GetGx() * 0.0174533; // 将角度速度转换为弧度，单位是rad/s

    // #3. 计算速度环的反馈值x_dot
    float omega2 = -theta_dot * (lp+rw) / rw;
    float omega1 = omega - omega2;
    float x_dot = omega1 * rw;

    // #4. 执行的速度环运算
    float theta_ref = qatan(PID_Compute(&pid_velocity, x_dot) / g);

    // #1. 将外环的设定值SP设置为0
    PID_ChangeSP(&pid_velocity, 0);

    // #3. 计算外环PID的输出
    float theta_dot_ref = PID_Compute(&pid_theta, theta);

    // #4. 计算内环PID的设定值SP
    PID_ChangeSP(&pid_theta_dot, theta_dot_ref);

    // #5. 计算内环PID的输出
    float theta_dot_dot_ref = PID_Compute(&pid_theta_dot, theta_dot);

    // #6. 倒立摆的逆结算
    float x_dot_dot_ref = (g * qsin(theta) - theta_dot_dot_ref * lp) / qcos(theta);

    // #7. 计算轮胎的转速
    if(last_time != 0)
    {
        omega_ref += 1.0f / rw * x_dot_dot_ref * deltaT;
    }

    // 转向环
    float gz = App_MPU6050_GetGz() * 0.0174533; // 将角度速度转换为弧度，单位是rad/s
    float omega_diff = PID_Compute(&pid_turn, gz);

    // #8. 设置轮胎的转速
    App_Motor_SetOmega_L(omega_ref + omega_diff);
    App_Motor_SetOmega_R(omega_ref - omega_diff);

    last_time = now;
}

//
// @简介：对控制系统进行复位
//
void App_Control_Reset(void)
{
    // #1. 复位暂存的值
    last_time = 0;
    omega_ref = 0.0f;

    // #2. 复位PID控制器
    PID_Reset(&pid_velocity);
    PID_Reset(&pid_theta);
    PID_Reset(&pid_theta_dot);
}

//
// @简介：改变平衡车移动的速度
// @参数：MoveSpeed - 移动速度，单位是m/s，最大行进速度为0.7m/s
//
void App_Control_SetMoveSpeed(float MoveSpeed)
{
    PID_ChangeSP(&pid_velocity, MoveSpeed);
}

//
// @简介：改变平衡车的转向速度
// @参数：TurnSpeed - 转向速度，单位是rad/s，最大转向速度为15rad/s
//
void App_Control_SetTurnSpeed(float TurnSpeed)
{
    PID_ChangeSP(&pid_turn, TurnSpeed);
}

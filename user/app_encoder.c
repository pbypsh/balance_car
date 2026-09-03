#include "app_encoder.h"
#include "delay.h"

static volatile int64_t encoder_l = 0; // 左编码器计数
static volatile int64_t encoder_r = 0; // 右编码器计数
static volatile int8_t direction_l = 0; // 左编码器方向, 1:正转, -1:逆转
static volatile int8_t direction_r = 0; // 右编码器方向, 1:正转, -1:逆转
static volatile int8_t t0_l , t1_l = 0; // 左编码器发生变化的时间戳，单位us
static volatile int8_t t0_r , t1_r = 0; // 右编码器发生变化的时间戳，单位us

static void Encoder_L_Init(void); // 左编码器初始化
static void Encoder_R_Init(void); // 右编码器初始化

//
// @简介：初始化编码器
// 
void App_Encoder_Init(void)
{
    Encoder_L_Init(); // 初始化左编码器
    Encoder_R_Init(); // 初始化右编码器
}

//
// @简介：读取左轮胎旋转的角度，单位：度
// 
float App_Encoder_GetPos_L(void)
{
    return encoder_l / 22.0f / (30613.0f / 1500.0f) * 360.0f; // 转换为角度,浮点数
}

//
// @简介：读取右轮胎旋转的角度，单位：度
// 
float App_Encoder_GetPos_R(void)
{
    return encoder_r / 22.0f / (30613.0f / 1500.0f) * 360.0f; // 转换为角度,浮点数
}

//
// @简介：读取左轮胎旋转的角速度，omega的值，单位：弧/s
// 
float App_Encoder_GetSpeed_L(void)
{
    __disable_irq(); // 关闭单片机的总中断

    int8_t direction_cpy = direction_l;
    int64_t t0_cpy = t0_l;
    int64_t t1_cpy = t1_l;

    __enable_irq(); // 开启单片机的总中断
    
    if(direction_cpy == +2 || direction_cpy == -2) return 0.0f;

    uint64_t now = GetUs();
    float T;

    if(t0_cpy - t1_cpy > now - t0_cpy) 
    {
        T = (t0_cpy - t1_cpy) * 1.0e-6;
    }
    else
    {
        T = (now - t0_cpy) * 1.0e-6;
    }

    return direction_cpy / T / (30613.0f / 1500.0f) * 6.2831853f; // 转换为速度，单位是弧/s
}

//
// @简介：读取右轮胎旋转的角速度，omega的值，单位：弧/s
// 
float App_Encoder_GetSpeed_R(void)
{
    __disable_irq(); // 关闭单片机的总中断

    int8_t direction_cpy = direction_r;
    int64_t t0_cpy = t0_r;
    int64_t t1_cpy = t1_r;

    __enable_irq(); // 开启单片机的总中断
    
    if(direction_cpy == +2 || direction_cpy == -2) return 0.0f;

    uint64_t now = GetUs();
    float T;

    if(t0_cpy - t1_cpy > now - t0_cpy) 
    {
        T = (t0_cpy - t1_cpy) * 1.0e-6;
    }
    else
    {
        T = (now - t0_cpy) * 1.0e-6;
    }

    return direction_cpy / T / (30613.0f / 1500.0f) * 6.2831853f; // 转换为速度，单位是弧/s
}

//
// @简介：左编码器初始化
// 
static void Encoder_L_Init(void)
{
    // 初始化A和B引脚
    // PB14，PB15  - IPU
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // 使能GPIOB时钟

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // EXTI初始化
    // 让EXTI_Line3监控PB3
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); // 使能AFIO时钟
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource3); // 配置EXTI_Line3为GPIOB_PinSource3

    // 配置EXTI参数
    EXTI_InitTypeDef EXTI_InitStruct = {0};
    
    EXTI_InitStruct.EXTI_Line = EXTI_Line3; // 选择能中断的线
    EXTI_InitStruct.EXTI_LineCmd = ENABLE; // 使能EXTI_Line3中断
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt; // 选择中断模式
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling; // 选择中断触发模式为下降沿或上升沿

    EXTI_Init(&EXTI_InitStruct); // 初始化EXTI

    //开启EXTI的中断
    NVIC_InitTypeDef NVIC_InitStruct = {0};
    
    NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn; // 选择能中断的线
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; // 使能EXTI3_IRQn
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; // 优先级为0
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0; // 子优先级为0

    NVIC_Init(&NVIC_InitStruct); // 初始化NVIC
}

//
// @简介：右编码器初始化
// 
static void Encoder_R_Init(void)
{
    // 初始化A和B引脚
    //关闭JTAG，开启SWD
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); // 使能AFIO时钟
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // 关闭JTAG，开启SWD

    // PB3，PB4  - IPU
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // 使能GPIOB时钟

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // EXTI初始化
    // 让EXTI_Line3监控PB14
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); // 使能AFIO时钟
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14); // 配置EXTI_Line3为GPIOB_PinSource14

    // 配置EXTI参数
    EXTI_InitTypeDef EXTI_InitStruct = {0};

    EXTI_InitStruct.EXTI_Line = EXTI_Line14; // 选择能中断的线
    EXTI_InitStruct.EXTI_LineCmd = ENABLE; // 使能EXTI_Line14中断
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt; // 选择中断模式
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling; // 选择中断触发模式为下降沿或上升沿

    EXTI_Init(&EXTI_InitStruct); // 初始化EXTI

    //开启EXTI的中断
    NVIC_InitTypeDef NVIC_InitStruct = {0};
    
    NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn; // 选择能中断的线
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; // 使能EXTI15_10_IRQn
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; // 优先级为0
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0; // 子优先级为0
    NVIC_Init(&NVIC_InitStruct); // 初始化NVIC
}

//
// @简介：EXTI3中断服务函数，对应右编码器的A相
// 
void EXTI3_IRQHandler(void)
{
    EXTI_ClearITPendingBit(EXTI_Line3); // 清除EXTI_Line3中断标志位

    t1_r = t0_r;
    t0_r = GetUs(); // 记录当前时间戳，单位us

    uint8_t a = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3); //A相的电平
    uint8_t b = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4); //B相的电平
    
    if((a == Bit_SET && b == Bit_RESET) || (a == Bit_RESET && b == Bit_SET)) // 现在轮胎正转
    {
        encoder_r++;

        if (direction_r < 0) // 之前是反转
        {
            direction_r = +2;
        }
        else
        {
            direction_r = 1;
        }
    }
    else  // 现在轮胎反转
    {
        encoder_r--;

        if (direction_r > 0) // 之前是正转
        {
            direction_r = -2;
        }
        else
        {
            direction_r = -1;
        }
    }   
}

//
// @简介：EXTI15_10中断服务函数，对应左编码器的B相
// 
void EXTI15_10_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line14) == SET) // 有中断发生
    {
        EXTI_ClearITPendingBit(EXTI_Line14); // 清除EXTI_Line14中断标志位

        t1_l = t0_l;
        t0_l = GetUs(); // 记录当前时间戳，单位us

        uint8_t a = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14); //A相的电平
        uint8_t b = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15); //B相的电平

        if((a == Bit_SET && b == Bit_RESET) || (a == Bit_RESET && b == Bit_SET)) // 现在轮胎反转
        {
            encoder_l--;

            if (direction_l > 0) // 之前是正转
            {
                direction_l = -2;
            }
            else
            {
                direction_l = -1;
            }

        }
        else  // 现在轮胎正转
        {
            encoder_l++;

            if (direction_l < 0) // 之前是反转
            {
                direction_l = +2;
            }
            else
            {
                direction_l = 1;
            }
        }   
    }
}

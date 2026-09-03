#include "app_bat.h"
#include "delay.h"

static volatile float vbat = 0.0f;

static void TIM2_TRGO_Init(void);
static void ADC1_Init(void);
static void LED_Init(void);

//
//@简介：对电池电压检测模块进行初始化
//
void App_Bat_Init(void)
{
    TIM2_TRGO_Init();
    ADC1_Init();
    LED_Init();
} 

static uint32_t last_time = 0; // 上次LED切换时间
static uint8_t stage = 0; // LED当前状态，0-熄灭，1-点亮

//
//@简介：电池电压检测模块的任务切片
//
void App_Bat_Proc(void)
{
    if (vbat > 7.9) // 电池满电，点亮3颗
    {
        GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
        GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET);
        GPIO_WriteBit(GPIOA, GPIO_Pin_6, Bit_SET);
    }
    else if (vbat > 7.4) // 电池75%，点亮2颗
    {
        GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
        GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET); 
        GPIO_WriteBit(GPIOA, GPIO_Pin_6, Bit_RESET);
    }
    else if (vbat > 7) // 电池50%，点亮1颗
    {
        GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
        GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET);
        GPIO_WriteBit(GPIOA, GPIO_Pin_6, Bit_RESET);
    }
    else if (vbat > 6.5) // 电池20%，熄灭所有
    {
        GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
        GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET);
        GPIO_WriteBit(GPIOA, GPIO_Pin_6, Bit_RESET);
    }
    else // 电池低电，熄灭所有
    {
        uint32_t now = GetTick();
        if (now - last_time > 100) 
        {
            switch (stage)
            {
            case 0:
            {
                GPIO_WriteBit(GPIOA, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6, Bit_SET); // 点亮
                stage = 1;
                break;
            }
            case 1:
            {
                GPIO_WriteBit(GPIOA, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6, Bit_RESET); // 熄灭
                stage = 0;
                break;
            }
            }
            last_time = now;
        }
    }
}


//
//@简介：对电源指示灯进行初始化
//      PA4 PA5 PA6 - Out_PP
//
static void LED_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); // 开启GPIOA时钟

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6; // 选择PA4,PA5,PA6引脚
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出模式
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz; // 2MHz输出速度
    
    GPIO_Init(GPIOA,&GPIO_InitStruct);
}

static void TIM2_TRGO_Init(void)
{
    // #1. 初始化TIM2_TRGO,每10ms产生一个脉冲
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //开启TIM2时钟

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructInit = {0};

    TIM_TimeBaseStructInit.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式
    TIM_TimeBaseStructInit.TIM_Period = 9999; //设置自动重装载寄存器值为9999，即10ms周期
    TIM_TimeBaseStructInit.TIM_Prescaler =71; //设置预分频器值为71，即72MHz/71=1000kHz，1ms周期
    TIM_TimeBaseStructInit.TIM_RepetitionCounter = 0; //设置重复计数寄存器值为0

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructInit); //初始化TIM2时间基础寄存器

    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update); //选择主机模式为update模式

    TIM_Cmd(TIM2, ENABLE); //使能TIM2
}

static void ADC1_Init(void)
{
    // #2. 对ADC进行初始化
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); //将ADC分频系数设置为6分频

    // 将PB0初始化成模拟输入模式

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); // 开启GPIOB时钟

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0; // 选择PB0引脚
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN; // 模拟输入模式

    GPIO_Init(GPIOB,&GPIO_InitStruct);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE); // 开启ADC1时钟

    // 初始化ADC的基本参数
    ADC_InitTypeDef ADC_InitStruct = {0};

    ADC_InitStruct.ADC_ContinuousConvMode = DISABLE; // 禁用连续转换模式
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right; // 数据对齐方式右对齐
    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 无外部触发转换
    ADC_InitStruct.ADC_Mode = ADC_Mode_Independent; // 独立模式
    ADC_InitStruct.ADC_NbrOfChannel = 1; // 1个通道
    ADC_InitStruct.ADC_ScanConvMode = DISABLE; // 禁用扫描转换模式

    ADC_Init(ADC1,&ADC_InitStruct);

    //设置注入序列的参数
	ADC_ExternalTrigInjectedConvConfig(ADC1,ADC_ExternalTrigInjecConv_T2_TRGO); // 选择TIM2_TRGO作为外部触发转换
    ADC_ExternalTrigInjectedConvCmd(ADC1,ENABLE); // 使能外部触发转换
    ADC_InjectedChannelConfig(ADC1,ADC_Channel_8,1,ADC_SampleTime_1Cycles5); // 选择注入通道8，采样时间为1个周期

    // 配置ADC的JEOC中断
    ADC_ITConfig(ADC1,ADC_IT_JEOC,ENABLE); // 使能JEOC中断

    // NVIC 中断优先级分组 0 1 2 3 4 0~15
    NVIC_InitTypeDef NVIC_InitStruct = {0};

    NVIC_InitStruct.NVIC_IRQChannel = ADC1_2_IRQn; // 选择ADC1中断  
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级为0
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2; // 子优先级为2
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; // 使能中断

    NVIC_Init(&NVIC_InitStruct); // 初始化NVIC

    ADC_Cmd(ADC1,ENABLE); // 使能ADC1
}

//
//@简介：获取电池电压值,单位为V
//
float App_Bat_Get(void)
{
    return vbat;
}

void ADC1_2_IRQHandler(void)
{
    if(ADC_GetITStatus(ADC1,ADC_IT_JEOC) == SET)
    {
        ADC_ClearFlag(ADC1,ADC_FLAG_JEOC); // 清除JEOC中断标志位

        uint16_t jdr1 = ADC_GetInjectedConversionValue(ADC1,ADC_InjectedChannel_1); // 获取注入通道1的转换结果

        vbat = jdr1 / 4095.0f*8.4f / 3.3; // 计算电池电压
        // 处理JEOC中断
    }
}

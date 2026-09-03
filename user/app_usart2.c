#include "app_usart2.h"

//
// @简介：初始化USART2，调试接口
// 
void App_USART2_Init(void)
{
    // #1. 对PA2和PA3进行初始化，PA2 - TX - AF_PP，PA3 - RX - IPU
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // 使能GPIOA时钟

    GPIO_InitTypeDef GPIO_InitStruct;

    // PA2 - TX - AF_PP
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz; // 921600

    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PA3 - RX - IPU
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;

    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // #2. 开启USART2时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
    // #3. 初始化USART2
    USART_InitTypeDef USART_InitStruct = {0};

    USART_InitStruct.USART_BaudRate = 921600;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;

    USART_Init(USART2, &USART_InitStruct);

    // #4. 使能USART2
    USART_Cmd(USART2, ENABLE);
}   

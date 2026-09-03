#include "bat_test.h"
#include "app_usart2.h"
#include "app_bat.h"
#include "delay.h"


//
// @简介：电池电压监控模块测试程序
//  通过USART2把电压值发送到PC端
//  用Vofa显示曲线
// 
void Bat_Test(void) // main
{
    App_USART2_Init(); // 初始化USART2 (用来打印数据)
    App_Bat_Init(); // 初始化电池电压监控模块 (用来读取电压值)

    while (1)
    {
        // 每间隔10ms显示一个数据点

        float volf = App_Bat_Get();

        My_USART_Printf(USART2, "%.3f\n", volf);
    
        Delay(10);
    }

}

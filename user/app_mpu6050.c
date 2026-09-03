#include "app_mpu6050.h"
#include "i2c.h"
#include "delay.h"
#include "task.h"
#include "math.h"
#include "qmath.h"


static float ax, ay, az; // 加速度计的值, 单位为g
static float temperature; // 温度的值, 单位为摄氏度
static float gx, gy, gz; // 陀螺仪的值, 单位为°/s
static float yaw, pitch, roll; // 欧拉角, 单位为度

static void reg_write(uint8_t reg, uint8_t value); // 写入寄存器
static uint8_t reg_read(uint8_t reg); // 读取寄存器

//
// @简介：初始化MPU6050
// 
void App_MPU6050_Init(void)
{
    // #1. 初始化I2C1=总线 PB8，PB9 - I2C1
    // 将I2C1的引脚重映射到PB8，PB9
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 
    GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE); 

    // 初始化PB8，PB9 - AF_OD
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 

    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // #2. 初始化I2C1
    // 开启I2C1时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    // 设置I2C1的参数
    I2C_InitTypeDef I2C_InitStructure = {0};

    I2C_InitStructure.I2C_ClockSpeed = 400000; // I2C时钟频率为400kHz
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2; // 占空比为2:1
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C; // I2C模式

    I2C_Init (I2C1, &I2C_InitStructure);

    // #3. 设置MPU6050的参数
    reg_write(0x6B, 0x80); // 复位MPU6050
    Delay(100); // 等待复位完成

    reg_write(0x6B, 0x00); // 使能MPU6050

    reg_write(0x1B, 0x18); // 将陀螺仪的量程设置为±2000°/s

    reg_write(0x1C, 0x00); // 将加速度计的量程设置为±2g
}

//
// @简介：MPU6050的进程函数
// 
void App_MPU6050_Proc(void)
{
    PERIODIC(5); // 进程函数每5ms运行一次

    App_MPU6050_Update(); // 更新MPU6050传感器的值

    // 通过陀螺仪的测量结果计算欧拉角
    float yaw_g = yaw + gz * 0.005; // 将陀螺仪的值转换为度
    float pitch_g = pitch + gy * 0.005; // 将陀螺仪的值转换为度
    float roll_g = roll + gx * 0.005; // 将陀螺仪的值转换为度 

    //通过加速度计解算欧拉角
    float pitch_a = qatan2(ay, az) / 3.1415927f * 180.0f; // 计算俯仰角
    float roll_a = qatan2(ax, az) / 3.1415927f * 180.0f; // 计算滚转角

    // 使用互补滤波器对陀螺仪和加速度计的计算结果进行融合
    yaw = yaw_g;
    pitch = 0.95238 * pitch_g + (1-0.95238) * pitch_a;
    roll = 0.95238 * roll_g + (1-0.95238) * roll_a;
}

//
// @简介：更新MPU6050的值
// 
void App_MPU6050_Update(void)
{
    // #1. 读取陀螺仪的值
    int16_t ax_raw = (int16_t)(reg_read(0x3B) << 8) + reg_read(0x3C); // ax的原始数据
    int16_t ay_raw = (int16_t)(reg_read(0x3D) << 8) + reg_read(0x3E); // ay的原始数据
    int16_t az_raw = (int16_t)(reg_read(0x3F) << 8) + reg_read(0x40); // az的原始数据

    ax = ax_raw * 6.1035e-5f; // 将原始数据转换为g
    ay = ay_raw * 6.1035e-5f; // 将原始数据转换为g
    az = az_raw * 6.1035e-5f; // 将原始数据转换为g

    int16_t temperature_raw = (int16_t)(reg_read(0x41) << 8) + reg_read(0x42); // 温度的原始数据

    temperature = temperature_raw / 340 + 36.53; // 将原始数据转换为摄氏度MPU6050
    temperature = temperature_raw / 333.87f + 21.0f; // MPU6500

    // #2. 读取加速度计的值
    int16_t gx_raw = (int16_t)(reg_read(0x43) << 8) + reg_read(0x44); // gx的原始数据
    int16_t gy_raw = (int16_t)(reg_read(0x45) << 8) + reg_read(0x46); // gy的原始数据
    int16_t gz_raw = (int16_t)(reg_read(0x47) << 8) + reg_read(0x48); // gz的原始数据

    gx = gx_raw * 6.1035e-2f; // 将原始数据转换为°/s
    gy = gy_raw * 6.1035e-2f; // 将原始数据转换为°/s
    gz = gz_raw * 6.1035e-2f; // 将原始数据转换为°/s
}

//
// @简介：获取x轴向加速度计的值，单位为g
// 
float App_MPU6050_GetAx(void)
{
    return ax;
}

//
// @简介：获取y轴向加速度计的值，单位为g
// 
float App_MPU6050_GetAy(void)
{
    return ay;
}

//
// @简介：获取z轴向加速度计的值，单位为g
// 
float App_MPU6050_GetAz(void)
{
    return az;
}

//
// @简介：获取温度的值，单位为摄氏度
// 
float App_MPU6050_GetTemperature(void)
{
    return temperature;
}

//
// @简介：获取绕x轴的角速度，单位为°/s
// 
float App_MPU6050_GetGx(void)
{
    return gx;
}

//
// @简介：获取绕y轴的角速度，单位为°/s
// 
float App_MPU6050_GetGy(void)
{
    return gy;
}

//
// @简介：获取绕z轴的角速度，单位为°/s
// 
float App_MPU6050_GetGz(void)
{
    return gz;
}

//
// @简介：获取偏航角的值，单位为°
// 
float App_MPU6050_GetYaw(void)
{
    return yaw;
}

//
// @简介：获取俯仰角的值，单位为°
// 
float App_MPU6050_GetPitch(void)
{
    return pitch;
}

//
// @简介：获取滚转角的值，单位为°
// 
float App_MPU6050_GetRoll(void)
{
    return roll;
}

//
// @简介：写入寄存器
// @参数 reg：寄存器地址
// @参数 value：要写入的值
//
 static void reg_write(uint8_t reg, uint8_t value)
{
    uint8_t byteToSend[] = {reg, value};

    My_I2C_SendBytes(I2C1, 0Xd0, byteToSend, 2);
}
//
// @简介：读取寄存器
// @参数 reg：寄存器地址
// @返回值：寄存器中的值
//
 static uint8_t reg_read(uint8_t reg)
{
    My_I2C_SendBytes(I2C1, 0Xd0, &reg, 1); // 发送寄存器地址

    uint8_t regValue;

    My_I2C_ReceiveBytes(I2C1,0Xd0,&regValue,1); // 读取一个字节数据

    return regValue;
}

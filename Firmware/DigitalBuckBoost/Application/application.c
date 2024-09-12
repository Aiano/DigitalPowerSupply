#include "application.h"
#include "hrtim.h"
#include "pwm.h"
#include "signal_conditioning.h"
#include "stdio.h"
#include "string.h"
#include "usart.h"
#include "pid.h"

/* 宏定义 */
// #define HARDWARE_DEBUG // 开启硬件调试
#ifdef HARDWARE_DEBUG

// #define HARDWARE_DEBUG_HRTIM_A // 端口A PWM测试
// #define HARDWARE_DEBUG_HRTIM_B // 端口B PWM测试
// #define HARDWARE_DEBUG_BUCK // 固定占空比Buck测试
// #define HARDWARE_DEBUG_BOOST // 固定占空比Boost测试
// #define HARDWARE_DEBUG_V_SENSE // 电压检测测试
// #define HARDWARE_DEBUG_I_SENSE // 电流检测测试
// #define HARDWARE_DEBUG_UART    // 串口测试
// #define HARDWARE_DEBUG_LED     // LED测试

#endif                        // HARDWARE_DEBUG
#define UVLO_MIN 11.3f        // 欠压锁定低电压
#define UVLO_MAX 11.7f        // 欠压锁定高电压
#define SLOW_START_SLOP 30.0f // 软起动斜率 单位：V/s

/* 类型定义 */
typedef enum
{
    BUCK_MODE,
    BOOST_MODE,
    BUCK_BOOST_MODE
} OPERATION_MODE;

/* 全局变量 */
uint8_t direction = 0;                      // 0-PortA to PortB  1-PortB to PortA
OPERATION_MODE operation_mode = BUCK_MODE;  // 工作模式
OPERATION_MODE slow_start_mode = BUCK_MODE; // 软起动工作模式
uint8_t flag_uvlo = 1;                      // 欠压锁定标志
uint8_t flag_slow_start = 0;                // 缓启动标志
uint8_t flag_error_supply_voltage = 0;      // 供电电压异常错误标志
uint8_t flag_init_finished = 0;             // 初始化完成标志
uint8_t flag_data_update = 0;               // 数据采集更新
float switching_frequency = 200000;         // 开关频率 单位：Hz
float switching_period = 0.000005f;         // 开关周期 单位：s
float duty_cycle1 = 0.5f;                   // 占空比1 输入端半桥上管导通占空比
float duty_cycle2 = 0.5f;                   // 占空比2 输出端半桥下管导通占空比
float target_Vout = 6.0f;                  // 目标输出电压 单位：V
float slow_start_Vout = 0.65f;              // 缓启动目标输出电压 单位：V
float comp_duty_cycle = 0.0f;               // PID补偿占空比

uint8_t tx_buffer[50]; // 串口发送字符串

void APP_DetermineDirection()
{
    HAL_Delay(500);                        // 等待供电电压稳定
    if (Vin > UVLO_MAX && Vout > UVLO_MAX) // 都满足解锁要求情况下，谁电压高谁当输入
    {
        if (Vin > Vout)
        {
            direction = 0;
        }
        else
        {
            direction = 1;
        }
    }
    else if (Vin > UVLO_MAX) // 只有一个满足解锁要求情况下，谁满足谁当输入
    {
        direction = 0;
    }
    else if (Vout > UVLO_MAX)
    {
        direction = 1;
    }
    else
    { // 都不满足情况下，报错，不输出电压
        flag_error_supply_voltage = 1;
        while (1)
        {
            HAL_GPIO_WritePin(LED_OUTPUT_GPIO_Port, LED_OUTPUT_Pin, GPIO_PIN_SET);
            HAL_GPIO_TogglePin(LED_PROTECTION_GPIO_Port, LED_PROTECTION_Pin);
            HAL_Delay(200);
        }
    }
}

/**
 * @brief 业务初始化
 *
 */
void APP_Init()
{
    /* 参数计算 */
    switching_period = 1.0f / switching_frequency;

    /* 硬件初始化 */
#ifdef HARDWARE_DEBUG // 调试
#ifdef HARDWARE_DEBUG_HRTIM_A
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);
    HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_MASTER | HRTIM_TIMERID_TIMER_B);
#endif
#ifdef HARDWARE_DEBUG_HRTIM_B
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
    HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_MASTER | HRTIM_TIMERID_TIMER_A);
#endif
#ifdef HARDWARE_DEBUG_BUCK
    PWM_Enable();
    PWM_SetDutyCycle(0.5f, 0.95f);
#endif
#ifdef HARDWARE_DEBUG_BOOST
    PWM_Enable();
    PWM_SetDutyCycle(0.95f, 0.5f);
#endif
#ifdef HARDWARE_DEBUG_V_SENSE
    PWM_SetDutyCycle(0.05f, 0.95f);
    PWM_Enable();
    // flag_slow_start = 1;
    SC_Init();
#endif
#else // 正常
    PID_Init();
    SC_Init();
    PWM_Init();
    APP_DetermineDirection();
    flag_init_finished = 1;
#endif
}

/**
 * @brief 业务主循环
 *
 */
void APP_MainLoop()
{
    if (flag_data_update)
    {
        sprintf((char *)tx_buffer, "%.3f,%.3f,%.3f,%.3f,%.3f,%d\n", Vin, Vout, duty_cycle1, duty_cycle2, pid_voltage.output, direction);
        HAL_UART_Transmit_DMA(&huart1, tx_buffer, strlen((char *)tx_buffer));
        flag_data_update = 0;
    }
}

/**
 * @brief 电压环
 *
 */
void APP_VoltageClosedLoop()
{
    /* 错误处理 */
    if (flag_error_supply_voltage || !flag_init_finished)
    {
        return;
    }

    /* 欠压锁定 */
    if (flag_uvlo)
    {
        if (Vin > UVLO_MAX) // 从欠压锁定到解锁
        {
            flag_uvlo = 0;
            flag_slow_start = 1; // 开始软启动
            slow_start_Vout = 0.65f;
            PWM_SetDutyCycle(0.05f, 0.95f);
            PWM_Enable();

            HAL_GPIO_WritePin(LED_OUTPUT_GPIO_Port, LED_OUTPUT_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_PROTECTION_GPIO_Port, LED_PROTECTION_Pin, GPIO_PIN_SET);
        }
        else
        {
            flag_slow_start = 0;
            slow_start_mode = BUCK_MODE;
            PWM_SetDutyCycle(0.05f, 0.95f);
            return;
        }
    }
    else
    {
        if (Vin < UVLO_MIN) // 从解锁到欠压锁定
        {
            flag_uvlo = 1;
            PWM_Disable();
            PWM_SetDutyCycle(0.05f, 0.95f);
            pid_voltage.output = 0;
            pid_voltage.integral = 0;

            HAL_GPIO_WritePin(LED_OUTPUT_GPIO_Port, LED_OUTPUT_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_PROTECTION_GPIO_Port, LED_PROTECTION_Pin, GPIO_PIN_RESET);
            return;
        }
    }

    /* 软启动 */
    if (flag_slow_start) // 软启动是固定输入（近似），过渡输出，所以要先从Buck过渡到Boost
    {
        slow_start_Vout += switching_period * SLOW_START_SLOP;

        switch (slow_start_mode)
        {
        case BUCK_MODE:
            duty_cycle2 = 0.05f;
            duty_cycle1 = slow_start_Vout / Vin * (1 - duty_cycle2);
            break;
        case BOOST_MODE:
            duty_cycle1 = 0.95f;
            duty_cycle2 = 1.0f - duty_cycle1 * Vin / slow_start_Vout;
            break;
        default:
            break;
        }

        if (duty_cycle1 < 0.05f)
            duty_cycle1 = 0.05f;
        else if (duty_cycle1 > 0.95f)
            duty_cycle1 = 0.95f;
        if (duty_cycle2 < 0.05f)
            duty_cycle2 = 0.05f;
        else if (duty_cycle2 > 0.95f)
            duty_cycle2 = 0.95f;

        PWM_SetDutyCycle(duty_cycle1, 1 - duty_cycle2);

        /* 模式转换 */
        if (slow_start_Vout >= Vin)
        {
            slow_start_mode = BOOST_MODE;
        }

        /* 结束标志 */
        if (slow_start_Vout >= target_Vout)
        {
            flag_slow_start = 0;
            slow_start_mode = BUCK_MODE;
        }
        return;
    }

    /* 模式判断 */
    if (Vin > target_Vout) // 正常工作是输入波动，输出固定，所以模式根据输入判断
    {
        operation_mode = BUCK_MODE;
    }
    else
    {
        operation_mode = BOOST_MODE;
    }

    /* PID */
    comp_duty_cycle = PID_Calculate(&pid_voltage, target_Vout - Vout);

    /* 计算占空比 */
    switch (operation_mode)
    {
    case BUCK_MODE:
        duty_cycle2 = 0.05f;
        duty_cycle1 = target_Vout / Vin * (1 - duty_cycle2);

        duty_cycle1 += comp_duty_cycle;

        if (duty_cycle1 > 0.95f)
        {
            duty_cycle2 += duty_cycle1 - 0.95f;
            duty_cycle1 = 0.95f;
        }
        break;
    case BOOST_MODE:
        duty_cycle1 = 0.95f;
        duty_cycle2 = 1.0f - duty_cycle1 * Vin / target_Vout;

        duty_cycle2 += comp_duty_cycle;

        if(duty_cycle2 < 0.05f){
            duty_cycle1 += duty_cycle2 - 0.05f;
            duty_cycle2 = 0.05;
        }
        break;
    case BUCK_BOOST_MODE:
        break;
    default:
        break;
    }

    /* 占空比限幅 */
    if (duty_cycle1 < 0.05f)
        duty_cycle1 = 0.05f;
    else if (duty_cycle1 > 0.95f)
        duty_cycle1 = 0.95f;
    if (duty_cycle2 < 0.05f)
        duty_cycle2 = 0.05f;
    else if (duty_cycle2 > 0.95f)
        duty_cycle2 = 0.95f;

    /* 更新占空比 */
    PWM_SetDutyCycle(duty_cycle1, 1 - duty_cycle2);
}

/**
 * @brief 电流环
 *
 */
void APP_CurrentClosedLoop()
{
}

/**
 * @brief 闭环控制
 *
 */
void APP_ClosedLoopControl()
{
    APP_VoltageClosedLoop();
}

/**
 * @brief ADC注入转换完成回调函数
 * @details 在这里完成PWM逐周期信号采样和反馈调整
 *
 * @param hadc
 */
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    SC_Compute();
    APP_ClosedLoopControl();
    flag_data_update = 1;
}
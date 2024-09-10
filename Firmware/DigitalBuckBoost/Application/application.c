#include "application.h"
#include "hrtim.h"
#include "pwm.h"
#include "signal_conditioning.h"

/* 宏定义 */
#define HARDWARE_DEBUG // 开启硬件调试
#ifdef HARDWARE_DEBUG

// #define HARDWARE_DEBUG_HRTIM_A // 端口A PWM测试
// #define HARDWARE_DEBUG_HRTIM_B // 端口B PWM测试
// #define HARDWARE_DEBUG_BUCK // 固定占空比Buck测试
// #define HARDWARE_DEBUG_BOOST // 固定占空比Boost测试
#define HARDWARE_DEBUG_V_SENSE // 电压检测测试
// #define HARDWARE_DEBUG_I_SENSE // 电流检测测试
// #define HARDWARE_DEBUG_UART    // 串口测试
// #define HARDWARE_DEBUG_LED     // LED测试

#endif                        // HARDWARE_DEBUG
#define UVLO_MIN 7.0f         // 欠压锁定低电压
#define UVLO_MAX 8.0f        // 欠压锁定高电压
#define SLOW_START_SLOP 30.0f // 软起动斜率 单位：V/s

/* 类型定义 */
typedef enum
{
    BUCK_MODE,
    BOOST_MODE,
    BUCK_BOOST_MODE
} OPERATION_MODE;

/* 全局变量 */
OPERATION_MODE operation_mode = BUCK_MODE; // 工作模式
uint8_t flag_uvlo = 1;                     // 欠压锁定标志
uint8_t flag_slow_start = 0;               // 缓启动标志
float switching_frequency = 200000;        // 开关频率 单位：Hz
float switching_period = 0.000005f;        // 开关周期 单位：s
float duty_cycle1 = 0.5f;                  // 占空比1 输入端半桥上管导通占空比
float duty_cycle2 = 0.5f;                  // 占空比2 输出端半桥下管导通占空比
float target_Vout = 6.0f;                  // 目标输出电压 单位：V
float slow_start_Vout = 0.65f;             // 缓启动目标输出电压 单位：V

/**
 * @brief 业务初始化
 *
 */
void APP_Init()
{
    /* 参数计算 */
    switching_period = 1.0f / switching_frequency;

    /* 硬件初始化 */
#ifdef HARDWARE_DEBUG
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
#else

#endif
}

/**
 * @brief 业务主循环
 *
 */
void APP_MainLoop()
{
}

/**
 * @brief 电压环
 *
 */
void APP_VoltageClosedLoop()
{
    /* 欠压锁定 */
    if (flag_uvlo)
    {
        if (Vin > UVLO_MAX) // 从欠压锁定到解锁
        {
            flag_uvlo = 0;
            flag_slow_start = 1; // 开始软启动
            slow_start_Vout = 0.65f;
            PWM_SetDutyCycle(0.05f, 0.95f);
        }
        else
        {
            flag_slow_start = 0;
            PWM_SetDutyCycle(0.05f, 0.95f);
        }
    }
    else
    {
        if (Vin < UVLO_MIN) // 从解锁到欠压锁定
        {
            flag_uvlo = 1;
            PWM_SetDutyCycle(0.05f, 0.95f);
            return;
        }
    }

    /* 软启动 */
    if (flag_slow_start)
    {
        slow_start_Vout += switching_period * SLOW_START_SLOP;

        duty_cycle2 = 0.05f;
        duty_cycle1 = slow_start_Vout / Vin * (1 - duty_cycle2);

        if (duty_cycle1 > 0.95f)
            duty_cycle1 = 0.95f;

        PWM_SetDutyCycle(duty_cycle1, 1 - duty_cycle2);

        if (slow_start_Vout >= target_Vout)
        {
            flag_slow_start = 0;
        }
        return;
    }

    switch (operation_mode)
    {
    case BUCK_MODE:
        duty_cycle2 = 0.05f;
        duty_cycle1 = target_Vout / Vin * (1 - duty_cycle2);

        if (duty_cycle1 < 0.05f)
            duty_cycle1 = 0.05f;
        else if (duty_cycle1 > 0.95f)
            duty_cycle1 = 0.95f;

        PWM_SetDutyCycle(duty_cycle1, 1 - duty_cycle2);
        break;
    case BOOST_MODE:
        break;
    case BUCK_BOOST_MODE:
        break;
    default:
        break;
    }
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
}
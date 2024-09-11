#include "pwm.h"
#include "hrtim.h"

#define FHRCK 5440000000.0f // HRTIM等效时钟频率
static uint16_t period = 27200;
static float frequency = 200000;
static float duty_portA = 0.5;
static float duty_portB = 0.5;


void PWM_Init()
{
    HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_MASTER);
}

void PWM_Enable()
{
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2 | HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
    HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_TIMER_B | HRTIM_TIMERID_TIMER_A);
}

void PWM_Disable()
{
    HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2 | HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
    HAL_HRTIM_WaveformCounterStop(&hhrtim1, HRTIM_TIMERID_TIMER_B | HRTIM_TIMERID_TIMER_A);
}

/**
 * @brief 设置PWM占空比
 * 
 * @param portA Port1侧半桥上管占空比 0~1
 * @param portB Port2侧半桥上管占空比 0~1
 */
void PWM_SetDutyCycle(float portA, float portB)
{
    duty_portA = portA;
    duty_portB = portB;
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, period * duty_portB);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, period * duty_portA);
}

void PWM_SetFreq(float freq)
{
    frequency = freq;
    period = FHRCK / frequency;
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, period * duty_portB);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, period * duty_portA);
}

void PWM_SetDeadTime(uint16_t deadTime)
{
    
}

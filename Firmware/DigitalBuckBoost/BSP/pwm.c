#include "pwm.h"
#include "hrtim.h"
#include "application.h"

#define FHRCK 5440000000.0f // HRTIM等效时钟频率
static uint16_t period = 27200;
static float frequency = 200000;
static float duty_port1 = 0.5;
static float duty_port2 = 0.5;


void PWM_Init()
{
    HAL_HRTIM_WaveformCountStart_IT(&hhrtim1, HRTIM_TIMERID_MASTER);
    
}

void PWM_Enable()
{
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2 | HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
    HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_TIMER_B | HRTIM_TIMERID_TIMER_A);
}

void PWM_Disable()
{
    HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2 | HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
    HAL_HRTIM_WaveformCountStop(&hhrtim1, HRTIM_TIMERID_TIMER_B | HRTIM_TIMERID_TIMER_A);
}

/**
 * @brief 设置PWM占空比
 * 
 * @param port1 Port1侧半桥上管占空比 0~1
 * @param port2 Port2侧半桥上管占空比 0~1
 */
void PWM_SetDutyCycle(float port1, float port2)
{
    duty_port1 = port1;
    duty_port2 = port2;
    if(direction == 0){
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, period * duty_port2);
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, period * duty_port1);
    }
    else{
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, period * duty_port1);
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, period * duty_port2);
    }
    
}

void PWM_SetFreq(float freq)
{
    frequency = freq;
    period = FHRCK / frequency;
    __HAL_HRTIM_SETPERIOD(&hhrtim1, HRTIM_TIMERINDEX_MASTER, period);
    __HAL_HRTIM_SETPERIOD(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, period);
    __HAL_HRTIM_SETPERIOD(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, period);
    if(direction == 0){
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, period * duty_port2);
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, period * duty_port1);
    }
    else{
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, period * duty_port1);
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, period * duty_port2);
    }
}

void PWM_SetDeadTime(uint16_t deadTime)
{
    
}

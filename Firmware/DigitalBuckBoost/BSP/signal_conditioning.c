#include "signal_conditioning.h"
#include "adc.h"
#include "application.h"

#define V_SCALE 0.1f
#define I_SCALE 200.0f
#define I_SAMPLE_RESISTANCE 0.001f
#define I_OFFSET 1.65f

static float Vcoff, Icoff1, Icoff2;

float Vin, Vout, Iin, Iout;

void SC_Init()
{
    Vcoff = 3.3f / V_SCALE / 0xFFF;
    Icoff1 = 3.3f / 0xFFF;
    Icoff2 = 1.0f / I_SAMPLE_RESISTANCE / I_SCALE;
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    __HAL_ADC_ENABLE_IT(&hadc1, ADC_IT_JEOC);
    HAL_ADCEx_InjectedStart(&hadc1);
}

void SC_Compute()
{
    if(direction == 0){
        Vin = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1) * Vcoff;
        Vout = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2) * Vcoff;
        Iin = (HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3) * Icoff1 - I_OFFSET) * Icoff2;
        Iout = (HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_4) * Icoff1 - I_OFFSET) * Icoff2;
    }else{
        Vin = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2) * Vcoff;
        Vout = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1) * Vcoff;
        Iin = (HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_4) * Icoff1 - I_OFFSET) * Icoff2;
        Iout = (HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3) * Icoff1 - I_OFFSET) * Icoff2;
    }
}


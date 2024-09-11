#ifndef __PWM_H
#define __PWM_H

#include "main.h"

void PWM_Init();
void PWM_Enable();
void PWM_Disable();
void PWM_SetDutyCycle(float port1, float port2);
void PWM_SetFreq(float freq);
void PWM_SetDeadTime(uint16_t deadTime);

#endif // __PWM_H
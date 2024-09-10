#ifndef __SIGNAL_CONDITIONING_H
#define __SIGNAL_CONDITIONING_H

#include "main.h"

extern float Vin, Vout, Iin, Iout;

void SC_Init();
void SC_Compute();

#endif
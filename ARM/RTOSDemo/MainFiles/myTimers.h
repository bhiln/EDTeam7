#ifndef _MY_TIMERS_H
#define _MY_TIMERS_H

#include "lcdTask.h"
#include "i2cIR0.h"

void startTimerForLCD(vtLCDStruct *vtLCDdata);
void startTimerForIR0(vtIR0Struct *vtIR0data);

#endif
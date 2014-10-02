#ifndef TASK_LOCATE_H
#define TASK_LOCATE_H

#include "vtI2C.h"
#include "taskLCD.h"

// Maximum length of a message that can be received by any IR task.
#define maxLenLocate (sizeof(portTickType))

// Types of messages this task will receive.
#define msgTypeIR00	0
#define msgTypeIR01 	1
#define msgTypeIR10	2
#define msgTypeIR11 	3
#define msgTypeIR20	4
#define msgTypeIR21 	5
#define msgTypeIR30	6
#define msgTypeIR31 	7
#define msgTypeIR40	8
#define msgTypeIR41 	9
#define msgTypeAC00	10

// Structure used to pass parameters to the task.
typedef struct __structLocate
{
	structLCD *dataLCD;
	xQueueHandle inQ;
} structLocate;

signed char locateName[] = "Locate";

// Sensor sensor.
void startTaskLocate(structLocate* dataLocate, unsigned portBASE_TYPE uxPriority, structLCD* dataLCD);
portBASE_TYPE sendTimerMsgLocate(structLocate* dataLocate, portTickType ticksElapsed, portTickType ticksToBlock);
portBASE_TYPE sendValueMsgLocate(structLocate* dataLocate, uint8_t msgType, uint16_t value, portTickType ticksToBlock);

#endif

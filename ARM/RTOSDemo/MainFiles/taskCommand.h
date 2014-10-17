#ifndef TASK_COMMAND_H
#define TASK_COMMAND_H

#include "defs.h"
//#include "taskLCD.h"
#include "vtI2C.h"

// Maximum length of a message that can be received by any IR task.
#define maxLenCommand (sizeof(portTickType))

// Structure used to pass parameters to the task.
typedef struct __structCommand
{
	vtI2CStruct* devI2C0;
	structLCD* dataLCD;
	xQueueHandle inQ;
} structCommand;

void startTaskCommand(structCommand* dataCommand, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0, structLCD* dataLCD);
portBASE_TYPE sendTimerMsgCommand(structCommand* dataCommand, portTickType ticksElapsed, portTickType ticksToBlock);
portBASE_TYPE sendValueMsgCommand(structCommand* dataCommand, uint8_t msgType, uint16_t value, portTickType ticksToBlock);

#endif

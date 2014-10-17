#ifndef TASK_SENSORS_H
#define TASK_SENSORS_H

#include "defs.h"
#include "taskLCD.h"
#include "taskLocate.h"
#include "vtI2C.h"

// Structure used to pass parameters to the task.
typedef struct __structSensors {
	vtI2CStruct*  devI2C0;
	structLCD*    dataLCD;
	structLocate* dataLocate;
	xQueueHandle  inQ;
} structSensors;

// Sensors task.
void startTaskSensors(structSensors* dataSensors, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0, structLCD* dataLCD);
portBASE_TYPE sendTimerMsgSensors(structSensors* dataSensors, portTickType ticksElapsed, portTickType ticksToBlock);
portBASE_TYPE sendValueMsgSensors(structSensors* dataSensors, uint8_t msgType, uint8_t* value, portTickType ticksToBlock);

void sendI2CSensorsQuery();

#endif

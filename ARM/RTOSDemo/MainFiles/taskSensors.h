#ifndef TASK_SENSORS_H
#define TASK_SENSORS_H

#include "vtI2C.h"
#include "taskLCD.h"
#include "taskLocate.h"

// Maximum length of a message that can be received by any IR task.
#define maxLenSensor (sizeof(portTickType))

signed char taskNameSensor[] = "Sensor"; 

// Structure used to pass parameters to the task.
typedef struct __structSensor {
	vtI2CStruct* devI2C0;
	structLCD* dataLCD;
	structLocate* dataLocate;
	xQueueHandle inQ;
} structSensor;

// Sensor task.
void startTaskSensor(structSensor* dataSensor, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C, structLCD* dataLCD);
portBASE_TYPE sendTimerMsgSensor(structSensor* dataSensor, uint8_t msgType, portTickType ticksElapsed, portTickType ticksToBlock);
portBASE_TYPE sendValueMsgSensor(structSensor* dataSensor, uint8_t msgType, uint16_t value, portTickType ticksToBlock);

#endif

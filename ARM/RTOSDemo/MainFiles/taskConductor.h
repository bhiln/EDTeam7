#ifndef TASK_CONDUCTOR_H
#define TASK_CONDUCTOR_H
#include "vtI2C.h"
#include "taskSensors.h"

// Structure used to pass parameters to the task.
typedef struct __structConductor {
	vtI2CStruct *devI2C0;
	structSensor *dataIR00;
	structSensor *dataIR01;
	structSensor *dataIR10;
	structSensor *dataIR11;
	structSensor *dataIR20;
	structSensor *dataIR21;
	structSensor *dataIR30;
	structSensor *dataIR31;
	structSensor *dataIR40;
	structSensor *dataIR41;
	structSensor *dataAC00;

} structConductor;

void startTaskConductor(structConductor *conductorData, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0,
	structSensor* dataIR00, structSensor* dataIR01,
	structSensor* dataIR10, structSensor* dataIR11,
	structSensor* dataIR20, structSensor* dataIR21,
	structSensor* dataIR30, structSensor* dataIR31,
	structSensor* dataIR40, structSensor* dataIR41,
	structSensor* dataAC00);


#endif

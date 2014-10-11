#ifndef TASK_CONDUCTOR_H
#define TASK_CONDUCTOR_H
#include "vtI2C.h"
#include "taskSensors.h"

// Structure used to pass parameters to the task.
typedef struct __structConductor {
	vtI2CStruct* devI2C0;
	structSensor* dataSensor;
} structConductor;

signed char taskNameConductor[] = "Conductor"; 

void startTaskConductor(structConductor* conductorData, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0, structSensor* dataSensor);

#endif

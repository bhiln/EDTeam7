/*------------------------------------------------------------------------------
 * File:		taskConductor.h
 * Authors: 	FreeRTOS, Igor Janjic
 * Description:	Specification file for conductor class. Routes I2C messages to
 *              their appropriate targets.
 *----------------------------------------------------------------------------*/

#ifndef TASK_CONDUCTOR_H
#define TASK_CONDUCTOR_H

/*------------------------------------------------------------------------------
 * Includes
 **/

#include <stdlib.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "projdefs.h"
#include "semphr.h"
#include "task.h"

#include "vtI2C.h"
#include "vtUtilities.h"

#include "debug.h"
#include "defs.h"
#include "taskLocate.h"
#include "taskLCD.h"
#include "taskSensors.h"

/*------------------------------------------------------------------------------
 * Data Structures
 **/

// Structure used to pass parameters to the task.
typedef struct __structConductor {
	vtI2CStruct*   devI2C0;
    structLCD*     dataLCD;
	structSensors* dataSensors;
    structLocate*  dataLocate;
} structConductor;

/*------------------------------------------------------------------------------
 * Functions
 **/

void startTaskConductor(structConductor* conductorData, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0, structLCD* dataLCD, structSensors* dataSensors,
    structLocate* locateData);

#endif

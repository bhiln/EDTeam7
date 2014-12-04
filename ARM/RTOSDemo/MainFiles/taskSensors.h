/*------------------------------------------------------------------------------
 * File:            taskSensors.h
 * Authors:         FreeRTOS, Igor Janjic
 * Description:     Specification file for the sensors task. Reads the sensor
 *                  data, processes it, and sends it to the relevant tasks.
 *----------------------------------------------------------------------------*/

#ifndef TASK_SENSORS_H
#define TASK_SENSORS_H

/*------------------------------------------------------------------------------
 * Includes
 **/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "FreeRTOS.h"
#include "lpc17xx_gpio.h"
#include "projdefs.h"
#include "semphr.h"
#include "task.h"

#include "vtUtilities.h"
#include "vtI2C.h"

#include "debug.h"
#include "defs.h"
#include "taskLCD.h"
#include "taskLocate.h"

/*------------------------------------------------------------------------------
 * Definitions
 **/

// Length of the queue to this task.
#define QUEUE_LEN_SENS          10

// Maximum length of the buffer for this task.
#define QUEUE_BUF_LEN_SENS      22

/*------------------------------------------------------------------------------
 * Task Data Structures
 **/

// Structure used to pass parameters to the task.
typedef struct __structSensors
{
	vtI2CStruct*  devI2C0;
	structLCD*    dataLCD;
	structLocate* dataLocate;
	xQueueHandle  inQ;
} structSensors;

// Actual data structure that is sent in a message.
typedef struct __msgSensors
{
    uint8_t type;
    uint8_t length;
    uint8_t buf[QUEUE_BUF_LEN_SENS];
} msgSensors;

/*------------------------------------------------------------------------------
 * Sensors API
 **/

void startTaskSensors(structSensors* dataSensors, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0, structLCD* dataLCD, structLocate* dataLocate);
void sendTimerMsgSensors(structSensors* dataSensors, portTickType ticksElapsed, portTickType ticksToBlock);
void sendValueMsgSensors(structSensors* dataSensors, uint8_t type, uint8_t* value, portTickType ticksToBlock);

#endif

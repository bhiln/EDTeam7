/*------------------------------------------------------------------------------
 * File:            taskCommand.h
 * Authors:         FreeRTOS, Igor Janjic
 * Description:     Specification file for the command task.
 *----------------------------------------------------------------------------*/

#ifndef TASK_COMMAND_H
#define TASK_COMMAND_H

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

/*------------------------------------------------------------------------------
 * Definitions
 **/

// Length of the queue to this task.
#define QUEUE_LEN_CMD           10

// Maximum length of the buffer for this task.
#define QUEUE_BUF_LEN_CMD       3

/*------------------------------------------------------------------------------
 * Task Data Structures
 **/

// Structure used to pass parameters to the task.
typedef struct __structCommand
{
	vtI2CStruct* devI2C0;
	structLCD* dataLCD;
	xQueueHandle inQ;
} structCommand;

// Actual data structure that is sent in a message.
typedef struct __msgCommand
{
	uint8_t type;
	uint8_t	length;
	uint8_t buf[QUEUE_BUF_LEN_CMD];
} msgCommand;

/*------------------------------------------------------------------------------
 * Command API
 **/

void startTaskCommand(structCommand* dataCommand, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0, structLCD* dataLCD);
void sendTimerMsgCommand(structCommand* dataCommand, portTickType ticksElapsed, portTickType ticksToBlock);
void sendValueMsgCommand(structCommand* dataCommand, uint8_t type, uint8_t* value, portTickType ticksToBlock);

bool execCommand(vtI2CStruct* devI2C0, structLCD* dataLCD, uint8_t* index, uint8_t type, uint8_t value, uint8_t speed);

#endif

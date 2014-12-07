/*------------------------------------------------------------------------------
 * File:		taskTimers.h
 * Authors: 	FreeRTOS, Igor Janjic
 * Description:	Specification file for all timers used.
 *----------------------------------------------------------------------------*/

#ifndef TASK_TIMERS_H
#define TASK_TIMERS_H

/*------------------------------------------------------------------------------
 * Includes
 **/

#include "FreeRTOS.h"
#include "projdefs.h"
#include "task.h"
#include "timers.h"

#include "vtUtilities.h"

#include "defs.h"
#include "taskLCD.h"
#include "taskCommand.h"
#include "taskLocate.h"
#include "taskSensors.h"

/*------------------------------------------------------------------------------
 * Definitions
 **/

// Number of milliseconds elapsed between task update.
#define LCD_UPDATE_TIME		    1000
#define lcdWRITE_RATE_BASE	    ((portTickType) LCD_UPDATE_TIME/portTICK_RATE_MS)

#define LOCATE_UPDATE_TIME	    500
#define LOCATE_WRITE_RATE_BASE	((portTickType) LOCATE_UPDATE_TIME/portTICK_RATE_MS)

#define COMMAND_UPDATE_TIME	    500
#define COMMAND_WRITE_RATE_BASE	((portTickType) COMMAND_UPDATE_TIME/portTICK_RATE_MS)

// Number of times to query slave PIC for sensor data (# reads/8 seconds)
#define SENSOR_READS		    8/8
#define SENSOR_WRITE_RATE_BASE	(((portTickType) (8000/(SENSOR_READS*8)))/portTICK_RATE_MS)

/*------------------------------------------------------------------------------
 * Functions
 **/

void startTimerLCD(structLCD* dataLCD);
void startTimerSensors(structSensors* dataSensors);
void startTimerLocate(structLocate* dataLocate);
void startTimerCommand(structCommand* dataCommand);

#endif

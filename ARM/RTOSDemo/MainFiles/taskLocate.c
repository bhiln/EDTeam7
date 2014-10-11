/*------------------------------------------------------------------------------
 * File:		taskLocate.c
 * Authors: 		FreeRTOS, Igor Janjic
 * Description:		Determines location of rover and ramps.
 *----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * Includes
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "projdefs.h"
#include "semphr.h"
#include "lpc17xx_gpio.h"

#include "vtUtilities.h"
#include "taskLCD.h"
#include "taskLocate.h"
#include "I2CTaskMsgTypes.h"
#include "debug.h"

/*------------------------------------------------------------------------------
 * Configuration
 */
// Length of the queue to this task.
#define queueLenLocate 10

// Stack sizes.
#define BASE_STACK 3
#if PRINTF_VERSION == 1
#define I2C_STACK_SIZE		((BASE_STACK+5)*configMINIMAL_STACK_SIZE)
#else
#define I2C_STACK_SIZE		(BASE_STACK*configMINIMAL_STACK_SIZE)
#endif

// Actual data structure that is sent in a message.
typedef struct __msgLocate
{
	uint8_t msgType;
	uint8_t	length;
	uint16_t buf[maxLenLocate + 1];
} msgLocate;

int getMsgTypeLocate(msgLocate *Buffer) {return(Buffer->msgType);}

static portTASK_FUNCTION(updateTaskLocate, pvParameters);
void locateRover();

void startTaskLocate(structLocate* dataLocate, unsigned portBASE_TYPE uxPriority, structLCD* dataLCD)
{

	// Create the queue that will be used to talk to this task.
	if ((dataLocate->inQ = xQueueCreate(queueLenLocate, sizeof(msgLocate))) == NULL)
		VT_HANDLE_FATAL_ERROR(0);

	// Create the task.
	portBASE_TYPE retval;
	dataLocate->dataLCD = dataLCD;

	if ((retval = xTaskCreate(updateTaskLocate, taskNameLocate, I2C_STACK_SIZE, (void*)dataLocate, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
		VT_HANDLE_FATAL_ERROR(retval);
}

portBASE_TYPE sendTimerMsgLocate(structLocate* dataLocate, portTickType ticksElapsed, portTickType ticksToBlock)
{
	if (dataLocate == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	
	msgLocate bufferLocate;
	bufferLocate.length = sizeof(ticksElapsed);

	if (bufferLocate.length > maxLenLocate)
		VT_HANDLE_FATAL_ERROR(bufferLocate.length);

	memcpy(bufferLocate.buf, (char*)&ticksElapsed, sizeof(ticksElapsed));
	bufferLocate.msgType = msgLocateTimer;
	return(xQueueSend(dataLocate->inQ, (void*) (&bufferLocate),ticksToBlock));
}

portBASE_TYPE sendValueMsgLocate(structLocate* dataLocate, uint8_t msgType, uint16_t value, portTickType ticksToBlock)
{
	msgLocate bufferLocate;

	if (dataLocate == NULL)
		VT_HANDLE_FATAL_ERROR(0);

	bufferLocate.length = sizeof(value);

	if (bufferLocate.length > maxLenLocate)
		VT_HANDLE_FATAL_ERROR(bufferLocate.length);
	
	memcpy(bufferLocate.buf, (char*)&value, sizeof(value));
	bufferLocate.msgType = msgType;
	return(xQueueSend(dataLocate->inQ, (void*)(&bufferLocate), ticksToBlock));
}

static portTASK_FUNCTION(updateTaskLocate, pvParameters)
{
	structLocate* param = (structLocate*)pvParameters;
	structLCD* dataLCD = param->dataLCD;
	
	// String buffer for printing.
	char bufferLCD[maxLenLCD + 1];
	
	// Buffer for receiving messages.
	msgLocate msgBuffer;
	
	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from either a timer or sensor task.
		if (xQueueReceive(param->inQ, (void*)&msgBuffer, portMAX_DELAY) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// Now, based on the type of the message and the state, do different things.
		switch(getMsgTypeLocate(&msgBuffer))
		{
			case msgLocateTimer:
			{
				locateRover();
				switch(curStateGoal)
				{
				case none:
				{
					execNone();
					break;
				}
				case roam:
				{
					execRoam();
					break;
				}
				case followWall:
				{
					execFollowWall();
					break;
				}
				case turnCornerLeft:
				{
					execTurnCornerLeft();
					break;
				}
				case turnCornerRight:
				{
					execTurnCornerRight();
					break;
				}
				case turnAround:
				{
					execTurnAround();
					break;
				}
				case moveTowardObject:
				{
					execMoveTowardObject();
					break;	
				}
				case moveTowardRamp:
				{
					execMoveTowardRamp();
					break;
				}
				case alignWithRamp:
				{
					execAlignWithRamp();
					break;
				}
				case moveOverRamp:
				{
					execMoveOverRamp();
					break;
				}
				default:
				{
					// Error...
					break;
				}
				}
			default:
			{
				locateData[msgBuffer.msgType] = msgBuffer.buf[1];
				break;	
			}
			}
		}
	}
}

// Populate the datastructure of objects around the rover.
void locateRover()
{
	const float horizSensorDistance = 2;
	const float vertSensorDistance = 2;

	uint8_t i = 0;
	for(i = 0; i < NUMBER_SENSORS - 4; i = i + 2)
	{
		uint8_t curRoverSide = 0;
		float distanceSensor0 = locateData[i];
		float distanceSensor1 = locateData[i + 1];

		Object* curSideObject = &(sideObject[curRoverSide]);

		// Calculate the distance to the object.
		curSideObject->distance = (distanceSensor0 + distanceSensor1)/2;

		// Calculate the angle to the object.
		if(distanceSensor0 > distanceSensor1)
			curSideObject->angle =  90 + atan(abs(distanceSensor0 - distanceSensor1)/horizSensorDistance);	
		else
			curSideObject->angle = atan(horizSensorDistance/(abs(distanceSensor0 - distanceSensor1)));

		// Now do the next rover side.
		curRoverSide = curRoverSide + 1;
	}
}

void updateRamp()
{

}

void execNone()
{

}

void execRoam()
{

}

void execFollowWall()
{

}

void execTurnCornerLeft()
{

}

void execTurnCornerRight()
{

}

void execTurnAround()
{

}

void execMoveTowardObject()
{

}

void execMoveTowardRamp()
{

}

void execAlignWithRamp()
{

}

void execMoveOverRamp()
{

}


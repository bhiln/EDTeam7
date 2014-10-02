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

typedef struct __DataSensorLocate
{
	uint16_t sensorIR00;
	uint16_t sensorIR01;
	uint16_t sensorIR10;
	uint16_t sensorIR11;
	uint16_t sensorIR20;
	uint16_t sensorIR21;
	uint16_t sensorIR30;
	uint16_t sensorIR31;
	uint16_t sensorIR40;
	uint16_t sensorIR41;
	uint16_t sensorAC00;
} DataSensorLocate;

int getMsgTypeLocate(msgLocate *Buffer) {return(Buffer->msgType);}

static portTASK_FUNCTION(updateTaskLocate, pvParameters);

void startTaskLocate(structLocate* dataLocate, unsigned portBASE_TYPE uxPriority, structLCD* dataLCD)
{

	// Create the queue that will be used to talk to this task.
	if ((dataLocate->inQ = xQueueCreate(queueLenLocate, sizeof(msgLocate))) == NULL)
		VT_HANDLE_FATAL_ERROR(0);

	// Create the task.
	portBASE_TYPE retval;
	dataLocate->dataLCD = dataLCD;

	if ((retval = xTaskCreate(updateTaskLocate, locateName, I2C_STACK_SIZE, (void*)dataLocate, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
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
	bufferLocate.msgType = msgTypeTimerLocate;
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
	DataSensorLocate dataSensorLocate;

	structLocate* param = (structLocate*)pvParameters;
	structLCD* dataLCD = param->dataLCD;
	
	// String buffer for printing.
	char bufferLCD[maxLenLCD + 1];
	
	// Buffer for receiving messages.
	msgLocate msgBuffer;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from either a timer or from an I2C operation.
		if (xQueueReceive(param->inQ, (void*)&msgBuffer, portMAX_DELAY) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// Now, based on the type of the message and the state, do different things.
		switch(getMsgTypeLocate(&msgBuffer))
		{
			case msgTypeTimerLocate:
			{
				// Send the Command task simulated data.
				break;
			}
			case msgTypeIR00:
			{
				dataSensorLocate.sensorIR00 = msgBuffer.buf[0];
				break;
			}
			case msgTypeIR01:
			{
				dataSensorLocate.sensorIR01 = msgBuffer.buf[0];
				break;
			}
			case msgTypeIR10:
			{
				dataSensorLocate.sensorIR10 = msgBuffer.buf[0];
				break;
			}
			case msgTypeIR11:
			{
				dataSensorLocate.sensorIR11 = msgBuffer.buf[0];
				break;
			}
			case msgTypeIR20:
			{
				dataSensorLocate.sensorIR20 = msgBuffer.buf[0];
				break;
			}
			case msgTypeIR21:
			{
				dataSensorLocate.sensorIR21 = msgBuffer.buf[0];
				break;
			}
			case msgTypeIR30:
			{
				dataSensorLocate.sensorIR30 = msgBuffer.buf[0];
				break;
			}
			case msgTypeIR31:
			{
				dataSensorLocate.sensorIR31 = msgBuffer.buf[0];
				break;
			}
			case msgTypeIR40:
			{
				dataSensorLocate.sensorIR40 = msgBuffer.buf[0];
				break;
			}
			case msgTypeIR41:
			{
				dataSensorLocate.sensorIR41 = msgBuffer.buf[0];
				break;
			}
			case msgTypeAC00:
			{
				dataSensorLocate.sensorAC00 = msgBuffer.buf[0];
				break;
			}
			default:
			{
				// error
			}
		}
	}
}


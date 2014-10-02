/*------------------------------------------------------------------------------
 * File:		taskCommand.c
 * Authors: 		FreeRTOS, Igor Janjic
 * Description:		Sends rover commands to the motor commander.
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
#include "taskCommand.h"
#include "I2CTaskMsgTypes.h"
#include "debug.h"

/*------------------------------------------------------------------------------
 * Configuration
 */
// Length of the queue to this task.
#define queueLenCommand 10

// Stack sizes.
#define BASE_STACK 3
#if PRINTF_VERSION == 1
#define I2C_STACK_SIZE		((BASE_STACK+5)*configMINIMAL_STACK_SIZE)
#else
#define I2C_STACK_SIZE		(BASE_STACK*configMINIMAL_STACK_SIZE)
#endif

// Actual data structure that is sent in a message.
typedef struct __msgCommand
{
	uint8_t msgType;
	uint8_t	length;
	uint8_t objectID;
	uint8_t buf[maxLenCommand + 1];
} msgCommand;

const uint8_t commandRLF[] = {0x11};
const uint8_t commandSSR[] = {0x31};

int getMsgTypeCommand(msgCommand *Buffer) {return(Buffer->msgType);}

static portTASK_FUNCTION(updateTaskCommand, pvParameters);

void startTaskCommand(structCommand* dataCommand, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0, structLCD* dataLCD)
{

	// Create the queue that will be used to talk to this task.
	if ((dataCommand->inQ = xQueueCreate(queueLenCommand, sizeof(msgCommand))) == NULL)
		VT_HANDLE_FATAL_ERROR(0);

	// Create the task.
	portBASE_TYPE retval;
	dataCommand->devI2C0 = devI2C0;
	dataCommand->dataLCD = dataLCD;

	if ((retval = xTaskCreate(updateTaskCommand, commandName, I2C_STACK_SIZE, (void*)dataCommand, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
		VT_HANDLE_FATAL_ERROR(retval);
}


portBASE_TYPE sendTimerMsgCommand(structCommand* dataCommand, portTickType ticksElapsed, portTickType ticksToBlock)
{
	if (dataCommand == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	
	msgCommand bufferCommand;
	bufferCommand.length = sizeof(ticksElapsed);

	if (bufferCommand.length > maxLenCommand)
		VT_HANDLE_FATAL_ERROR(bufferCommand.length);

	memcpy(bufferCommand.buf, (char*)&ticksElapsed, sizeof(ticksElapsed));
	bufferCommand.msgType = msgTypeTimerCommand;
	return(xQueueSend(dataCommand->inQ, (void*) (&bufferCommand),ticksToBlock));
}

portBASE_TYPE sendValueMsgCommand(structCommand* dataCommand, uint8_t msgType, uint16_t value, portTickType ticksToBlock)
{
	msgCommand bufferCommand;

	if (dataCommand == NULL)
		VT_HANDLE_FATAL_ERROR(0);

	bufferCommand.length = sizeof(value);

	if (bufferCommand.length > maxLenCommand)
		VT_HANDLE_FATAL_ERROR(bufferCommand.length);
	
	memcpy(bufferCommand.buf, (char*)&value, sizeof(value));
	bufferCommand.msgType = msgType;
	return(xQueueSend(dataCommand->inQ, (void*)(&bufferCommand), ticksToBlock));
}

static portTASK_FUNCTION(updateTaskCommand, pvParameters)
{
	structCommand* param = (structCommand*)pvParameters;
	vtI2CStruct* devI2C0 = param->devI2C0;
	structLCD* dataLCD = param->dataLCD;
	
	// String buffer for printing.
	char bufferLCD[maxLenLCD + 1];
	
	// Buffer for receiving messages.
	msgCommand msgBuffer;

	uint8_t dataReady = 0;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from either a timer or from an I2C operation.
		if (xQueueReceive(param->inQ, (void*)&msgBuffer, portMAX_DELAY) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// Now, based on the type of the message and the state, do different things.
		switch(getMsgTypeCommand(&msgBuffer))
		{
			case msgTypeTimerCommand:
			{
				if(dataReady == 0)
					dataReady = 1;
				else
					dataReady = 0;
				break;
			}
			// Get all data.
			case msgTypeDist:
			{
				break;
			}
			case msgTypeAngle:
			{
				break;
			}
			default:
			{
				// error
			}
		}

		// Command ready to be sent so send it.	
		if(dataReady)
		{
			if (vtI2CEnQ(devI2C0, msgTypeCommand, SLAVE_ADDR, sizeof(commandRLF), commandRLF, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgTypeCommandSpeed, SLAVE_ADDR, sizeof(commandSSR), commandSSR, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);

		}
	}
}


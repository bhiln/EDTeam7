/*------------------------------------------------------------------------------
 * File:		i2cIR0.c
 * Authors: 	FreeRTOS, Igor Janjic
 * Description:	Reads the IR0 sensor data from the PIC via I2C.
 *----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * Includes
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

// Schedular includes.
#include "FreeRTOS.h"
#include "task.h"
#include "projdefs.h"
#include "semphr.h"
#include "lpc17xx_gpio.h"

#include "vtUtilities.h"
#include "vtI2C.h"
#include "LCDtask.h"
#include "i2cIR0.h"
#include "I2CTaskMsgTypes.h"
#include "debug.h"

/*------------------------------------------------------------------------------
 * Configuration
 */
// Length of the queue to this task.
#define vtIR0QLen 10

// Stack sizes.
#define baseStack 3
#if PRINTF_VERSION == 1
#define i2cSTACK_SIZE		((baseStack+5)*configMINIMAL_STACK_SIZE)
#else
#define i2cSTACK_SIZE		(baseStack*configMINIMAL_STACK_SIZE)
#endif

/*------------------------------------------------------------------------------
 * I2CIR0 Task
 */ 
// Actual data structure that is sent in a message.
typedef struct __vtIR0Msg {
	uint8_t msgType; 				// Type of message
	uint8_t	length;	 				// Length of the message
	uint8_t buf[vtIR0MaxLen + 1]; 	// On the way in, message to be sent, on the way out, message received (if any)
} vtIR0Msg;

// The i2cIR0 task.
static portTASK_FUNCTION_PROTO(vi2cIR0UpdateTask, pvParameters);

void vStarti2cIR0Task(vtIR0Struct *params, unsigned portBASE_TYPE uxPriority, vtI2CStruct *i2c, vtLCDStruct *lcd)
{
	// Create the queue that will be used to talk to this task.
	if ((params->inQ = xQueueCreate(vtIR0QLen, sizeof(vtIR0Msg))) == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	// Start the task.
	portBASE_TYPE retval;
	params->dev = i2c;
	params->lcdData = lcd;
	if ((retval = xTaskCreate( vi2cIR0UpdateTask, ( signed char * ) "i2cIR0", i2cSTACK_SIZE, (void *) params, uxPriority, ( xTaskHandle * ) NULL )) != pdPASS) {
		VT_HANDLE_FATAL_ERROR(retval);
	}
}

portBASE_TYPE SendIR0TimerMsg(vtIR0Struct *ir0Data, portTickType ticksElapsed, portTickType ticksToBlock)
{
	if (ir0Data == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	vtIR0Msg ir0Buffer;
	ir0Buffer.length = sizeof(ticksElapsed);
	if (ir0Buffer.length > vtIR0MaxLen) {
		// no room for this message
		VT_HANDLE_FATAL_ERROR(ir0Buffer.length);
	}
	memcpy(ir0Buffer.buf, (char *)&ticksElapsed, sizeof(ticksElapsed));
	ir0Buffer.msgType = IR0MsgTypeTimer;
	return(xQueueSend(ir0Data->inQ,(void *) (&ir0Buffer),ticksToBlock));
}

portBASE_TYPE SendIR0ValueMsg(vtIR0Struct *ir0Data, uint8_t msgType, uint8_t value, portTickType ticksToBlock)
{
	vtIR0Msg ir0Buffer;

	if (ir0Data == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	ir0Buffer.length = sizeof(value);
	if (ir0Buffer.length > vtIR0MaxLen) {
		// no room for this message
		VT_HANDLE_FATAL_ERROR(ir0Buffer.length);
	}
	memcpy(ir0Buffer.buf, (char *)&value, sizeof(value));
	ir0Buffer.msgType = msgType;
	return(xQueueSend(ir0Data->inQ, (void *) (&ir0Buffer), ticksToBlock));
}

int getMsgType(vtIR0Msg *Buffer)
{
	return(Buffer->msgType);
}

//uint8_t getValue(vtTempMsg *Buffer)
//{
//	uint8_t *ptr = (uint8_t *) Buffer->buf;
//	return(*ptr);
//}

// I2C commands for the IR0 sensor.
const uint8_t i2cCmdReadIR0Byte0[]= {0xAA};
const uint8_t i2cCmdReadIR0Byte1[]= {0xA8};

// Definitions of the states for the FSM below.
const uint8_t fsmStateReadIR0Byte0 = 0;
const uint8_t fsmStateReadIR0Byte1 = 1;

// This is the actual task that is run.
static portTASK_FUNCTION(vi2cIR0UpdateTask, pvParameters)
{
	uint8_t voltageByte0 = 0;
	uint8_t voltageByte1 = 0;

	// Get the parameters
	vtIR0Struct *param = (vtIR0Struct *) pvParameters;
	
	// Get the I2C device pointer
	vtI2CStruct *devPtr = param->dev;
	
	// Get the LCD information pointer
	vtLCDStruct *lcdData = param->lcdData;
	
	// String buffer for printing
	char lcdBuffer[vtLCDMaxLen+1];
	
	// Buffer for receiving messages
	vtIR0Msg msgBuffer;
	uint8_t currentState;

	currentState = fsmStateReadIR0Byte0;
	// Like all good tasks, this should never exit
	for(;;)
	{
		// Wait for a message from either a timer or from an I2C operation
		if (xQueueReceive(param->inQ, (void *) &msgBuffer, portMAX_DELAY) != pdTRUE) {
			VT_HANDLE_FATAL_ERROR(0);
		}
		// Now, based on the type of the message and the state, we decide on the new state and action to take
		switch(getMsgType(&msgBuffer))
		{
			case IR0MsgTypeTimer:
			{
				// Timer messages never change the state, they just cause an action (or not) 
				if (vtI2CEnQ(devPtr, vtI2CMsgTypeIR0ReadByte0, 0x4F, sizeof(i2cCmdReadIR0Byte0), i2cCmdReadIR0Byte0, 2) != pdTRUE)
				{
					VT_HANDLE_FATAL_ERROR(0);
				}
				if (vtI2CEnQ(devPtr, vtI2CMsgTypeIR0ReadByte1, 0x4F, sizeof(i2cCmdReadIR0Byte1), i2cCmdReadIR0Byte1, 2) != pdTRUE)
				{
					VT_HANDLE_FATAL_ERROR(0);
				}
				break;
			}
			case vtI2CMsgTypeIR0ReadByte0:
			{
				if (currentState == fsmStateReadIR0Byte0)
				{
					currentState = fsmStateReadIR0Byte1;
					voltageByte0 = msgBuffer.buf[0];
				}
				else
				{
					// unexpectedly received this message
				}
				break;
			}
			case vtI2CMsgTypeIR0ReadByte1:
			{
				if (currentState == fsmStateReadIR0Byte1)
				{  	
					unsigned int noReply = 0x11;
					currentState = fsmStateReadIR0Byte0;
					voltageByte1 = msgBuffer.buf[0];
					unsigned int voltage = 0;
					float voltF = 0;
					voltage = voltageByte0 << 8; // MSB
					voltage = voltage | voltageByte1;
					voltage = (float)(voltF);
					
					voltage = voltage/(1023/3.3);
					if(voltageByte0 == noReply)
						sprintf(lcdBuffer, "%d", noReply);
					sprintf(lcdBuffer, "%d", voltage);
					printf("%d\n", voltage);
					if (lcdData != NULL)
					{
						if (SendLCDPrintMsg(lcdData, strnlen(lcdBuffer, vtLCDMaxLen), lcdBuffer, portMAX_DELAY) != pdTRUE)
							VT_HANDLE_FATAL_ERROR(0);
						//if (SendLCDTimerMsg(lcdData, 10/portTICK_RATE_MS, portMAX_DELAY) != pdTRUE)
							//VT_HANDLE_FATAL_ERROR(0);
					}					
				}
				else
				{
					// unexpectedly received this message
				}
				break;
			}	
		}
	}
}

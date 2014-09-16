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

#include "vtUtilities.h"
#include "vtI2C.h"
#include "LCDtask.h"
#include "i2cIR0.h"
#include "I2CTaskMsgTypes.h"

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

uint8_t getValue(vtIR0Msg *Buffer)
{
	uint8_t *ptr = (uint8_t *) Buffer->buf;
	return(*ptr);
}

// I2C commands for the IR0 sensor.
const uint8_t i2cCmdInit[]= {0xAC,0x00};
const uint8_t i2cCmdStartConvert[]= {0xEE};
const uint8_t i2cCmdStopConvert[]= {0x22};
const uint8_t i2cCmdReadVals[]= {0xAA};
const uint8_t i2cCmdReadCnt[]= {0xA8};
const uint8_t i2cCmdReadSlope[]= {0xA9};

// Definitions of the states for the FSM below.
const uint8_t fsmStateInit1Sent = 0;
const uint8_t fsmStateInit2Sent = 1;
const uint8_t fsmStateIR0Read1 = 2;
const uint8_t fsmStateIR0Read2 = 3;
const uint8_t fsmStateIR0Read3 = 4;

// This is the actual task that is run.
static portTASK_FUNCTION(vi2cIR0UpdateTask, pvParameters)
{
	float voltage = 0.5;

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

	// Assumes that the I2C device (and thread) have already been initialized
	// This task is implemented as a Finite State Machine.  The incoming messages are examined to see
	//   whether or not the state should change.
	//
	// IR0 sensor configuration sequence (DS1621) Address 0x4F
	if (vtI2CEnQ(devPtr, vtI2CMsgTypeIR0Init, 0x4F, sizeof(i2cCmdInit), i2cCmdInit,0) != pdTRUE) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	currentState = fsmStateInit1Sent;
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
			case vtI2CMsgTypeIR0Init:
			{
				if (currentState == fsmStateInit1Sent) {
					currentState = fsmStateInit2Sent;
					// Must wait 10ms after writing to the temperature sensor's configuration registers(per sensor data sheet)
					vTaskDelay(10/portTICK_RATE_MS);
					// Tell it to start converting
					if (vtI2CEnQ(devPtr,vtI2CMsgTypeIR0Init,0x4F,sizeof(i2cCmdStartConvert),i2cCmdStartConvert,0) != pdTRUE) {
						VT_HANDLE_FATAL_ERROR(0);
					}
	
				} else 	if (currentState == fsmStateInit2Sent) {
					currentState = fsmStateIR0Read1;
	
				} else {
					// unexpectedly received this message
					VT_HANDLE_FATAL_ERROR(0);
				}
				break;
			}
			case IR0MsgTypeTimer: {
				// Timer messages never change the state, they just cause an action (or not) 
				if ((currentState != fsmStateInit1Sent) && (currentState != fsmStateInit2Sent)) 
				{
					if (vtI2CEnQ(devPtr, vtI2CMsgTypeIR0Read1, 0x4F, sizeof(i2cCmdReadVals), i2cCmdReadVals, 2) != pdTRUE)
					{
						VT_HANDLE_FATAL_ERROR(0);
					}
				}
				else
				{
					// just ignore timer messages until initialization is complete
				} 
				break;
			}
			case vtI2CMsgTypeIR0Read1:
			{
				if (currentState == fsmStateIR0Read1)
				{
					currentState = fsmStateIR0Read1;
					voltage = getValue(&msgBuffer);
					
					int dec1 = voltage;
					float f2 = voltage - dec1;
					int dec2 = trunc(f2*1000);

					//printf("%d.%01d V\n", dec1, dec2);
					sprintf(lcdBuffer, "%d.%01d", dec1, dec2);
					printf(lcdBuffer);
					if (lcdData != NULL)
					{
						if (SendLCDPrintMsg(lcdData, strnlen(lcdBuffer, vtLCDMaxLen), lcdBuffer, portMAX_DELAY) != pdTRUE)
							VT_HANDLE_FATAL_ERROR(0);
						//if (SendLCDTimerMsg(lcdData, 10/portTICK_RATE_MS, portMAX_DELAY) != pdTRUE)
							//VT_HANDLE_FATAL_ERROR(0);
					}
				}
				else {
					// unexpectedly received this message
					VT_HANDLE_FATAL_ERROR(0);
				}
				break;
			}	
		}
	}
}

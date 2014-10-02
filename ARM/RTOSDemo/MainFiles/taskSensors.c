/*------------------------------------------------------------------------------
 * File:		taskSensors.c
 * Authors: 		FreeRTOS, Igor Janjic
 * Description:		Reads the sensor data, processes it, and sends it to the
 * 			relevant tasks.
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
#include "vtI2C.h"
#include "taskLCD.h"
#include "taskSensors.h"
#include "I2CTaskMsgTypes.h"
#include "debug.h"

/*------------------------------------------------------------------------------
 * Configuration
 */
// Length of the queue to this task.
#define queueLenIR 10

// Stack sizes.
#define BASE_STACK 3
#if PRINTF_VERSION == 1
#define I2C_STACK_SIZE		((BASE_STACK+5)*configMINIMAL_STACK_SIZE)
#else
#define I2C_STACK_SIZE		(BASE_STACK*configMINIMAL_STACK_SIZE)
#endif

// Actual data structure that is sent in a message.
typedef struct __msgIR {
	uint8_t msgType;
	uint8_t	length;
	uint8_t buf[maxLenIR + 1];
} msgIR;

int getMsgType(msgIR *Buffer) {return(Buffer->msgType);}

// Global commands.
const uint8_t recvNoReply = 0x00;

// I2C commands for the IR00 sensor.
const uint8_t queryReadByte0IR00[] = {0x0A};
const uint8_t queryReadByte1IR00[] = {0x08};

// I2C commands for the IR01 sensor.
const uint8_t queryReadByte0IR01[] = {0x0C};
const uint8_t queryReadByte1IR01[] = {0x0D};

// I2C commands for the IR10 sensor.
const uint8_t queryReadByte0IR10[] = {0x1A};
const uint8_t queryReadByte1IR10[] = {0x1B};

// I2C commands for the IR11 sensor.
const uint8_t queryReadByte0IR11[] = {0x1C};
const uint8_t queryReadByte1IR11[] = {0x1D};

// I2C commands for the IR20 sensor.
const uint8_t queryReadByte0IR20[] = {0x2A};
const uint8_t queryReadByte1IR20[] = {0x2B};

// I2C commands for the IR21 sensor.
const uint8_t queryReadByte0IR21[] = {0x2C};
const uint8_t queryReadByte1IR21[] = {0x2D};

// I2C commands for the IR30 sensor.
const uint8_t queryReadByte0IR30[] = {0x3A};
const uint8_t queryReadByte1IR30[] = {0x3B};

// I2C commands for the IR31 sensor.
const uint8_t queryReadByte0IR31[] = {0x3C};
const uint8_t queryReadByte1IR31[] = {0x3D};

// I2C commands for the IR40 sensor.
const uint8_t queryReadByte0IR40[] = {0x4A};
const uint8_t queryReadByte1IR40[] = {0x4B};

// I2C commands for the IR41 sensor.
const uint8_t queryReadByte0IR41[] = {0x4C};
const uint8_t queryReadByte1IR41[] = {0x4D};

// I2C commands for the AC00 sensor.
const uint8_t queryReadByte0AC00[] = {0x5A};
const uint8_t queryReadByte1AC00[] = {0x5B};

// Definitions of the states for the FSM for IR00.
const uint8_t stateReadByte0IR00 = 0;
const uint8_t stateReadByte1IR00 = 1;
const uint8_t stateReadByte0IR01 = 0;
const uint8_t stateReadByte1IR01 = 1;
const uint8_t stateReadByte0IR10 = 0;
const uint8_t stateReadByte1IR10 = 1;
const uint8_t stateReadByte0IR11 = 0;
const uint8_t stateReadByte1IR11 = 1;
const uint8_t stateReadByte0IR20 = 0;
const uint8_t stateReadByte1IR20 = 1;
const uint8_t stateReadByte0IR21 = 0;
const uint8_t stateReadByte1IR21 = 1;
const uint8_t stateReadByte0IR30 = 0;
const uint8_t stateReadByte1IR30 = 1;
const uint8_t stateReadByte0IR31 = 0;
const uint8_t stateReadByte1IR31 = 1;
const uint8_t stateReadByte0IR40 = 0;
const uint8_t stateReadByte1IR40 = 1;
const uint8_t stateReadByte0IR41 = 0;
const uint8_t stateReadByte1IR41 = 1;
const uint8_t stateReadByte0AC00 = 0;
const uint8_t stateReadByte1AC00 = 1;

// Sensor task update functions.
static portTASK_FUNCTION_PROTO(updateTaskIR00, pvParameters);
static portTASK_FUNCTION_PROTO(updateTaskIR01, pvParameters);
static portTASK_FUNCTION_PROTO(updateTaskIR10, pvParameters);
static portTASK_FUNCTION_PROTO(updateTaskIR11, pvParameters);
static portTASK_FUNCTION_PROTO(updateTaskIR20, pvParameters);
static portTASK_FUNCTION_PROTO(updateTaskIR21, pvParameters);
static portTASK_FUNCTION_PROTO(updateTaskIR30, pvParameters);
static portTASK_FUNCTION_PROTO(updateTaskIR31, pvParameters);
static portTASK_FUNCTION_PROTO(updateTaskIR40, pvParameters);
static portTASK_FUNCTION_PROTO(updateTaskIR41, pvParameters);
static portTASK_FUNCTION_PROTO(updateTaskAC00, pvParameters);

void startTaskSensor(uint8_t sensor, structSensor* dataSensor, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0, structLCD* dataLCD)
{
	// Create the queue that will be used to talk to this task.
	if ((dataSensor->inQ = xQueueCreate(queueLenIR, sizeof(msgIR))) == NULL)
		VT_HANDLE_FATAL_ERROR(0);

	// Create the task.
	portBASE_TYPE retval;
	dataSensor->devI2C0 = devI2C0;
	dataSensor->dataLCD = dataLCD;

	switch(sensor)
	{
	case sensorIR00:
	{
		if ((retval = xTaskCreate(updateTaskIR00, sensorNameIR00, I2C_STACK_SIZE, (void*) dataSensor, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
			VT_HANDLE_FATAL_ERROR(retval);
		break;
	}
	case sensorIR01:
	{
		if ((retval = xTaskCreate(updateTaskIR01, sensorNameIR01, I2C_STACK_SIZE, (void*) dataSensor, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
			VT_HANDLE_FATAL_ERROR(retval);
		break;
	}
	case sensorIR10:
	{
		if ((retval = xTaskCreate(updateTaskIR10, sensorNameIR10, I2C_STACK_SIZE, (void*) dataSensor, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
			VT_HANDLE_FATAL_ERROR(retval);
		break;
	}
	case sensorIR11:
	{
		if ((retval = xTaskCreate(updateTaskIR11, sensorNameIR11, I2C_STACK_SIZE, (void*) dataSensor, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
			VT_HANDLE_FATAL_ERROR(retval);
		break;
	}
	case sensorIR20:
	{
		if ((retval = xTaskCreate(updateTaskIR20, sensorNameIR20, I2C_STACK_SIZE, (void*) dataSensor, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
			VT_HANDLE_FATAL_ERROR(retval);
		break;
	}
	case sensorIR21:
	{
		if ((retval = xTaskCreate(updateTaskIR21, sensorNameIR21, I2C_STACK_SIZE, (void*) dataSensor, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
			VT_HANDLE_FATAL_ERROR(retval);
		break;
	}
	case sensorIR30:
	{
		if ((retval = xTaskCreate(updateTaskIR30, sensorNameIR30, I2C_STACK_SIZE, (void*) dataSensor, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
			VT_HANDLE_FATAL_ERROR(retval);
		break;
	}
	case sensorIR31:
	{
		if ((retval = xTaskCreate(updateTaskIR31, sensorNameIR31, I2C_STACK_SIZE, (void*) dataSensor, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
			VT_HANDLE_FATAL_ERROR(retval);
		break;
	}
	case sensorIR40:
	{
		if ((retval = xTaskCreate(updateTaskIR40, sensorNameIR40, I2C_STACK_SIZE, (void*) dataSensor, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
			VT_HANDLE_FATAL_ERROR(retval);
		break;
	}
	case sensorIR41:
	{
		if ((retval = xTaskCreate(updateTaskIR41, sensorNameIR41, I2C_STACK_SIZE, (void*) dataSensor, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
			VT_HANDLE_FATAL_ERROR(retval);
		break;
	}
	case sensorAC00:
	{
		if ((retval = xTaskCreate(updateTaskAC00, sensorNameAC00, I2C_STACK_SIZE, (void*) dataSensor, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
			VT_HANDLE_FATAL_ERROR(retval);
		break;
	}
	default:
	{
		// error
	}
	}
}

portBASE_TYPE sendTimerMsgSensor(structSensor* dataSensor, uint8_t msgType, portTickType ticksElapsed, portTickType ticksToBlock)
{
	if (dataSensor == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	
	msgIR bufferSensor;
	bufferSensor.length = sizeof(ticksElapsed);

	if (bufferSensor.length > maxLenIR)
		VT_HANDLE_FATAL_ERROR(bufferSensor.length);

	memcpy(bufferSensor.buf, (char*)&ticksElapsed, sizeof(ticksElapsed));
	bufferSensor.msgType = msgType;
	return(xQueueSend(dataSensor->inQ, (void*) (&bufferSensor),ticksToBlock));
}

portBASE_TYPE sendValueMsgSensor(structSensor* dataSensor, uint8_t msgType, uint8_t value, portTickType ticksToBlock)
{
	msgIR bufferSensor;

	if (dataSensor == NULL)
		VT_HANDLE_FATAL_ERROR(0);

	bufferSensor.length = sizeof(value);

	if (bufferSensor.length > maxLenIR)
		VT_HANDLE_FATAL_ERROR(bufferSensor.length);
	
	memcpy(bufferSensor.buf, (char*)&value, sizeof(value));
	bufferSensor.msgType = msgType;
	return(xQueueSend(dataSensor->inQ, (void*)(&bufferSensor), ticksToBlock));
}

static portTASK_FUNCTION(updateTaskIR00, pvParameters)
{
	uint8_t voltageByte0IR00 = 0;
	uint8_t voltageByte1IR00 = 0;

	structSensor* param = (structSensor*)pvParameters;
	vtI2CStruct* devI2C0 = param->devI2C0;
	structLCD* dataLCD = param->dataLCD;
	
	// String buffer for printing.
	char bufferLCD[maxLenLCD + 1];
	
	// Buffer for receiving messages.
	msgIR msgBuffer;

	uint8_t currentState = stateReadByte0IR00;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from either a timer or from an I2C operation.
		if (xQueueReceive(param->inQ, (void*)&msgBuffer, portMAX_DELAY) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// Now, based on the type of the message and the state, we decide on the new state and action to take.
		switch(getMsgType(&msgBuffer))
		{
		case msgTypeTimerIR00:
		{
			// Timer messages never change the state, they just cause an action (or not).
			if (vtI2CEnQ(devI2C0, msgTypeIR00ReadByte0, SLAVE_ADDR, sizeof(queryReadByte0IR00), queryReadByte0IR00, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgTypeIR00ReadByte1, SLAVE_ADDR, sizeof(queryReadByte1IR00), queryReadByte1IR00, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			break;
		}
		case msgTypeIR00ReadByte0:
		{
			if (currentState == stateReadByte0IR00)
			{
				currentState = stateReadByte1IR00;
				voltageByte0IR00 = msgBuffer.buf[0];
			}
			else
			{
				// Unexpectedly received this message.
			}
			break;
		}
		case msgTypeIR00ReadByte1:
		{
			if (currentState == stateReadByte1IR00)
			{  	
				currentState = stateReadByte0IR00;
				voltageByte1IR00 = msgBuffer.buf[0];

				unsigned int voltage = 0;
				float voltF = 0;
				voltage = voltageByte0IR00 << 8; // MSB
				voltage = voltage | voltageByte1IR00;
				voltage = (float)(voltF);
				
				voltage = voltage/(1023/3.3);
				if(voltageByte0IR00 == recvNoReply)
					sprintf(bufferLCD, "%d", recvNoReply);
				else
					sprintf(bufferLCD, "%d", voltage);
				printf("%d\n", voltage);
				if (dataLCD != NULL)
				{
					if (SendLCDPrintMsg(dataLCD, strnlen(bufferLCD, maxLenLCD), bufferLCD, portMAX_DELAY) != pdTRUE)
						VT_HANDLE_FATAL_ERROR(0);
					//if (SendLCDTimerMsg(dataLCD, 10/portTICK_RATE_MS, portMAX_DELAY) != pdTRUE)
						//VT_HANDLE_FATAL_ERROR(0);
				}					
			}
			else
			{
				// unexpectedly received this message
			}
			break;
		}	
		default:
		{
			// error
		}
		}
	}
}

static portTASK_FUNCTION(updateTaskIR01, pvParameters)
{
	uint8_t voltByte0IR01 = 0;
	uint8_t voltByte1IR01 = 0;

	structSensor* param = (structSensor*)pvParameters;
	vtI2CStruct* devI2C0 = param->devI2C0;
	structLCD* dataLCD = param->dataLCD;
	
	// String buffer for printing.
	char bufferLCD[maxLenLCD + 1];
	
	// Buffer for receiving messages.
	msgIR msgBuffer;

	uint8_t currentState = stateReadByte0IR01;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from either a timer or from an I2C operation.
		if (xQueueReceive(param->inQ, (void*)&msgBuffer, portMAX_DELAY) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// Now, based on the type of the message and the state, we decide on the new state and action to take.
		switch(getMsgType(&msgBuffer))
		{
		case msgTypeTimerIR01:
		{
			// Timer messages never change the state, they just cause an action (or not).
			if (vtI2CEnQ(devI2C0, msgTypeIR01ReadByte0, SLAVE_ADDR, sizeof(queryReadByte0IR01), queryReadByte0IR01, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgTypeIR01ReadByte1, SLAVE_ADDR, sizeof(queryReadByte1IR01), queryReadByte1IR01, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			break;
		}
		case msgTypeIR01ReadByte0:
		{
			if (currentState == stateReadByte0IR01)
			{
				currentState = stateReadByte1IR01;
				voltByte0IR01 = msgBuffer.buf[0];
			}
			else
			{
				// Unexpectedly received this message.
			}
			break;
		}
		case msgTypeIR01ReadByte1:
		{
			if (currentState == stateReadByte1IR01)
			{  	
				currentState = stateReadByte0IR01;
				voltByte1IR01 = msgBuffer.buf[0];

				unsigned int volt = 0;
				float voltF = 0;
				volt = voltByte0IR01 << 8; // MSB
				volt = volt | voltByte1IR01;
				volt = (float)(voltF);
				
				volt = volt/(1023/3.3);
			}
			else
			{
				// unexpectedly received this message
			}
			break;
		}	
		default:
		{
			// error
		}
		}
	}
}

static portTASK_FUNCTION(updateTaskIR10, pvParameters)
{
	uint8_t voltByte0IR10 = 0;
	uint8_t voltByte1IR10 = 0;

	structSensor* param = (structSensor*)pvParameters;
	vtI2CStruct* devI2C0 = param->devI2C0;
	structLCD* dataLCD = param->dataLCD;
	
	// String buffer for printing.
	char bufferLCD[maxLenLCD + 1];
	
	// Buffer for receiving messages.
	msgIR msgBuffer;

	uint8_t currentState = stateReadByte0IR10;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from either a timer or from an I2C operation.
		if (xQueueReceive(param->inQ, (void*)&msgBuffer, portMAX_DELAY) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// Now, based on the type of the message and the state, we decide on the new state and action to take.
		switch(getMsgType(&msgBuffer))
		{
		case msgTypeTimerIR10:
		{
			// Timer messages never change the state, they just cause an action (or not).
			if (vtI2CEnQ(devI2C0, msgTypeIR10ReadByte0, SLAVE_ADDR, sizeof(queryReadByte0IR10), queryReadByte0IR10, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgTypeIR10ReadByte1, SLAVE_ADDR, sizeof(queryReadByte1IR10), queryReadByte1IR10, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			break;
		}
		case msgTypeIR10ReadByte0:
		{
			if (currentState == stateReadByte0IR10)
			{
				currentState = stateReadByte1IR10;
				voltByte0IR10 = msgBuffer.buf[0];
			}
			else
			{
				// Unexpectedly received this message.
			}
			break;
		}
		case msgTypeIR10ReadByte1:
		{
			if (currentState == stateReadByte1IR10)
			{  	
				currentState = stateReadByte0IR10;
				voltByte1IR10 = msgBuffer.buf[0];

				unsigned int volt = 0;
				float voltF = 0;
				volt = voltByte0IR10 << 8; // MSB
				volt = volt | voltByte1IR10;
				volt = (float)(voltF);
				
				volt = volt/(1023/3.3);
			}
			else
			{
				// unexpectedly received this message
			}
			break;
		}	
		default:
		{
			// error
		}
		}
	}
}

static portTASK_FUNCTION(updateTaskIR11, pvParameters)
{
	uint8_t voltByte0IR11 = 0;
	uint8_t voltByte1IR11 = 0;

	structSensor* param = (structSensor*)pvParameters;
	vtI2CStruct* devI2C0 = param->devI2C0;
	structLCD* dataLCD = param->dataLCD;
	
	// String buffer for printing.
	char bufferLCD[maxLenLCD + 1];
	
	// Buffer for receiving messages.
	msgIR msgBuffer;

	uint8_t currentState = stateReadByte0IR11;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from either a timer or from an I2C operation.
		if (xQueueReceive(param->inQ, (void*)&msgBuffer, portMAX_DELAY) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// Now, based on the type of the message and the state, we decide on the new state and action to take.
		switch(getMsgType(&msgBuffer))
		{
		case msgTypeTimerIR11:
		{
			// Timer messages never change the state, they just cause an action (or not).
			if (vtI2CEnQ(devI2C0, msgTypeIR11ReadByte0, SLAVE_ADDR, sizeof(queryReadByte0IR11), queryReadByte0IR11, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgTypeIR11ReadByte1, SLAVE_ADDR, sizeof(queryReadByte1IR11), queryReadByte1IR11, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			break;
		}
		case msgTypeIR11ReadByte0:
		{
			if (currentState == stateReadByte0IR11)
			{
				currentState = stateReadByte1IR11;
				voltByte0IR11 = msgBuffer.buf[0];
			}
			else
			{
				// Unexpectedly received this message.
			}
			break;
		}
		case msgTypeIR11ReadByte1:
		{
			if (currentState == stateReadByte1IR11)
			{  	
				currentState = stateReadByte0IR11;
				voltByte1IR11 = msgBuffer.buf[0];

				unsigned int volt = 0;
				float voltF = 0;
				volt = voltByte0IR11 << 8; // MSB
				volt = volt | voltByte1IR11;
				volt = (float)(voltF);
				
				volt = volt/(1023/3.3);
			}
			else
			{
				// unexpectedly received this message
			}
			break;
		}	
		default:
		{
			// error
		}
		}
	}
}

static portTASK_FUNCTION(updateTaskIR20, pvParameters)
{
	uint8_t voltByte0IR20 = 0;
	uint8_t voltByte1IR20 = 0;

	structSensor* param = (structSensor*)pvParameters;
	vtI2CStruct* devI2C0 = param->devI2C0;
	structLCD* dataLCD = param->dataLCD;
	
	// String buffer for printing.
	char bufferLCD[maxLenLCD + 1];
	
	// Buffer for receiving messages.
	msgIR msgBuffer;

	uint8_t currentState = stateReadByte0IR20;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from either a timer or from an I2C operation.
		if (xQueueReceive(param->inQ, (void*)&msgBuffer, portMAX_DELAY) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// Now, based on the type of the message and the state, we decide on the new state and action to take.
		switch(getMsgType(&msgBuffer))
		{
		case msgTypeTimerIR20:
		{
			// Timer messages never change the state, they just cause an action (or not).
			if (vtI2CEnQ(devI2C0, msgTypeIR20ReadByte0, SLAVE_ADDR, sizeof(queryReadByte0IR20), queryReadByte0IR20, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgTypeIR20ReadByte1, SLAVE_ADDR, sizeof(queryReadByte1IR20), queryReadByte1IR20, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			break;
		}
		case msgTypeIR20ReadByte0:
		{
			if (currentState == stateReadByte0IR20)
			{
				currentState = stateReadByte1IR20;
				voltByte0IR20 = msgBuffer.buf[0];
			}
			else
			{
				// Unexpectedly received this message.
			}
			break;
		}
		case msgTypeIR20ReadByte1:
		{
			if (currentState == stateReadByte1IR20)
			{  	
				currentState = stateReadByte0IR20;
				voltByte1IR20 = msgBuffer.buf[0];

				unsigned int volt = 0;
				float voltF = 0;
				volt = voltByte0IR20 << 8; // MSB
				volt = volt | voltByte1IR20;
				volt = (float)(voltF);
				
				volt = volt/(1023/3.3);
			}
			else
			{
				// unexpectedly received this message
			}
			break;
		}	
		default:
		{
			// error
		}
		}
	}
}

static portTASK_FUNCTION(updateTaskIR21, pvParameters)
{
	uint8_t voltByte0IR21 = 0;
	uint8_t voltByte1IR21 = 0;

	structSensor* param = (structSensor*)pvParameters;
	vtI2CStruct* devI2C0 = param->devI2C0;
	structLCD* dataLCD = param->dataLCD;
	
	// String buffer for printing.
	char bufferLCD[maxLenLCD + 1];
	
	// Buffer for receiving messages.
	msgIR msgBuffer;

	uint8_t currentState = stateReadByte0IR21;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from either a timer or from an I2C operation.
		if (xQueueReceive(param->inQ, (void*)&msgBuffer, portMAX_DELAY) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// Now, based on the type of the message and the state, we decide on the new state and action to take.
		switch(getMsgType(&msgBuffer))
		{
		case msgTypeTimerIR21:
		{
			// Timer messages never change the state, they just cause an action (or not).
			if (vtI2CEnQ(devI2C0, msgTypeIR21ReadByte0, SLAVE_ADDR, sizeof(queryReadByte0IR21), queryReadByte0IR21, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgTypeIR21ReadByte1, SLAVE_ADDR, sizeof(queryReadByte1IR21), queryReadByte1IR21, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			break;
		}
		case msgTypeIR21ReadByte0:
		{
			if (currentState == stateReadByte0IR21)
			{
				currentState = stateReadByte1IR21;
				voltByte0IR21 = msgBuffer.buf[0];
			}
			else
			{
				// Unexpectedly received this message.
			}
			break;
		}
		case msgTypeIR21ReadByte1:
		{
			if (currentState == stateReadByte1IR21)
			{  	
				currentState = stateReadByte0IR21;
				voltByte1IR21 = msgBuffer.buf[0];

				unsigned int volt = 0;
				float voltF = 0;
				volt = voltByte0IR21 << 8; // MSB
				volt = volt | voltByte1IR21;
				volt = (float)(voltF);
				
				volt = volt/(1023/3.3);
			}
			else
			{
				// unexpectedly received this message
			}
			break;
		}	
		default:
		{
			// error
		}
		}
	}
}

static portTASK_FUNCTION(updateTaskIR30, pvParameters)
{
	uint8_t voltByte0IR30 = 0;
	uint8_t voltByte1IR30 = 0;

	structSensor* param = (structSensor*)pvParameters;
	vtI2CStruct* devI2C0 = param->devI2C0;
	structLCD* dataLCD = param->dataLCD;
	
	// String buffer for printing.
	char bufferLCD[maxLenLCD + 1];
	
	// Buffer for receiving messages.
	msgIR msgBuffer;

	uint8_t currentState = stateReadByte0IR30;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from either a timer or from an I2C operation.
		if (xQueueReceive(param->inQ, (void*)&msgBuffer, portMAX_DELAY) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// Now, based on the type of the message and the state, we decide on the new state and action to take.
		switch(getMsgType(&msgBuffer))
		{
		case msgTypeTimerIR30:
		{
			// Timer messages never change the state, they just cause an action (or not).
			if (vtI2CEnQ(devI2C0, msgTypeIR30ReadByte0, SLAVE_ADDR, sizeof(queryReadByte0IR30), queryReadByte0IR30, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgTypeIR30ReadByte1, SLAVE_ADDR, sizeof(queryReadByte1IR30), queryReadByte1IR30, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			break;
		}
		case msgTypeIR30ReadByte0:
		{
			if (currentState == stateReadByte0IR30)
			{
				currentState = stateReadByte1IR30;
				voltByte0IR30 = msgBuffer.buf[0];
			}
			else
			{
				// Unexpectedly received this message.
			}
			break;
		}
		case msgTypeIR30ReadByte1:
		{
			if (currentState == stateReadByte1IR30)
			{  	
				currentState = stateReadByte0IR30;
				voltByte1IR30 = msgBuffer.buf[0];

				unsigned int volt = 0;
				float voltF = 0;
				volt = voltByte0IR30 << 8; // MSB
				volt = volt | voltByte1IR30;
				volt = (float)(voltF);
				
				volt = volt/(1023/3.3);
			}
			else
			{
				// unexpectedly received this message
			}
			break;
		}	
		default:
		{
			// error
		}
		}
	}
}

static portTASK_FUNCTION(updateTaskIR31, pvParameters)
{
	uint8_t voltByte0IR31 = 0;
	uint8_t voltByte1IR31 = 0;

	structSensor* param = (structSensor*)pvParameters;
	vtI2CStruct* devI2C0 = param->devI2C0;
	structLCD* dataLCD = param->dataLCD;
	
	// String buffer for printing.
	char bufferLCD[maxLenLCD + 1];
	
	// Buffer for receiving messages.
	msgIR msgBuffer;

	uint8_t currentState = stateReadByte0IR31;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from either a timer or from an I2C operation.
		if (xQueueReceive(param->inQ, (void*)&msgBuffer, portMAX_DELAY) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// Now, based on the type of the message and the state, we decide on the new state and action to take.
		switch(getMsgType(&msgBuffer))
		{
		case msgTypeTimerIR31:
		{
			// Timer messages never change the state, they just cause an action (or not).
			if (vtI2CEnQ(devI2C0, msgTypeIR31ReadByte0, SLAVE_ADDR, sizeof(queryReadByte0IR31), queryReadByte0IR31, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgTypeIR31ReadByte1, SLAVE_ADDR, sizeof(queryReadByte1IR31), queryReadByte1IR31, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			break;
		}
		case msgTypeIR31ReadByte0:
		{
			if (currentState == stateReadByte0IR31)
			{
				currentState = stateReadByte1IR31;
				voltByte0IR31 = msgBuffer.buf[0];
			}
			else
			{
				// Unexpectedly received this message.
			}
			break;
		}
		case msgTypeIR31ReadByte1:
		{
			if (currentState == stateReadByte1IR31)
			{  	
				currentState = stateReadByte0IR31;
				voltByte1IR31 = msgBuffer.buf[0];

				unsigned int volt = 0;
				float voltF = 0;
				volt = voltByte0IR31 << 8; // MSB
				volt = volt | voltByte1IR31;
				volt = (float)(voltF);
				
				volt = volt/(1023/3.3);
			}
			else
			{
				// unexpectedly received this message
			}
			break;
		}	
		default:
		{
			// error
		}
		}
	}
}

static portTASK_FUNCTION(updateTaskIR40, pvParameters)
{
	uint8_t voltByte0IR40 = 0;
	uint8_t voltByte1IR40 = 0;

	structSensor* param = (structSensor*)pvParameters;
	vtI2CStruct* devI2C0 = param->devI2C0;
	structLCD* dataLCD = param->dataLCD;
	
	// String buffer for printing.
	char bufferLCD[maxLenLCD + 1];
	
	// Buffer for receiving messages.
	msgIR msgBuffer;

	uint8_t currentState = stateReadByte0IR40;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from either a timer or from an I2C operation.
		if (xQueueReceive(param->inQ, (void*)&msgBuffer, portMAX_DELAY) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// Now, based on the type of the message and the state, we decide on the new state and action to take.
		switch(getMsgType(&msgBuffer))
		{
		case msgTypeTimerIR40:
		{
			// Timer messages never change the state, they just cause an action (or not).
			if (vtI2CEnQ(devI2C0, msgTypeIR40ReadByte0, SLAVE_ADDR, sizeof(queryReadByte0IR40), queryReadByte0IR40, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgTypeIR40ReadByte1, SLAVE_ADDR, sizeof(queryReadByte1IR40), queryReadByte1IR40, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			break;
		}
		case msgTypeIR40ReadByte0:
		{
			if (currentState == stateReadByte0IR40)
			{
				currentState = stateReadByte1IR40;
				voltByte0IR40 = msgBuffer.buf[0];
			}
			else
			{
				// Unexpectedly received this message.
			}
			break;
		}
		case msgTypeIR40ReadByte1:
		{
			if (currentState == stateReadByte1IR40)
			{  	
				currentState = stateReadByte0IR40;
				voltByte1IR40 = msgBuffer.buf[0];

				unsigned int volt = 0;
				float voltF = 0;
				volt = voltByte0IR40 << 8; // MSB
				volt = volt | voltByte1IR40;
				volt = (float)(voltF);
				
				volt = volt/(1023/3.3);
			}
			else
			{
				// unexpectedly received this message
			}
			break;
		}	
		default:
		{
			// error
		}

		}
	}
}

static portTASK_FUNCTION(updateTaskIR41, pvParameters)
{
	uint8_t voltByte0IR41 = 0;
	uint8_t voltByte1IR41 = 0;

	structSensor* param = (structSensor*)pvParameters;
	vtI2CStruct* devI2C0 = param->devI2C0;
	structLCD* dataLCD = param->dataLCD;
	
	// String buffer for printing.
	char bufferLCD[maxLenLCD + 1];
	
	// Buffer for receiving messages.
	msgIR msgBuffer;

	uint8_t currentState = stateReadByte0IR41;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from either a timer or from an I2C operation.
		if (xQueueReceive(param->inQ, (void*)&msgBuffer, portMAX_DELAY) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// Now, based on the type of the message and the state, we decide on the new state and action to take.
		switch(getMsgType(&msgBuffer))
		{
		case msgTypeTimerIR41:
		{
			// Timer messages never change the state, they just cause an action (or not).
			if (vtI2CEnQ(devI2C0, msgTypeIR41ReadByte0, SLAVE_ADDR, sizeof(queryReadByte0IR41), queryReadByte0IR41, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgTypeIR41ReadByte1, SLAVE_ADDR, sizeof(queryReadByte1IR41), queryReadByte1IR41, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			break;
		}
		case msgTypeIR41ReadByte0:
		{
			if (currentState == stateReadByte0IR41)
			{
				currentState = stateReadByte1IR41;
				voltByte0IR41 = msgBuffer.buf[0];
			}
			else
			{
				// Unexpectedly received this message.
			}
			break;
		}
		case msgTypeIR41ReadByte1:
		{
			if (currentState == stateReadByte1IR41)
			{  	
				currentState = stateReadByte0IR41;
				voltByte1IR41 = msgBuffer.buf[0];

				unsigned int volt = 0;
				float voltF = 0;
				volt = voltByte0IR41 << 8; // MSB
				volt = volt | voltByte1IR41;
				volt = (float)(voltF);
				
				volt = volt/(1023/3.3);
			}
			else
			{
				// unexpectedly received this message
			}
			break;
		}	
		default:
		{
			// error
		}
		}
	}
}

/*------------------------------------------------------------------------------
 * AC00 Update Task
 */
static portTASK_FUNCTION(updateTaskAC00, pvParameters)
{
	uint8_t voltByte0AC00 = 0;
	uint8_t voltByte1AC00 = 0;

	structSensor* param = (structSensor*)pvParameters;
	vtI2CStruct* devI2C0 = param->devI2C0;
	structLCD* dataLCD = param->dataLCD;
	
	// String buffer for printing.
	char bufferLCD[maxLenLCD + 1];
	
	// Buffer for receiving messages.
	msgIR msgBuffer;

	uint8_t currentState = stateReadByte0AC00;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from either a timer or from an I2C operation.
		if (xQueueReceive(param->inQ, (void*)&msgBuffer, portMAX_DELAY) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// Now, based on the type of the message and the state, we decide on the new state and action to take.
		switch(getMsgType(&msgBuffer))
		{
		case msgTypeTimerAC00:
		{
			// Timer messages never change the state, they just cause an action (or not).
			if (vtI2CEnQ(devI2C0, msgTypeAC00ReadByte0, SLAVE_ADDR, sizeof(queryReadByte0AC00), queryReadByte0AC00, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgTypeAC00ReadByte1, SLAVE_ADDR, sizeof(queryReadByte1AC00), queryReadByte1AC00, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			break;
		}
		case msgTypeAC00ReadByte0:
		{
			if (currentState == stateReadByte0AC00)
			{
				currentState = stateReadByte1AC00;
				voltByte0AC00 = msgBuffer.buf[0];
			}
			else
			{
				// Unexpectedly received this message.
			}
			break;
		}
		case msgTypeAC00ReadByte1:
		{
			if (currentState == stateReadByte1AC00)
			{  	
				currentState = stateReadByte0AC00;
				voltByte1AC00 = msgBuffer.buf[0];
			}
			else
			{
				// unexpectedly received this message
			}
			break;
		}	
		default:
		{
			// error
		}
		}
	}
}

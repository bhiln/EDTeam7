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
#include "taskLocate.h"
#include "I2CTaskMsgTypes.h"
#include "debug.h"

/*------------------------------------------------------------------------------
 * Configuration
 **/

// Length of the queue to this task.
#define queueLenSensor 10

// Stack sizes.
#define BASE_STACK 3
#if PRINTF_VERSION == 1
#define I2C_STACK_SIZE		((BASE_STACK+5)*configMINIMAL_STACK_SIZE)
#else
#define I2C_STACK_SIZE		(BASE_STACK*configMINIMAL_STACK_SIZE)
#endif

// Actual data structure that is sent in a message.
typedef struct __msgSensor {
	uint8_t msgType;
	uint8_t	length;
	uint8_t buf[maxLenSensor + 1];
} msgSensor;


// I2C commands for reading sensor data.
const uint8_t recvNoReply = 0x00;
const uint8_t queryReadSensor[] = {0x0A};

static portTASK_FUNCTION_PROTO(updateTaskSensor, pvParameters);

int getMsgTypeSensor(msgSensor *Buffer) {return(Buffer->msgType);}

void startTaskSensor(structSensor* dataSensor, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0, structLCD* dataLCD)
{
	// Create the queue that will be used to talk to this task.
	if ((dataSensor->inQ = xQueueCreate(queueLenSensor, sizeof(msgSensor))) == NULL)
		VT_HANDLE_FATAL_ERROR(0);

	// Create the task.
	portBASE_TYPE retval;
	dataSensor->devI2C0 = devI2C0;
	dataSensor->dataLCD = dataLCD;
	
	if ((retval = xTaskCreate(updateTaskSensor, taskNameSensor, I2C_STACK_SIZE, (void*) dataSensor, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
		VT_HANDLE_FATAL_ERROR(retval);
}

portBASE_TYPE sendTimerMsgSensor(structSensor* dataSensor, uint8_t msgType, portTickType ticksElapsed, portTickType ticksToBlock)
{
	if (dataSensor == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	
	msgSensor bufferSensor;
	bufferSensor.length = sizeof(ticksElapsed);

	if (bufferSensor.length > maxLenSensor)
		VT_HANDLE_FATAL_ERROR(bufferSensor.length);

	memcpy(bufferSensor.buf, (char*)&ticksElapsed, sizeof(ticksElapsed));
	bufferSensor.msgType = msgType;
	return(xQueueSend(dataSensor->inQ, (void*) (&bufferSensor),ticksToBlock));
}

portBASE_TYPE sendValueMsgSensor(structSensor* dataSensor, uint8_t msgType, uint16_t value, portTickType ticksToBlock)
{
	msgSensor bufferSensor;

	if (dataSensor == NULL)
		VT_HANDLE_FATAL_ERROR(0);

	bufferSensor.length = sizeof(value);

	if (bufferSensor.length > maxLenSensor)
		VT_HANDLE_FATAL_ERROR(bufferSensor.length);
	
	memcpy(bufferSensor.buf, (char*)&value, sizeof(value));
	bufferSensor.msgType = msgType;
	return(xQueueSend(dataSensor->inQ, (void*)(&bufferSensor), ticksToBlock));
}

static portTASK_FUNCTION(updateTaskSensor, pvParameters)
{
	// Get pointers to sensor message fields.
	structSensor* param = (structSensor*)pvParameters;
	vtI2CStruct* devI2C0 = param->devI2C0;
	structLCD* dataLCD = param->dataLCD;
	structLocate* dataLocate = param->dataLocate;
	
	// String buffer for printing.
	char bufferLCD[maxLenLCD + 1];
	
	// Buffer for receiving messages.
	msgSensor msgBuffer;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from either a timer or from an I2C operation.
		if (xQueueReceive(param->inQ, (void*)&msgBuffer, portMAX_DELAY) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// Now, based on the type of the message and the state, we decide on the new state and action to take.
		switch(getMsgTypeSensor(&msgBuffer))
		{
		case msgSensorTimer:
		{
			// Query for all sensor data.
			if (vtI2CEnQ(devI2C0, msgSensorIR00, SLAVE_ADDR, sizeof(queryReadSensor), queryReadSensor, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgSensorIR01, SLAVE_ADDR, sizeof(queryReadSensor), queryReadSensor, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgSensorIR10, SLAVE_ADDR, sizeof(queryReadSensor), queryReadSensor, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgSensorIR11, SLAVE_ADDR, sizeof(queryReadSensor), queryReadSensor, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgSensorIR20, SLAVE_ADDR, sizeof(queryReadSensor), queryReadSensor, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgSensorIR21, SLAVE_ADDR, sizeof(queryReadSensor), queryReadSensor, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgSensorIR30, SLAVE_ADDR, sizeof(queryReadSensor), queryReadSensor, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgSensorIR31, SLAVE_ADDR, sizeof(queryReadSensor), queryReadSensor, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgSensorIR40, SLAVE_ADDR, sizeof(queryReadSensor), queryReadSensor, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgSensorIR41, SLAVE_ADDR, sizeof(queryReadSensor), queryReadSensor, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			if (vtI2CEnQ(devI2C0, msgSensorAC00, SLAVE_ADDR, sizeof(queryReadSensor), queryReadSensor, 2) != pdTRUE)
				VT_HANDLE_FATAL_ERROR(0);
			break;
		}
		case msgSensorIR00:
		{
			// Get the sensor data from the queue (should already
			// be 2 bytes from conductor task).
			uint16_t sensorData = msgBuffer.buf[1];

			// Convert the IR voltage to a distance.
			uint16_t locateData = sensorData; //Blah change this now.
			
			// Send the processed sensor data to the locate task.
			sendValueMsgLocate(dataLocate, msgSensorIR00, locateData, portMAX_DELAY);
		}
		default:
		{
			// error
		}
		}
	}
}

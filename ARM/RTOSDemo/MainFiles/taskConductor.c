#include <stdlib.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "projdefs.h"
#include "semphr.h"

#include "vtUtilities.h"
#include "vtI2C.h"
#include "taskSensors.h"
#include "I2CTaskMsgTypes.h"
#include "taskConductor.h"

#define INSPECT_STACK 	1
#define BASE_STACK 	2
#if PRINTF_VERSION == 1
#define CONDUCTOR_STACK_SIZE	((BASE_STACK+5)*configMINIMAL_STACK_SIZE)
#else
#define CONDUCTOR_STACK_SIZE	(BASE_STACK*configMINIMAL_STACK_SIZE)
#endif

static portTASK_FUNCTION_PROTO(updateTaskConductor, pvParameters);

void startTaskConductor(structConductor* params, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0, structSensors* dataSensors)
{
	portBASE_TYPE retval;
	params->devI2C0 = devI2C0;
	params->dataSensors = dataSensors;

	if ((retval = xTaskCreate(updateTaskConductor, taskNameConductor, CONDUCTOR_STACK_SIZE, (void*)params, uxPriority, (xTaskHandle*) NULL )) != pdPASS) {
		VT_HANDLE_FATAL_ERROR(retval);
	}
}

static portTASK_FUNCTION(updateTaskConductor, pvParameters)
{
	uint8_t rxLen;
	uint8_t status;
	uint8_t bufferI2C[vtI2CMLen];

	// Get the values from I2C.
	uint8_t* values = &(bufferI2C[0]);

	// Get the parameters.
	structConductor* param = (structConductor*)pvParameters;

	// Get the I2C device pointer.
	vtI2CStruct* devI2C0 = param->devI2C0;
	
	// Get sensor data pointers.
	structSensors* dataSensors = param->dataSensors;

	// The received message type.
	uint8_t recvMsgType;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from an I2C operation.
		if (vtI2CDeQ(devI2C0, vtI2CMLen, bufferI2C, &rxLen, &recvMsgType, &status) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// If the I2C message is dropped, handle it.

		// Decide where to send the message.
		switch(recvMsgType)
		{
			// Sensor task.
            case msgTypeSensors: 
			{
				// Send the values to the sensor task.
				sendValueMsgSensors(dataSensors, recvMsgType, values, portMAX_DELAY);
				break;
			}
			default:
			{
				VT_HANDLE_FATAL_ERROR(recvMsgType);
				break;
			}
		}
	}
}


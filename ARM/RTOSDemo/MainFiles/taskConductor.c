#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

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

void startTaskConductor(structConductor* params, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0,
	structSensor* dataIR00, structSensor* dataIR01,
	structSensor* dataIR10, structSensor* dataIR11,
	structSensor* dataIR20, structSensor* dataIR21,
	structSensor* dataIR30, structSensor* dataIR31,
	structSensor* dataIR40, structSensor* dataIR41,
	structSensor* dataAC00)
{
	portBASE_TYPE retval;
	params->devI2C0 = devI2C0;
	params->dataIR00 = dataIR00;
	params->dataIR01 = dataIR01;
	params->dataIR10 = dataIR10;
	params->dataIR11 = dataIR11;
	params->dataIR20 = dataIR20;
	params->dataIR21 = dataIR21;
	params->dataIR30 = dataIR30;
	params->dataIR31 = dataIR31;
	params->dataIR40 = dataIR40;
	params->dataIR41 = dataIR41;
	params->dataAC00 = dataAC00;

	if ((retval = xTaskCreate(updateTaskConductor, (signed char*)"Conductor", CONDUCTOR_STACK_SIZE, (void*)params, uxPriority, (xTaskHandle*) NULL )) != pdPASS) {
		VT_HANDLE_FATAL_ERROR(retval);
	}
}

static portTASK_FUNCTION(updateTaskConductor, pvParameters)
{
	uint8_t rxLen, status;
	uint8_t bufferI2C[vtI2CMLen];
	uint8_t *value = &(bufferI2C[0]);

	// Get the parameters.
	structConductor* param = (structConductor*)pvParameters;

	// Get the I2C device pointer.
	vtI2CStruct *devI2C0 = param->devI2C0;
	
	// Get sensor data pointers.
	structSensor *dataIR00 = param->dataIR00;
	structSensor *dataIR01 = param->dataIR01;
	structSensor *dataIR10 = param->dataIR10;
	structSensor *dataIR11 = param->dataIR11;
	structSensor *dataIR20 = param->dataIR20;
	structSensor *dataIR21 = param->dataIR21;
	structSensor *dataIR30 = param->dataIR30;
	structSensor *dataIR31 = param->dataIR31;
	structSensor *dataIR40 = param->dataIR40;
	structSensor *dataIR41 = param->dataIR41;
	structSensor *dataAC00 = param->dataAC00;

	uint8_t recvMsgType;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from an I2C operation.
		if (vtI2CDeQ(devI2C0, vtI2CMLen, bufferI2C, &rxLen, &recvMsgType, &status) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// Decide where to send the message.
		switch(recvMsgType)
		{
			// Front left sensor.
			case msgTypeIR00Init:
			{
				sendValueMsgSensor(dataIR00, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR00ReadByte0:
			{
				sendValueMsgSensor(dataIR00, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR00ReadByte1:
			{
				sendValueMsgSensor(dataIR00, recvMsgType, (*value), portMAX_DELAY);
				break;
			}

			// Front right sensor.
			case msgTypeIR01Init:
			{
				sendValueMsgSensor(dataIR01, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR01ReadByte0:
			{
				sendValueMsgSensor(dataIR01, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR01ReadByte1:
			{
				sendValueMsgSensor(dataIR01, recvMsgType, (*value), portMAX_DELAY);
				break;
			}

			// Front top sensor.
			case msgTypeIR10Init:
			{
				sendValueMsgSensor(dataIR10, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR10ReadByte0:
			{
				sendValueMsgSensor(dataIR10, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR10ReadByte1:
			{
				sendValueMsgSensor(dataIR10, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
	
			// Front bottom sensor.
			case msgTypeIR11Init:
			{
				sendValueMsgSensor(dataIR11, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR11ReadByte0:
			{
				sendValueMsgSensor(dataIR11, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR11ReadByte1:
			{
				sendValueMsgSensor(dataIR11, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			// Right top sensor.
			case msgTypeIR20Init:
			{
				sendValueMsgSensor(dataIR20, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR20ReadByte0:
			{
				sendValueMsgSensor(dataIR20, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR20ReadByte1:
			{
				sendValueMsgSensor(dataIR20, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			// Right bottom sensor.
			case msgTypeIR21Init:
			{
				sendValueMsgSensor(dataIR21, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR21ReadByte0:
			{
				sendValueMsgSensor(dataIR21, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR21ReadByte1:
			{
				sendValueMsgSensor(dataIR21, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			// Back right sensor.
			case msgTypeIR30Init:
			{
				sendValueMsgSensor(dataIR30, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR30ReadByte0:
			{
				sendValueMsgSensor(dataIR30, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR30ReadByte1:
			{
				sendValueMsgSensor(dataIR30, recvMsgType, (*value), portMAX_DELAY);
				break;
			}

			// Back left sensor.
			case msgTypeIR31Init:
			{
				sendValueMsgSensor(dataIR31, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR31ReadByte0:
			{
				sendValueMsgSensor(dataIR31, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR31ReadByte1:
			{
				sendValueMsgSensor(dataIR31, recvMsgType, (*value), portMAX_DELAY);
				break;
			}

			// Left bottom sensor.
			case msgTypeIR40Init:
			{
				sendValueMsgSensor(dataIR40, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR40ReadByte0:
			{
				sendValueMsgSensor(dataIR40, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR40ReadByte1:
			{
				sendValueMsgSensor(dataIR40, recvMsgType, (*value), portMAX_DELAY);
				break;
			}

			// Left top sensor.
			case msgTypeIR41Init:
			{
				sendValueMsgSensor(dataIR41, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR41ReadByte0:
			{
				sendValueMsgSensor(dataIR41, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeIR41ReadByte1:
			{
				sendValueMsgSensor(dataIR41, recvMsgType, (*value), portMAX_DELAY);
				break;
			}

			// Middle sensor (accelerometer).
			case msgTypeAC00Init:
			{
				sendValueMsgSensor(dataAC00, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeAC00ReadByte0:
			{
				sendValueMsgSensor(dataAC00, recvMsgType, (*value), portMAX_DELAY);
				break;
			}
			case msgTypeAC00ReadByte1:
			{
				sendValueMsgSensor(dataAC00, recvMsgType, (*value), portMAX_DELAY);
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


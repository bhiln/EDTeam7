/*------------------------------------------------------------------------------
 * File:		taskConductor.c
 * Authors: 	FreeRTOS, Igor Janjic
 * Description:	Implementation file for conductor class. Routes I2C messages to
 *              their appropriate targets.
 *----------------------------------------------------------------------------*/

#include "taskConductor.h"

static portTASK_FUNCTION_PROTO(updateTaskConductor, pvParameters);

void startTaskConductor(structConductor* params, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0, structLCD* dataLCD, structSensors* dataSensors, structLocate* dataLocate)
{
	params->devI2C0     = devI2C0;
    params->dataLCD     = dataLCD;
	params->dataSensors = dataSensors;
    params->dataLocate  = dataLocate;

    portBASE_TYPE retval = xTaskCreate(updateTaskConductor, taskNameConductor, CONDUCTOR_STACK_SIZE, (void*)params, uxPriority, (xTaskHandle*)NULL);
	if(retval != pdPASS)
    {
        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, errorTaskCreateConductor, portMAX_DELAY);
		VT_HANDLE_FATAL_ERROR(retval);
	}
}

static portTASK_FUNCTION(updateTaskConductor, pvParameters)
{
	uint8_t rxLen;
	uint8_t status;
	uint8_t bufferI2C[vtI2CMLen];

	// Get the values from I2C.
	uint8_t* values = bufferI2C;

	// Get pointers to paramter fields.
	structConductor* param       = (structConductor*)pvParameters;
	vtI2CStruct*     devI2C0     = param->devI2C0;
    structLCD*       dataLCD     = param->dataLCD;
	structSensors*   dataSensors = param->dataSensors;
    structLocate*    dataLocate  = param->dataLocate;

	// The received message type.
	uint8_t recvMsgType;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from an I2C operation.
        portBASE_TYPE retI2C = vtI2CDeQ(devI2C0, vtI2CMLen, bufferI2C, &rxLen, &recvMsgType, &status);
		if(retI2C != pdTRUE)
        {
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, errorI2CDequeConductor, portMAX_DELAY);
		    VT_HANDLE_FATAL_ERROR(retI2C);
        }

        // Log that there is an error with I2C slave.
        if(rxLen == 0) 
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, errorI2C, portMAX_DELAY);

		// Decide where to send the message.
		switch(recvMsgType)
		{
			// Sensor task.
            case MSG_TYPE_SENSORS: 
			{
                // If rxLen > 0, then we received a message succesfully.
                if(rxLen > 0)
                {
                    // If values[0] is 0, then we have no sensor data.
                    if(values[0] == 0)
                    {
                        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugNoRcvdSensorData, portMAX_DELAY);
                    }
                    // If values[0] is 1, then we have new sensor data.
                    else if(values[0] == 1)
                    {
                        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugRcvdSensorData, portMAX_DELAY);
                        sendValueMsgSensors(dataSensors, recvMsgType, values, portMAX_DELAY);
                    }
                }
				break;
			}
            case MSG_TYPE_ACK:
			{
                if(values[0] == 0)
                    sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugNoRcvdAck, portMAX_DELAY);
                else if (values[0] == 1)
                {
                    sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugRcvdAck, portMAX_DELAY);
                    sendValueMsgLocate(dataLocate, recvMsgType, (float*)values, portMAX_DELAY);
                }
				break;
            }
			default:
			{
				VT_HANDLE_FATAL_ERROR(0);
				break;
			}
		}
	}
}


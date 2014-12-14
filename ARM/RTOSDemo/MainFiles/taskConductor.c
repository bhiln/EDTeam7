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
        char eventsMsg[QUEUE_BUF_LEN_LCD];
        sprintf(eventsMsg, "Error: failed creating %s", taskNameConductor);
        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
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
        toggleLED(PIN_LED_1);  

        char eventsMsg[QUEUE_BUF_LEN_LCD];

		// Wait for a message from an I2C operation.
        portBASE_TYPE retI2C = vtI2CDeQ(devI2C0, vtI2CMLen, bufferI2C, &rxLen, &recvMsgType, &status);
		if(retI2C != pdTRUE)
        {
            sprintf(eventsMsg, "Error: cannot dequeue from %s", taskNameI2C);
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
		    VT_HANDLE_FATAL_ERROR(retI2C);
        }

        // Otherwise route it.
        else
        {
		    // Decide where to send the message.
		    switch(recvMsgType)
            {
                // Sensor task.
                case MSG_TYPE_I2C_SENSORS: 
                {
                    // Log that there is an error with I2C, since we are expecting a message.
                    if(rxLen == 0) 
                    {
                        sprintf(eventsMsg, "Error: bad I2C connection");
                        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
                    }
                    else
                    {
                        // If values[23] is 1, then the command is completed.
                        if(values[22] == 0x01)
						{
                            sprintf(eventsMsg, "Command completed");
                            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
							float ack = (float)values[22];
                            sendValueMsgLocate(dataLocate, MSG_TYPE_ACK, &ack, portMAX_DELAY);
						}
                        
                        // If values[0] is 0, then we have no sensor data.
                        if(values[0] == 0)
                        {
                            sprintf(eventsMsg, "No received sensor data");
                            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
                        }

                        // If values[0] is 1, then we have new sensor data.
                        else if(values[0] == 0xFE)
                        {
                            sprintf(eventsMsg, "Received new sensor data");
                            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
                            sendValueMsgSensors(dataSensors, MSG_TYPE_SENSORS, &values[2], portMAX_DELAY);
                        }
                        else
                        {
                            sprintf(eventsMsg, "Error: unrecognized sensor message header");
                            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
                        }
                    }
                    break;
                }
                case MSG_TYPE_I2C_ACK:
                {
                    // Log that there is an error with I2C, since we are expecting a message.
                    if(rxLen == 0) 
                    {
                        sprintf(eventsMsg, "Error: bad I2C connection");
                        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
                    }

                    // If values[0] = 0, then we have no ack.
                    else if(values[0] == 0)
                    {
                        sprintf(eventsMsg, "Command not yet completed");
                        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
                    }
                    
                    // If values[0] = 1, then we have an ack.
                    else if (values[0] == 1)
                    {
                        sprintf(eventsMsg, "Command completed");
                        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
                        sendValueMsgLocate(dataLocate, MSG_TYPE_ACK, (float*)values, portMAX_DELAY);
                    }
                    else
                    {
                        sprintf(eventsMsg, "Error: unrecognized ack message header");
                        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
                    }
                    break;
                }
                case MSG_TYPE_I2C_CMD:
                {
                    // Nothing to send to anywhere so break.
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
}


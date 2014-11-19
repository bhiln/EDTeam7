/*------------------------------------------------------------------------------
 * File:		    taskCommand.c
 * Authors: 		FreeRTOS, Igor Janjic
 * Description:		Implementation file for the command task. Sends rover
 *                  commands to the motor commander.
 *----------------------------------------------------------------------------*/

#include "taskCommand.h"

static portTASK_FUNCTION_PROTO(updateTaskCommand, pvParameters);

void startTaskCommand(structCommand* dataCommand, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0, structLCD* dataLCD)
{
    // Create the queue that will be used to talk to this task.
    dataCommand->inQ = xQueueCreate(queueLenCommand, sizeof(msgCommand));
    if(dataCommand->inQ == NULL)
    {
        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, errorQueueCreateCommand, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(0);
    }

    // Create the task.
    dataCommand->devI2C0 = devI2C0;
    dataCommand->dataLCD = dataLCD;
    
    portBASE_TYPE retval = xTaskCreate(updateTaskCommand, taskNameCommand, COMMAND_STACK_SIZE, (void*)dataCommand, uxPriority, (xTaskHandle*)NULL);
    if(retval != pdPASS)
    {
        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, errorTaskCreateCommand, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(retval);
    }
}


void sendTimerMsgCommand(structCommand* dataCommand, portTickType ticksElapsed, portTickType ticksToBlock)
{
    msgCommand msg;
    msg.length = sizeof(ticksElapsed);

    memcpy(msg.buf, &ticksElapsed, msg.length);
    msg.type = MSG_TYPE_TIMER_CMD;

    portBASE_TYPE retval = xQueueSend(dataCommand->inQ, (void*)(&msg), ticksToBlock);
    if(retval != pdTRUE)
    {
        sendValueMsgLCD(dataCommand->dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, errorQueueSendCommand, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(retval);
    }

}

void sendValueMsgCommand(structCommand* dataCommand, uint8_t type, uint8_t* value, portTickType ticksToBlock)
{
    msgCommand msg;
    msg.length = sizeof(uint8_t)*bufLenCommand;

    memcpy(msg.buf, value, msg.length);
    msg.type = type;
    portBASE_TYPE retval = xQueueSend(dataCommand->inQ, (void*)(&msg), ticksToBlock);
    if(retval != pdTRUE)
    {
        sendValueMsgLCD(dataCommand->dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, errorQueueSendCommand, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(retval);
    }
}

static portTASK_FUNCTION(updateTaskCommand, pvParameters)
{
	structCommand* param   = (structCommand*)pvParameters;
	vtI2CStruct*   devI2C0 = param->devI2C0;
	structLCD*     dataLCD = param->dataLCD;
	
	// Buffer for receiving messages.
	msgCommand msg;

	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from the queue.
        portBASE_TYPE retQueue = xQueueReceive(param->inQ, (void*)&msg, portMAX_DELAY);
        if (retQueue != pdTRUE)
        {
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, errorQueueReceiveCommand, portMAX_DELAY);
            VT_HANDLE_FATAL_ERROR(retQueue);
        }

		// Now, based on the type of the message and the state, do different things.
		switch(msg.type)
		{
        case MSG_TYPE_TIMER_CMD:
        {
            break;
        }
        case MSG_TYPE_CMD:
        {
            execCommand(devI2C0, msg.buf[0], msg.buf[1], msg.buf[2]);
            break;
        }
        default:
        {
            // error
        }
		}
	}
}

bool execCommand(vtI2CStruct* devI2C0, uint8_t type, uint8_t value, uint8_t speed)
{
    uint8_t cmd[CMD_LEN];
    cmd[CMD_TYPE]  = type;
    cmd[CMD_VALUE] = value;
    cmd[CMD_SPEED] = speed;

    // Send an I2C command to slave.
    portBASE_TYPE retI2C = pdTRUE;
    retI2C = vtI2CEnQ(devI2C0, MSG_TYPE_CMD, SLAVE_ADDR, sizeof((uint8_t*)cmd), (uint8_t*)cmd, 1);
    return (retI2C == pdTRUE);
}


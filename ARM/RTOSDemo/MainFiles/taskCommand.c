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
    char eventsMsg[QUEUE_BUF_LEN_LCD];

    // Create the queue that will be used to talk to this task.
    dataCommand->inQ = xQueueCreate(QUEUE_LEN_CMD, sizeof(msgCommand));
    if(dataCommand->inQ == NULL)
    {
        sprintf(eventsMsg, "Error: failed creating queue for task %s", taskNameCommand);
        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(0);
    }

    // Create the task.
    dataCommand->devI2C0 = devI2C0;
    dataCommand->dataLCD = dataLCD;
    
    portBASE_TYPE retval = xTaskCreate(updateTaskCommand, taskNameCommand, COMMAND_STACK_SIZE, (void*)dataCommand, uxPriority, (xTaskHandle*)NULL);
    if(retval != pdPASS)
    {
        sprintf(eventsMsg, "Error: failed creating task %s", taskNameCommand);
        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(retval);
    }
}


void sendTimerMsgCommand(structCommand* dataCommand, portTickType ticksElapsed, portTickType ticksToBlock)
{
    char eventsMsg[QUEUE_BUF_LEN_LCD];

    msgCommand msg;
    msg.length = sizeof(ticksElapsed);

    memcpy(msg.buf, &ticksElapsed, msg.length);
    msg.type = MSG_TYPE_TIMER_CMD;

    portBASE_TYPE retval = xQueueSend(dataCommand->inQ, (void*)(&msg), ticksToBlock);
    if(retval != pdTRUE)
    {
        sprintf(eventsMsg, "Error: failed sending to timer queue for task %s", taskNameCommand);
        sendValueMsgLCD(dataCommand->dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(retval);
    }

}

void sendValueMsgCommand(structCommand* dataCommand, uint8_t type, uint8_t* value, portTickType ticksToBlock)
{
    char eventsMsg[QUEUE_BUF_LEN_LCD];

    msgCommand msg;
    msg.length = sizeof(uint8_t)*QUEUE_BUF_LEN_CMD;

    memcpy(msg.buf, value, msg.length);
    msg.type = type;
    portBASE_TYPE retval = xQueueSend(dataCommand->inQ, (void*)(&msg), ticksToBlock);
    if(retval != pdTRUE)
    {
        sprintf(eventsMsg, "Error: failed sending to value queue for task %s", taskNameCommand);
        sendValueMsgLCD(dataCommand->dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
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

    char eventsMsg[QUEUE_BUF_LEN_LCD];

    uint8_t index = 0;

    uint8_t defaultJoyLeft[2]  = {20, 50};
    uint8_t defaultJoyRight[2] = {20, 50};
    uint8_t defaultJoyUp[2]    = {20, 50};
    uint8_t defaultJoyDown[2]  = {20, 50};

	// Like all good tasks, this should never exit.
	for(;;)
	{
        toggleLED(PIN_LED_4);

		// Wait for a message from the queue.
        portBASE_TYPE retQueue = xQueueReceive(param->inQ, (void*)&msg, portMAX_DELAY);
        if (retQueue != pdTRUE)
        {
            sprintf(eventsMsg, "Error: failed receiving from queue for task %s", taskNameCommand);
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
            VT_HANDLE_FATAL_ERROR(retQueue);
        }

		// Now, based on the type of the message and the state, do different things.
		switch(msg.type)
		{
        case MSG_TYPE_TIMER_CMD:
        {
            // Check the joystick to see if a manual command must be sent.
            uint32_t joyRead  = GPIO_ReadValue(PORT_JOY);
			uint32_t joyLeft  = joyRead & PIN_JOY_LEFT;
            uint32_t joyRight = joyRead & PIN_JOY_RIGHT;
            uint32_t joyUp    = joyRead & PIN_JOY_UP;
            uint32_t joyDown  = joyRead & PIN_JOY_DOWN;
            uint32_t joySel   = joyRead & PIN_JOY_SEL;

            // Send the command if need be.
            if(!joyLeft)
                execCommand(devI2C0, dataLCD, &index, CMD_TL, defaultJoyLeft[0], defaultJoyLeft[1]);
            else if(!joyRight)
                execCommand(devI2C0, dataLCD, &index, CMD_TR, defaultJoyRight[0], defaultJoyRight[1]);
            else if(!joyUp)
                execCommand(devI2C0, dataLCD, &index, CMD_MF, defaultJoyUp[0], defaultJoyUp[1]);
            else if(!joyDown)
                execCommand(devI2C0, dataLCD, &index, CMD_MB, defaultJoyDown[0], defaultJoyDown[1]);
            else if(!joySel)
                execCommand(devI2C0, dataLCD, &index, CMD_STOP, 0, 0);

            // Query the PIC for a complete command.
            //uint8_t query = 0x0B; 
            //portBASE_TYPE retI2C = vtI2CEnQ(devI2C0, MSG_TYPE_I2C_ACK, SLAVE_ADDR, sizeof(query), &query, 1);
            //if(retI2C != pdTRUE)
            //{
            //    sprintf(eventsMsg, "Error: unable to communicate over I2C");
            //    sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
            //    VT_HANDLE_FATAL_ERROR(retI2C);
            //}
			//else
			//{
			//	sprintf(eventsMsg, "Sending query for command ack");
            //	sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
			//}
			break;
        }
        case MSG_TYPE_CMD:
        {
            execCommand(devI2C0, dataLCD, &index, msg.buf[0], msg.buf[1], msg.buf[2]);
            break;
        }
        default:
        {
            // Error
        }
		}
	}
}

bool execCommand(vtI2CStruct* devI2C0, structLCD* dataLCD, uint8_t* index, uint8_t type, uint8_t value, uint8_t speed)
{
    char eventsMsg[QUEUE_BUF_LEN_LCD];
    char infoMsg[QUEUE_BUF_LEN_LCD];

    if(*index == 253)
        *index = 0;

    // Make sure the command is right.  
    if(value > 253)
        return false;
    if(speed > 63)
        return false;

    uint8_t cmd[CMD_LEN];
    cmd[CMD_INIT] = 0xFE;
    cmd[CMD_INDEX] = *index;
    cmd[CMD_MSG_TYPE] = 0x32;
    cmd[CMD_TYPE] = type;
    cmd[CMD_VALUE] = value;
    cmd[CMD_SPEED] = speed;
    cmd[CMD_TERM] = 0xFF;

	// Send command over I2C.
    portBASE_TYPE retI2C = vtI2CEnQ(devI2C0, MSG_TYPE_I2C_CMD, SLAVE_ADDR, sizeof(cmd), cmd, 0);
    if(retI2C != pdTRUE)
    {
        sprintf(eventsMsg, "Error: unable to communicate over I2C");
        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(retI2C);
        return false;
    }
    else
    {
        switch(cmd[CMD_TYPE])
        {
        case CMD_MF:
        {
            sprintf(eventsMsg, "Sending cmd to move forward %u at %u", cmd[CMD_VALUE], cmd[CMD_SPEED]);
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);

            sprintf(infoMsg, "CMD_MF %u %u %u", cmd[CMD_INDEX], cmd[CMD_VALUE], cmd[CMD_SPEED]);
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_CMD, QUEUE_BUF_LEN_LCD, infoMsg, portMAX_DELAY);
            break;
        }
        case CMD_MB:
        {
            sprintf(eventsMsg, "Sending cmd to move back %u at %u", cmd[CMD_VALUE], cmd[CMD_SPEED]);
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);

            sprintf(infoMsg, "CMD_MB %u %u %u", cmd[CMD_INDEX], cmd[CMD_VALUE], cmd[CMD_SPEED]);
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_CMD, QUEUE_BUF_LEN_LCD, infoMsg, portMAX_DELAY);
            break;
        }
        case CMD_TL:
        {
            sprintf(eventsMsg, "Sending cmd to turn left %u at %u", cmd[CMD_VALUE], cmd[CMD_SPEED]);
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);

            sprintf(infoMsg, "CMD_TL %u %u %u", cmd[CMD_INDEX], cmd[CMD_VALUE], cmd[CMD_SPEED]);
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_CMD, QUEUE_BUF_LEN_LCD, infoMsg, portMAX_DELAY);
            break;
        }
        case CMD_TR:
        {
            sprintf(eventsMsg, "Sending cmd to turn right %u at %u", cmd[CMD_VALUE], cmd[CMD_SPEED]);
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);

            sprintf(infoMsg, "CMD_TR %u %u %u", cmd[CMD_INDEX], cmd[CMD_VALUE], cmd[CMD_SPEED]);
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_CMD, QUEUE_BUF_LEN_LCD, infoMsg, portMAX_DELAY);
            break;
        }
        case CMD_STOP:
        {
            sprintf(eventsMsg, "Sending cmd to stop");
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);

            sprintf(infoMsg, "CMD_STOP %u %u %u", cmd[CMD_INDEX], cmd[CMD_VALUE], cmd[CMD_SPEED]);
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_CMD, QUEUE_BUF_LEN_LCD, infoMsg, portMAX_DELAY);
            break;
        }
        }
        (*index)++;
        return (retI2C == pdTRUE);
    }
}


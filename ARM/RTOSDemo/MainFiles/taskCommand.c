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
    dataCommand->inQ = xQueueCreate(QUEUE_LEN_CMD, sizeof(msgCommand));
    if(dataCommand->inQ == NULL)
    {
        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, errorQueueCreateCommand, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(0);
    }

    // Create the task.
    dataCommand->devI2C0 = devI2C0;
    dataCommand->dataLCD = dataLCD;
    
    portBASE_TYPE retval = xTaskCreate(updateTaskCommand, taskNameCommand, COMMAND_STACK_SIZE, (void*)dataCommand, uxPriority, (xTaskHandle*)NULL);
    if(retval != pdPASS)
    {
        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, errorTaskCreateCommand, portMAX_DELAY);
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
        sendValueMsgLCD(dataCommand->dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, errorQueueSendCommand, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(retval);
    }

}

void sendValueMsgCommand(structCommand* dataCommand, uint8_t type, uint8_t* value, portTickType ticksToBlock)
{
    msgCommand msg;
    msg.length = sizeof(uint8_t)*QUEUE_BUF_LEN_CMD;

    memcpy(msg.buf, value, msg.length);
    msg.type = type;
    portBASE_TYPE retval = xQueueSend(dataCommand->inQ, (void*)(&msg), ticksToBlock);
    if(retval != pdTRUE)
    {
        sendValueMsgLCD(dataCommand->dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, errorQueueSendCommand, portMAX_DELAY);
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
        toggleLED(PIN_LED_4);

		// Wait for a message from the queue.
        portBASE_TYPE retQueue = xQueueReceive(param->inQ, (void*)&msg, portMAX_DELAY);
        if (retQueue != pdTRUE)
        {
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, errorQueueReceiveCommand, portMAX_DELAY);
            VT_HANDLE_FATAL_ERROR(retQueue);
        }

		// Now, based on the type of the message and the state, do different things.
		switch(msg.type)
		{
        case MSG_TYPE_TIMER_CMD:
        {
            // Check the joystick to see if a manual command must be sent.
            uint32_t joyRead = GPIO_ReadValue(PORT_JOY);
			uint32_t joyLeft  = joyRead & PIN_JOY_LEFT;
            uint32_t joyRight = joyRead & PIN_JOY_RIGHT;
            uint32_t joyUp    = joyRead & PIN_JOY_UP;
            uint32_t joyDown  = joyRead & PIN_JOY_DOWN;
            uint32_t joySel   = joyRead & PIN_JOY_SEL;

            // Send the command if need be.
            if(!joyLeft)
            {
                sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, "Sending command to turn left at (5, 5).", portMAX_DELAY);
                if(execCommand(devI2C0, dataLCD, CMD_TL, 5, 5))
                    writeDebug(PIN_DEBUG_0, 1);
                else
                    writeDebug(PIN_DEBUG_1, 1);
            }
            else if(!joyRight)
            {
                sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, "Sending command to turn right at (5, 5).", portMAX_DELAY);
                execCommand(devI2C0, dataLCD, CMD_TR, 5, 5);
            }
            else if(!joyUp)
            {
                sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, "Sending command to move forward at (5, 5).", portMAX_DELAY);
                execCommand(devI2C0, dataLCD, CMD_MF, 5, 5);
            }
            else if(!joyDown)
            {
                sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, "Sending command to move backward at (5, 5).", portMAX_DELAY);
                execCommand(devI2C0, dataLCD, CMD_MB, 5, 5);
            }
            else if(!joySel)
            {
                sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, "Sending command to stop", portMAX_DELAY);
                execCommand(devI2C0, dataLCD, CMD_STOP, 0, 0);
            }
            break;
        }
        case MSG_TYPE_CMD:
        {
            execCommand(devI2C0, dataLCD, msg.buf[0], msg.buf[1], msg.buf[2]);
            break;
        }
        default:
        {
            // error
        }
		}
	}
}

bool execCommand(vtI2CStruct* devI2C0, structLCD* dataLCD, uint8_t type, uint8_t value, uint8_t speed)
{
    static uint8_t index = 0;
    //if((type != CMD_MF) || (type != CMD_TL) || (type != CMD_TR) || (type != CMD_STOP) || (type != CMD_MB))
    //    return false;
    //if(value > 253)
    //    return false;
    //if(speed > 63)
    //    return false;
    uint8_t cmd[CMD_LEN];
    cmd[CMD_INIT] = 0xFE;
    cmd[CMD_INDEX] = index;
    cmd[CMD_MSG_TYPE] = 0x32;
    cmd[CMD_TYPE] = type;
    cmd[CMD_VALUE] = value;
    cmd[CMD_SPEED] = speed;
    cmd[CMD_TERM] = 0xFF;

    if(index == 253)
        index = 0;

	// Send command over I2C.
    portBASE_TYPE retI2C = vtI2CEnQ(devI2C0, MSG_TYPE_ACK, SLAVE_ADDR, sizeof(cmd), cmd, 0);
    if(retI2C != pdTRUE)
    {
        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, "some error with blah", portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(retI2C);
    }
    else
        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, "bhalsjdf its working", portMAX_DELAY);
    index++;
    return (retI2C == pdTRUE);
}


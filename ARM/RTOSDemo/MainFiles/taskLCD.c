/*------------------------------------------------------------------------------
 * File:		LCDtask.c
 * Authors: 	FreeRTOS, Igor Janjic
 * Description:	File for implementing the LCD task.
 *----------------------------------------------------------------------------*/

#include "taskLCD.h"

static portTASK_FUNCTION(updateTaskLCD, pvParameters);

void startTaskLCD(structLCD* dataLCD, unsigned portBASE_TYPE uxPriority)
{
	if((dataLCD->inQ = xQueueCreate(queueLenLCD, sizeof(msgLCD))) == NULL)
		VT_HANDLE_FATAL_ERROR(0);

	portBASE_TYPE retval;
	if ((retval = xTaskCreate(updateTaskLCD, taskNameLCD, LCD_STACK_SIZE, (void*)dataLCD, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
		VT_HANDLE_FATAL_ERROR(retval);
}

portBASE_TYPE sendTimerMsgLCD(structLCD* dataLCD, portTickType ticksElapsed, portTickType ticksToBlock)
{
	msgLCD msg;
	msg.length = sizeof(ticksElapsed);

	memcpy(msg.buf, (char*)&ticksElapsed, sizeof(ticksElapsed));
	msg.type = MSG_TYPE_TIMER_LCD;
	return(xQueueSend(dataLCD->inQ, (void*)(&msg), ticksToBlock));
}

portBASE_TYPE sendValueMsgLCD(structLCD* dataLCD, uint8_t type, uint8_t length, char* value, portTickType ticksToBlock)
{
	msgLCD msg;
	if (length > maxLenLCD)
		VT_HANDLE_FATAL_ERROR(msg.length);

	msg.length = strnlen(value, maxLenLCD);
	msg.type = type;
	strncpy((char*)msg.buf, value, maxLenLCD);
	return(xQueueSend(dataLCD->inQ, (void*)(&msg), ticksToBlock));
}

static portTASK_FUNCTION(updateTaskLCD, pvParameters)
{
	msgLCD msg;
	structLCD *param = (structLCD*)pvParameters;

    debugIndex     = 0;
    sensorsIndex   = 0;

    debugCurLine   = 0;

	// Inspect to the stack remaining to see how much room is remaining.
	// 1. I'll check it here before anything really gets started.
	// 2. I'll check during the run to see if it drops below 10%.
	// 3. You could use break points or logging to check on this, but
	//    you really don't want to print it out because printf() can
	//    result in significant stack usage.
	// 4. Note that this checking is not perfect -- in fact, it will not
	//    be able to tell how much the stack grows on a printf() call and
	//    that growth can be *large* if version 1 of printf() is used.
	#ifdef INSPECT_STACK   
	unsigned portBASE_TYPE InitialStackLeft = uxTaskGetStackHighWaterMark(NULL);
	unsigned portBASE_TYPE CurrentStackLeft;
	float remainingStack = InitialStackLeft;
	remainingStack /= LCD_STACK_SIZE;
	if (remainingStack < 0.10) {
		// If the stack is really low, stop everything because we don't want it to run out
		// The 0.10 is just leaving a cushion, in theory, you could use exactly all of it
		VT_HANDLE_FATAL_ERROR(0);
	}
	#endif

	// Initialize the LCD and set the initial colors.
	GLCD_Init();
	GLCD_SetTextColor(Red);
	GLCD_SetBackColor(Blue);
	GLCD_Clear(Blue);

	// Note that srand() & rand() require the use of malloc() and should not be used unless you are using
	//   MALLOC_VERSION==1
	#if MALLOC_VERSION==1
	srand((unsigned) 55);
	#endif

	Tabs curTab = debug;
	drawTabs(curTab);

    // Configure joystick.

	// This task should never exit.
	for(;;)
	{
        // Change the current tab based on the button press.

		// Make sure there is enough room in the stack.	
		#ifdef INSPECT_STACK   
		CurrentStackLeft = uxTaskGetStackHighWaterMark(NULL);
		float remainingStack = CurrentStackLeft;
		remainingStack /= LCD_STACK_SIZE;
		if (remainingStack < 0.10)
			VT_HANDLE_FATAL_ERROR(0);
		#endif

		// Wait for a message
		if (xQueueReceive(param->inQ, (void *)&msg, portMAX_DELAY) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);
		
		// Log that we are processing a message.
		vtITMu8(vtITMPortLCDMsg, msg.type);
		vtITMu8(vtITMPortLCDMsg, msg.type);

		// Take a different action depending on the type of the message that we received.
		switch(msg.type)
		{
        case MSG_TYPE_TIMER_LCD:
        {
            break;
        }
		case MSG_TYPE_LCD_DEBUG:
		{
            if(curTab == debug)
			    updateTab(curTab, &msg);
			break;
		}
        case MSG_TYPE_LCD_SENSORS:
        {
            if(curTab == sensors)
                updateTab(curTab, &msg);
            break;
        }
        case MSG_TYPE_LCD_CMDS:
        {
            if(curTab == cmds)
                updateTab(curTab, &msg);
            break;
        }
        case MSG_TYPE_LCD_ROVER:
        {
            if(curTab == rover)
                updateTab(curTab, &msg);
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

void drawTabs(Tabs tab)
{
	switch(tab)
	{
    case debug:
    {
        drawDebug();
        break;
    }
    case sensors:
    {
        drawSensors();
        break;
    }
    case cmds:
    {
        drawCmds();
        break;
    }
    case rover:
    {
        drawRover();
        break;
    }
    default:
    {
		VT_HANDLE_FATAL_ERROR(0);
        break;
    }
	} 
}

void updateTab(Tabs tab, msgLCD* msg)
{		
	switch(tab)
	{
    case debug:
    {
        updateDebug(msg);
        break;
    }
    case sensors:
    {
        updateSensors(msg);
        break;
    }
    case cmds:
    {  	
        updateCmds(msg);
        break;
    }
    case rover:
    {
        updateRover(msg);
        break;
    }
    default:
    {
		VT_HANDLE_FATAL_ERROR(0);
        break;
    }
	}
}

void drawDebug()
{
    unsigned char initMsg[] = "Debug";
    GLCD_DisplayString(0, centerStr(initMsg, 1, 1), 1, initMsg);
    debugCurLine = TAB_INIT_LINE;
	GLCD_SetTextColor(Green);
}

void drawSensors()
{
    unsigned char initMsg[] = "Sensors";
    GLCD_DisplayString(0, centerStr(initMsg, 1, 1), 1, initMsg);

    // Draw the sensors template.
	GLCD_SetTextColor(DarkGreen);
    GLCD_DisplayString(2, 0, 1, (unsigned char*)"IR00:");
    GLCD_DisplayString(2, 11, 1, (unsigned char*)"IR01:");
    GLCD_DisplayString(3, 0, 1, (unsigned char*)"IR10:");
    GLCD_DisplayString(3, 11, 1, (unsigned char*)"IR11:");
    GLCD_DisplayString(4, 0, 1, (unsigned char*)"IR20:");
    GLCD_DisplayString(4, 11, 1, (unsigned char*)"IR21:");
    GLCD_DisplayString(5, 0, 1, (unsigned char*)"IR30:");
    GLCD_DisplayString(5, 11, 1, (unsigned char*)"IR31:");
    GLCD_DisplayString(6, 0, 1, (unsigned char*)"IR40:");
    GLCD_DisplayString(6, 11, 1, (unsigned char*)"IR41:");
    GLCD_DisplayString(7, 0, 1, (unsigned char*)"AC00:");

	GLCD_SetTextColor(Green);
}


void drawCmds()
{

}

void drawRover()
{

}


void updateDebug(msgLCD* msg)
{
    unsigned char printedMsg[maxLenLCD];
    sprintf((char*)printedMsg, "[%04d] ", debugIndex);
    strcat((char*)printedMsg, (const char*)msg->buf);
    if(debugCurLine == LINES_SMALL)
    {
        debugCurLine = TAB_INIT_LINE;
        GLCD_ClearWindow(0, 32, SCREEN_WIDTH, SCREEN_HEIGHT - 32, Blue);
    }
    GLCD_DisplayString(debugCurLine, 0, 0, printedMsg);
    debugCurLine++;
    debugIndex++;
}

void updateSensors(msgLCD* msg)
{
    unsigned char* sensorsMsg = (unsigned char*)msg->buf;  
    unsigned char parsedSensorsMsg[11][5];
    parseSensorsMsg(parsedSensorsMsg, sensorsMsg);

    // Draw the sensors values.
	GLCD_SetTextColor(DarkGreen);
    GLCD_DisplayString(2, 5,  1, parsedSensorsMsg[0]);
    GLCD_DisplayString(2, 16, 1, parsedSensorsMsg[1]);
    GLCD_DisplayString(3, 5,  1, parsedSensorsMsg[2]);
    GLCD_DisplayString(3, 16, 1, parsedSensorsMsg[3]);
    GLCD_DisplayString(4, 5,  1, parsedSensorsMsg[4]);
    GLCD_DisplayString(4, 16, 1, parsedSensorsMsg[5]);
    GLCD_DisplayString(5, 5,  1, parsedSensorsMsg[6]);
    GLCD_DisplayString(5, 16, 1, parsedSensorsMsg[7]);
    GLCD_DisplayString(6, 5,  1, parsedSensorsMsg[8]);
    GLCD_DisplayString(6, 16, 1, parsedSensorsMsg[9]);
    GLCD_DisplayString(7, 5,  1, parsedSensorsMsg[10]);
    sensorsIndex++;
}


void updateCmds(msgLCD* msg)
{

}

void updateRover(msgLCD* msg)
{

}

unsigned int centerStr(unsigned char* uncentStr, uint8_t fontInd, uint8_t alignInd)
{
	unsigned int lineLen;

	if(fontInd && alignInd)
		lineLen = SCREEN_WIDTH/FONT_LARGE_WIDTH;
	else if(!fontInd && alignInd)
		lineLen = SCREEN_WIDTH/FONT_SMALL_WIDTH;
	else if(fontInd && !alignInd)
		lineLen = SCREEN_HEIGHT/FONT_LARGE_HEIGHT;
	else if(!fontInd && !alignInd)
		lineLen = SCREEN_HEIGHT/FONT_SMALL_HEIGHT;
	else
		lineLen = 0; 

	return (lineLen - strlen((const char*)uncentStr))/2; 
}

void parseSensorsMsg(unsigned char parsedSensorsMsg[11][5], unsigned char* sensorsMsg)
{
    int i;
    int j = 0; 
    for(i = 0; i < maxLenLCD; i = i + 3)
    {
        strncpy((char*)parsedSensorsMsg[j], (const char*)sensorsMsg + i, 3);
        parsedSensorsMsg[j][3] = parsedSensorsMsg[j][2];
        parsedSensorsMsg[j][2] = '.';
        parsedSensorsMsg[j][4] = '\0';
        j++;
        if(j == 11)
         break;
    }
}

/*------------------------------------------------------------------------------
 * File:		LCDtask.c
 * Authors: 	FreeRTOS, Igor Janjic
 * Description:	File for implementing the LCD task.
 *----------------------------------------------------------------------------*/

#include "taskLCD.h"

static portTASK_FUNCTION(updateTaskLCD, pvParameters);

void startTaskLCD(structLCD* dataLCD, unsigned portBASE_TYPE uxPriority)
{
    // Create the queue that will be used to talk to this task.
    dataLCD->inQ = xQueueCreate(QUEUE_LEN_LCD, sizeof(msgLCD));
    if(dataLCD->inQ == NULL)
        VT_HANDLE_FATAL_ERROR(0);

    // Create the task.
    portBASE_TYPE retval = xTaskCreate(updateTaskLCD, taskNameLCD, LCD_STACK_SIZE, (void*)dataLCD, uxPriority, (xTaskHandle*)NULL);
    if(retval != pdPASS)
        VT_HANDLE_FATAL_ERROR(retval);
}

void sendTimerMsgLCD(structLCD* dataLCD, portTickType ticksElapsed, portTickType ticksToBlock)
{
	msgLCD msg;
	msg.length = sizeof(ticksElapsed);

	memcpy(msg.buf, (char*)&ticksElapsed, sizeof(ticksElapsed));
	msg.type = MSG_TYPE_TIMER_LCD;

    portBASE_TYPE retval = xQueueSend(dataLCD->inQ, (void*)(&msg), ticksToBlock);
    if(retval != pdTRUE)
        VT_HANDLE_FATAL_ERROR(retval);
}

void sendValueMsgLCD(structLCD* dataLCD, uint8_t type, uint8_t length, char* value, portTickType ticksToBlock)
{
	msgLCD msg;
	if (length > QUEUE_BUF_LEN_LCD)
		VT_HANDLE_FATAL_ERROR(msg.length);

	msg.length = strnlen(value, QUEUE_BUF_LEN_LCD);
	msg.type = type;
	strncpy((char*)msg.buf, value, QUEUE_BUF_LEN_LCD);

    portBASE_TYPE retval = xQueueSend(dataLCD->inQ, (void*)(&msg), ticksToBlock);
    if(retval != pdTRUE)
        VT_HANDLE_FATAL_ERROR(retval);
}

static portTASK_FUNCTION(updateTaskLCD, pvParameters)
{
    // Get pointers to parameter fields.
	structLCD *param = (structLCD*)pvParameters;

    // Buffer for receiving messages.
	msgLCD msg;

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
	GLCD_Clear(Black);
	GLCD_SetBackColor(Black);

	// Note that srand() & rand() require the use of malloc() and should not be used unless you are using
	//   MALLOC_VERSION==1
	#if MALLOC_VERSION==1
	srand((unsigned) 55);
	#endif

    TabData tabData = {
        .curTab = info,   
        .tabInfo = {
            .motion     = (char*)malloc(QUEUE_BUF_LEN_LCD * sizeof(char)),
            .goalPrime  = (char*)malloc(QUEUE_BUF_LEN_LCD * sizeof(char)),
            .goalSec    = (char*)malloc(QUEUE_BUF_LEN_LCD * sizeof(char)),
            .sizeMap    = (char*)malloc(QUEUE_BUF_LEN_LCD * sizeof(char)),
            .cmd        = (char*)malloc(QUEUE_BUF_LEN_LCD * sizeof(char)),
            .sensorData = (char*)malloc(QUEUE_BUF_LEN_LCD * sizeof(char)),

            .lineMotion     = 8,
            .lineGoalPrime  = 11,
            .lineGoalSec    = 14,
            .lineSizeMap    = 17,
            .lineCmd        = 20,
            .lineSensorData = 23
        },
        .tabEvents = {
            .initLine   = 4,
            .totalLines = LINES_SMALL - 4,
            .curLine    = 4,
            .curIndex   = 0,
        }
    };
    tabData.tabInfo.motion[0]     = '\0';
    tabData.tabInfo.goalPrime[0]  = '\0';
    tabData.tabInfo.goalSec[0]    = '\0';
    tabData.tabInfo.sizeMap[0]    = '\0';
    tabData.tabInfo.cmd[0]        = '\0';
    tabData.tabInfo.sensorData[0] = '\0';

    tabData.tabEvents.header = (char**)malloc(tabData.tabEvents.totalLines * sizeof(char*));
    tabData.tabEvents.body   = (char**)malloc(tabData.tabEvents.totalLines * sizeof(char*));
    uint8_t i;
    for (i = 0; i < tabData.tabEvents.totalLines; i++)
    {
        tabData.tabEvents.header[i] = (char*)malloc(QUEUE_BUF_LEN_LCD * sizeof(char));
        tabData.tabEvents.body[i]   = (char*)malloc(QUEUE_BUF_LEN_LCD * sizeof(char));
    }

    // First tab to display.
    drawInfo(&(tabData.tabInfo));

	// This task should never exit.
	for(;;)
	{
        toggleLED(PIN_LED_0);

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
            // Check the push button INT0 to see if the LCD tab must be changed.
            uint32_t pushRead = GPIO_ReadValue(PORT_PUSH);
			uint32_t push     = pushRead & PIN_PUSH;

            // Switch the LCD tab if need be.
            if(!push)
            {
				GLCD_Clear(Black);
                switch(tabData.curTab) 
                {
                case info:
                {
                    tabData.curTab = events;
                    drawEvents(&(tabData.tabEvents));
                    break;
                }
                case events:
                {
                    tabData.curTab = sensors;
                    drawSensors(&(tabData.tabSensors));
                    break;
                }
	            case sensors:
                {
                    tabData.curTab = map;
                    drawMap(&(tabData.tabMap));
                    break;
                } 
	            case map:
                {
                    tabData.curTab = info;
                    drawInfo(&(tabData.tabInfo));
                    break;
                }
                default:
                {
                    VT_HANDLE_FATAL_ERROR(0);
                }
                }
            }
            break;
        }
        case MSG_TYPE_LCD_MOTION:
        {
            updateInfo(&tabData, &msg, MSG_TYPE_LCD_MOTION);
            break;
        }
        case MSG_TYPE_LCD_GOAL_PRIME:
        {
            updateInfo(&tabData, &msg,MSG_TYPE_LCD_GOAL_PRIME);
            break;
        }
        case MSG_TYPE_LCD_GOAL_SEC:
        {
            updateInfo(&tabData, &msg, MSG_TYPE_LCD_GOAL_SEC);
            break;
        }
        case MSG_TYPE_LCD_SIZE_MAP:
        {
            updateInfo(&tabData, &msg, MSG_TYPE_LCD_SIZE_MAP);
            break;
        }
        case MSG_TYPE_LCD_CMD:
        {
            updateInfo(&tabData, &msg, MSG_TYPE_LCD_CMD);
            break;
        }
        case MSG_TYPE_LCD_SENS_DATA:
        {
            updateInfo(&tabData, &msg, MSG_TYPE_LCD_SENS_DATA);
            break;
        }
		case MSG_TYPE_LCD_EVENTS:
		{
			updateEvents(&tabData, &msg);
			break;
		}
        case MSG_TYPE_LCD_SENSORS:
        {
			updateSensors(&tabData, &msg);
            break;
        }
        case MSG_TYPE_LCD_MAP:
        {
			updateMap(&tabData, &msg);
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

void drawInfo(TabInfo* tabInfo)
{
   	GLCD_SetTextColor(Blue);
    unsigned char initMsg0[] = "Team 7";
    unsigned char initMsg1[] = "VT ECE 4564";
	GLCD_DisplayString(0, centerStr(initMsg0, 1, 1), 1, initMsg0);
    GLCD_DisplayString(1, centerStr(initMsg1, 1, 1), 1, initMsg1);
	GLCD_ClearWindow(30, 2*FONT_LARGE_HEIGHT + 2, SCREEN_WIDTH - 2*30, 2, DarkCyan);

    // Update each of the fields.
    GLCD_SetTextColor(Cyan);
	GLCD_DisplayString(tabInfo->lineMotion, 0, 0, (unsigned char*)"Motion: ");
    GLCD_SetTextColor(Green);
	GLCD_DisplayString(tabInfo->lineMotion + 1, 0, 0, (unsigned char*)(tabInfo->motion));

    GLCD_SetTextColor(Cyan);
	GLCD_DisplayString(tabInfo->lineGoalPrime, 0, 0, (unsigned char*)"Primary Goal: ");
    GLCD_SetTextColor(Green);
	GLCD_DisplayString(tabInfo->lineGoalPrime + 1, 0, 0, (unsigned char*)(tabInfo->goalPrime));

    GLCD_SetTextColor(Cyan);
    GLCD_DisplayString(tabInfo->lineGoalSec, 0, 0, (unsigned char*)"Secondary Goal: ");
    GLCD_SetTextColor(Green);
    GLCD_DisplayString(tabInfo->lineGoalSec + 1, 0, 0, (unsigned char*)(tabInfo->goalSec));

    GLCD_SetTextColor(Cyan);
    GLCD_DisplayString(tabInfo->lineSizeMap, 0, 0, (unsigned char*)"Allocated Map Size: ");
    GLCD_SetTextColor(Green);
    GLCD_DisplayString(tabInfo->lineSizeMap + 1, 0, 0, (unsigned char*)(tabInfo->sizeMap));

    GLCD_SetTextColor(Cyan);
    GLCD_DisplayString(tabInfo->lineCmd, 0, 0, (unsigned char*)"Last Command: ");
    GLCD_SetTextColor(Green);
    GLCD_DisplayString(tabInfo->lineCmd + 1, 0, 0, (unsigned char*)(tabInfo->cmd));

    GLCD_SetTextColor(Cyan);
    GLCD_DisplayString(tabInfo->lineSensorData, 0, 0, (unsigned char*)"Sensor Data: ");
    GLCD_SetTextColor(Green);
    GLCD_DisplayString(tabInfo->lineSensorData + 1, 0, 0, (unsigned char*)(tabInfo->sensorData));
}

void drawEvents(TabEvents* tabEvents)
{
	GLCD_SetTextColor(Blue);
    unsigned char initMsg[] = "0 :: Events";
    GLCD_DisplayString(0, centerStr(initMsg, 1, 1), 1, initMsg);
	GLCD_ClearWindow(30, FONT_LARGE_HEIGHT + 2, SCREEN_WIDTH - 2*30, 2, DarkCyan);

    uint8_t i;
    for(i = 0; i < tabEvents->curLine - tabEvents->initLine; i++)
	{
		GLCD_SetTextColor(Cyan);
	    GLCD_DisplayString(i + tabEvents->initLine, 0, 0, (unsigned char*)(tabEvents->header[i]));
	    GLCD_SetTextColor(Green);
	    GLCD_DisplayString(i + tabEvents->initLine, 7, 0, (unsigned char*)(tabEvents->body[i]));	
	}
}

void drawSensors(TabSensors* tabSensors)
{
	GLCD_SetTextColor(Blue);
    unsigned char initMsg[] = "1 :: Sensors";
    GLCD_DisplayString(0, centerStr(initMsg, 1, 1), 1, initMsg);
	GLCD_ClearWindow(30, FONT_LARGE_HEIGHT + 2, SCREEN_WIDTH - 2*30, 2, DarkCyan);
}


void drawMap(TabMap* tabMap)
{
    GLCD_SetTextColor(Blue);
    unsigned char initMsg[] = "2 :: Map";
    GLCD_DisplayString(0, centerStr(initMsg, 1, 1), 1, initMsg);
	GLCD_ClearWindow(30, FONT_LARGE_HEIGHT + 2, SCREEN_WIDTH - 2*30, 2, DarkCyan);
}

void updateInfo(TabData* tabData, msgLCD* msg, uint8_t type)
{
    TabInfo* tabInfo = &(tabData->tabInfo);

    switch(type)		  
    {
        case MSG_TYPE_LCD_MOTION:
        {
            strcpy(tabInfo->motion, (const char*)msg);
            if(tabData->curTab == info)
            {
                GLCD_SetTextColor(Cyan);
                GLCD_DisplayString(tabInfo->lineMotion, 0, 0, (unsigned char*)"Motion: ");
                GLCD_ClearLn(tabInfo->lineMotion + 1, 0);
                GLCD_SetTextColor(Green);
                GLCD_DisplayString(tabInfo->lineMotion + 1, 0, 0, (unsigned char*)(tabInfo->motion));
            }
            break;
        }
        case MSG_TYPE_LCD_GOAL_PRIME:
        {
            strcpy(tabInfo->goalPrime, (const char*)msg);
            if(tabData->curTab == info)
            {
                GLCD_SetTextColor(Cyan);
                GLCD_DisplayString(tabInfo->lineGoalPrime, 0, 0, (unsigned char*)"Primary Goal: ");
                GLCD_ClearLn(tabInfo->lineGoalPrime + 1, 0);
                GLCD_SetTextColor(Green);
                GLCD_DisplayString(tabInfo->lineGoalPrime + 1, 0, 0, (unsigned char*)(tabInfo->goalPrime));
            }
            break;
        }
        case MSG_TYPE_LCD_GOAL_SEC:
        {
            strcpy(tabInfo->goalSec, (const char*)msg);
            if(tabData->curTab == info)
            {
                GLCD_SetTextColor(Cyan);
                GLCD_DisplayString(tabInfo->lineGoalSec, 0, 0, (unsigned char*)"Secondary Goal: ");
                GLCD_ClearLn(tabInfo->lineGoalSec + 1, 0);
                GLCD_SetTextColor(Green);
                GLCD_DisplayString(tabInfo->lineGoalSec + 1, 0, 0, (unsigned char*)(tabInfo->goalSec));
            }
            break;
        }
        case MSG_TYPE_LCD_SIZE_MAP:
        {
            strcpy(tabInfo->sizeMap, (const char*)msg);
            if(tabData->curTab == info)
            {
                GLCD_SetTextColor(Cyan);
                GLCD_DisplayString(tabInfo->lineSizeMap, 0, 0, (unsigned char*)"Allocated Map Size: ");
                GLCD_ClearLn(tabInfo->lineSizeMap + 1, 0);
                GLCD_SetTextColor(Green);
                GLCD_DisplayString(tabInfo->lineSizeMap + 1, 0, 0, (unsigned char*)(tabInfo->sizeMap));
            }
            break;
        }
        case MSG_TYPE_LCD_CMD:
        {
            strcpy(tabInfo->cmd, (const char*)msg);
            if(tabData->curTab == info)
            {
                GLCD_SetTextColor(Cyan);
                GLCD_DisplayString(tabInfo->lineCmd, 0, 0, (unsigned char*)"Last Command: ");
                GLCD_ClearLn(tabInfo->lineCmd + 1, 0);
                GLCD_SetTextColor(Green);
                GLCD_DisplayString(tabInfo->lineCmd + 1, 0, 0, (unsigned char*)(tabInfo->cmd));
            }
            break;
        }
        case MSG_TYPE_LCD_SENS_DATA:
        {
            strcpy(tabInfo->sensorData, (const char*)msg);
            if(tabData->curTab == info)
            {
                GLCD_SetTextColor(Cyan);
                GLCD_DisplayString(tabInfo->lineSensorData, 0, 0, (unsigned char*)"Sensor Data: ");
                GLCD_ClearLn(tabInfo->lineSensorData + 1, 0);
                GLCD_SetTextColor(Green);
                GLCD_DisplayString(tabInfo->lineSensorData + 1, 0, 0, (unsigned char*)(tabInfo->sensorData));
            }
            break;
        }
        default:
        {
            VT_HANDLE_FATAL_ERROR(0);
        }
    }
}

void updateEvents(TabData* tabData, msgLCD* msg)
{
    TabEvents* tabEvents = &(tabData->tabEvents);
    if(tabEvents->curLine == LINES_SMALL)
    {
        tabEvents->curLine = tabEvents->initLine;
        if(tabData->curTab == events)
            GLCD_ClearWindow(0, 32, SCREEN_WIDTH, SCREEN_HEIGHT - 32, Black);
    }
	uint8_t dataIndex = tabEvents->curLine - tabEvents->initLine;
    sprintf((char*)tabEvents->header[dataIndex], "[%04d]", tabEvents->curIndex);
    sprintf((char*)tabEvents->body[dataIndex], "%s", (char*)msg->buf);

    // Update the display.
	if(tabData->curTab == events)
	{
		GLCD_SetTextColor(Cyan);
	    GLCD_DisplayString(tabEvents->curLine, 0, 0, (unsigned char*)(tabEvents->header[dataIndex]));
	    GLCD_SetTextColor(Green);
	    GLCD_DisplayString(tabEvents->curLine, 7, 0, (unsigned char*)(tabEvents->body[dataIndex]));
	}
    tabEvents->curLine++;
    tabEvents->curIndex++;
}

void updateSensors(TabData* tabData, msgLCD* msg)
{
	GLCD_SetTextColor(Green);
}

void updateMap(TabData* tabData, msgLCD* msg)
{
	GLCD_SetTextColor(Green);
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

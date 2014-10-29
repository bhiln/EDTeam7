/*------------------------------------------------------------------------------
 * File:		LCDtask.c
 * Authors: 	FreeRTOS, Igor Janjic
 * Description:	File for implementing the LCD task.
 *----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * Includes
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "FreeRTOS.h"
#include "task.h"

#include "GLCD.h"
#include "vtUtilities.h"
#include "taskLCD.h"
#include "string.h"
#include "myTimers.h"
#include "debug.h"

/*------------------------------------------------------------------------------
 * Preprocessor Commands
 */

// Defines a command for monitering the stack size to make sure that the stack
// does not overflow. 
#define INSPECT_STACK 1

// Set to a larger stack size because of (a) using printf() and (b) the depth of
// function calls for some of the LCD operations.
#define BASE_STACK 3
#if PRINTF_VERSION == 1
#define LCD_STACK_SIZE		((BASE_STACK+5)*configMINIMAL_STACK_SIZE)
#else
#define LCD_STACK_SIZE		(BASE_STACK*configMINIMAL_STACK_SIZE)
#endif

/*------------------------------------------------------------------------------
 * Private Data Structures
 */

#define vtLCDQLen 		10
#define LCDMsgTypeTimer 	1
#define LCDMsgTypePrint 	2

// Define various screen properties.
#define SCREEN_WIDTH 		320
#define SCREEN_HEIGHT 		240
#define SMALL_PIX_WIDTH 	6
#define SMALL_PIX_HEIGHT  	8
#define LARGE_PIX_WIDTH		16
#define LARGE_PIX_HEIGHT	24

// Define dimensions of the screen.
#define X_OFFSET		SMALL_PIX_WIDTH*6
#define Y_OFFSET		LARGE_PIX_HEIGHT + SMALL_PIX_HEIGHT
#define X_WIDTH			SCREEN_WIDTH - SMALL_PIX_WIDTH*13 + 2
#define Y_WIDTH			SCREEN_HEIGHT - LARGE_PIX_HEIGHT*2 - SMALL_PIX_HEIGHT*6 + 3

#define NUMBER_SAMPLES 		300			

typedef enum
{
	sensorVoltPlotIR00,
	sensorDistPlotIR00
} Tabs;

// Actual data structure that is sent in a message.
typedef struct __msgLCD {
	uint8_t msgType;
	uint8_t	length;
	uint8_t buf[maxLenLCD + 1];
} msgLCD;

portTickType unpackTimerMsg(msgLCD *bufferLCD);
int 	     getMsgType(msgLCD *bufferLCD);
int 	     getMsgLength(msgLCD *bufferLCD);
void 	     copyMsgString(char *target, msgLCD *bufferLCD, int targetMaxLen);
unsigned int centerStr(char* uncentStr, unsigned int fontInd, unsigned int alignInd);
unsigned int mapVolt2Pix(float volt);
unsigned int mapTime2Pix(float time);
unsigned int mapVolt2Dist(float volt);
unsigned int mapDist2Pix(float dist);
void 	     drawLCDIR0VoltPlot(char* tabName, unsigned int tabNameLine);
void 	     updateLCDIR0VoltPlot(msgLCD* msgBuffer);
void 	     drawLCDIR0DistPlot(char* tabName, unsigned int tabNameLine);
void 	     updateLCDIR0DistPlot(msgLCD* msgBuffer);
void 	     drawLCDTabs(Tabs tab);
void 	     updateLCDTabs(Tabs tab, msgLCD* msgBuffer);
static 	     portTASK_FUNCTION_PROTO(updateTaskLCD, pvParameters);

/*-----------------------------------------------------------------------------
 * Public API
 */

void startTaskLCD(structLCD *ptr, unsigned portBASE_TYPE uxPriority)
{
	if (ptr == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}

	// Create the queue that will be used to talk to this task.
	if ((ptr->inQ = xQueueCreate(vtLCDQLen, sizeof(msgLCD))) == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}

	// Start the task.
	portBASE_TYPE retval;
	if ((retval = xTaskCreate( updateTaskLCD, ( signed char * ) "LCD", LCD_STACK_SIZE, (void*)ptr, uxPriority, ( xTaskHandle * ) NULL )) != pdPASS) {
		VT_HANDLE_FATAL_ERROR(retval);
	}
}

portBASE_TYPE SendLCDTimerMsg(structLCD *lcdData, portTickType ticksElapsed, portTickType ticksToBlock)
{
	if (lcdData == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	msgLCD bufferLCD;
	bufferLCD.length = sizeof(ticksElapsed);
	if (bufferLCD.length > maxLenLCD) {
		// no room for this message
		VT_HANDLE_FATAL_ERROR(bufferLCD.length);
	}
	memcpy(bufferLCD.buf,(char *)&ticksElapsed,sizeof(ticksElapsed));
	bufferLCD.msgType = LCDMsgTypeTimer;
	return(xQueueSend(lcdData->inQ,(void *) (&bufferLCD),ticksToBlock));
}

portBASE_TYPE SendLCDPrintMsg(structLCD *lcdData,int length,char *pString,portTickType ticksToBlock)
{
	if (lcdData == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	msgLCD bufferLCD;

	if (length > maxLenLCD) {
		// no room for this message
		VT_HANDLE_FATAL_ERROR(bufferLCD.length);
	}
	bufferLCD.length = strnlen(pString,maxLenLCD);
	bufferLCD.msgType = LCDMsgTypePrint;
	strncpy((char *)bufferLCD.buf,pString,maxLenLCD);
	return(xQueueSend(lcdData->inQ,(void *) (&bufferLCD),ticksToBlock));
}

/*-----------------------------------------------------------------------------
 * Private Routines
 */

portTickType unpackTimerMsg(msgLCD *bufferLCD)
{
	portTickType *ptr = (portTickType *) bufferLCD->buf;
	return(*ptr);
}

int getMsgType(msgLCD *bufferLCD)
{
	return(bufferLCD->msgType);
} 

int getMsgLength(msgLCD *bufferLCD)
{
	return(bufferLCD->msgType);
}

void copyMsgString(char *target,msgLCD *bufferLCD,int targetMaxLen)
{
	strncpy(target,(char *)(bufferLCD->buf),targetMaxLen);
}

unsigned int centerStr(char* uncentStr, unsigned int fontInd, unsigned int alignInd)
{
	unsigned int lineLen;

	if(fontInd && alignInd)
		lineLen = SCREEN_WIDTH/LARGE_PIX_WIDTH;
	else if(!fontInd && alignInd)
		lineLen = SCREEN_WIDTH/SMALL_PIX_WIDTH;
	else if(fontInd && !alignInd)
		lineLen = SCREEN_HEIGHT/LARGE_PIX_HEIGHT;
	else if(!fontInd && !alignInd)
		lineLen = SCREEN_HEIGHT/SMALL_PIX_HEIGHT;
	else
		lineLen = 0; 

	return (lineLen - strlen(uncentStr))/2; 
}

unsigned int mapVolt2Pix(float volt)
{
	unsigned int volt2Pix = 0;
	float voltageWidth = 3;
	float pixPerVoltage = (float)(Y_WIDTH)/voltageWidth;
	float offset = (float)(SCREEN_HEIGHT - LARGE_PIX_HEIGHT - SMALL_PIX_HEIGHT*5 + 4);
	if(volt < 3)
		volt2Pix = (unsigned int)(offset - volt*pixPerVoltage);
  	return volt2Pix;
}

unsigned int mapTime2Pix(float time)
{
	unsigned int time2Pix = 0;
	float timeWidth = 8;
	float pixPerSec = (float)(X_WIDTH)/timeWidth;
	if(time < 8)
		time2Pix = (unsigned int)((X_OFFSET - 2 + X_WIDTH) - pixPerSec*time);
	return time2Pix;
}

unsigned int mapVolt2Dist(float volt)
{
	unsigned int volt2Dist = 0;
	float polyCoef2 = 47.25530465;
	float polyCoef1 = 193.4144014;
	float polyCoef0 = 216.5086058;
	volt2Dist = (unsigned int)(polyCoef2*volt*volt - polyCoef1*volt + polyCoef0);
	return volt2Dist;
}

unsigned int mapDist2Pix(float dist)
{
 	unsigned int dist2Pix = 0;
	float distWidth = 60;
	float pixPerDist = (float)(Y_WIDTH)/distWidth;
	float offset = (float)(SCREEN_HEIGHT - LARGE_PIX_HEIGHT - SMALL_PIX_HEIGHT*5 + 4);
	if(dist < 60)
		dist2Pix = (unsigned int)(offset - dist*pixPerDist);
  	return dist2Pix;
}

void drawLCDIR0VoltPlot(char* tabName, unsigned int tabNameLine)
{
	tabName = "Sensor 0";

	char* graphTitle = "IR";
	char* graphLabelX = "Time (s)";
	char* graphLabelY = "Voltage (V)";

	unsigned int tabOffset;
	unsigned int graphTitleOffset;
	unsigned int graphLabelXOffset;
	unsigned int graphLabelYOffset;
	  
	tabOffset 	  = centerStr(tabName, 1, 1);
	graphTitleOffset  = centerStr(graphTitle, 1, 1);
	graphLabelXOffset = centerStr(graphLabelX, 0, 1);
	graphLabelYOffset = centerStr(graphLabelY, 0, 0);

	unsigned int graphTitleLine  = 0;
	unsigned int graphLabelXLine = 25;
	unsigned int graphlabelYCol  = 1;

	GLCD_ClearLn(tabNameLine, 1);
	GLCD_ClearLn(graphTitleLine, 1);
	GLCD_ClearLn(graphLabelXLine, 0);

	// Draw the titles.
	GLCD_DisplayString(tabNameLine, tabOffset, 1, (unsigned char*)tabName);
	GLCD_DisplayString(graphTitleLine, graphTitleOffset, 1, (unsigned char*)graphTitle);

	// Draw vertical plot line.
	GLCD_ClearWindow(X_OFFSET, Y_OFFSET, 1, Y_WIDTH, Green);

	// Draw horizontal plot line.
	GLCD_ClearWindow(X_OFFSET - 2, SCREEN_HEIGHT - LARGE_PIX_HEIGHT - SMALL_PIX_HEIGHT*5, X_WIDTH, 1, Green);

	// Draw X label.
	GLCD_DisplayString(graphLabelXLine, graphLabelXOffset, 0, (unsigned char*)graphLabelX);

	// Draw Y label.
	unsigned int i;
	for(i = 0; i < strlen(graphLabelY); i++)
		GLCD_DisplayChar(i + graphLabelYOffset, graphlabelYCol, 0, (unsigned char)graphLabelY[i]);
	
	GLCD_SetTextColor(Green);
	// Draw the x ticks.
	int xTicks[9];
	for(i = 0; i < 9; i++)
		xTicks[i] = 8 - i;
	
	for(i = 0; i < 9; i++)
	{
		unsigned int ln = (SCREEN_HEIGHT - LARGE_PIX_HEIGHT - SMALL_PIX_HEIGHT*4)/SMALL_PIX_HEIGHT;
		int x = xTicks[i];
		unsigned char z = (unsigned char)((unsigned int)'0' + x%10);
		GLCD_DisplayChar(ln, 5 + 5*i, 0, '-');
		GLCD_DisplayChar(ln, 6 + 5*i, 0, z);	
	}
	
	// Draw the y ticks.
	int yTicks[6];
	for(i = 0; i < 6 ; i++)
		yTicks[i] = 30 - 5*i;

	for(i = 1; i < 20; i = i + 3) {
		unsigned int ln = i + 3;
		if(i == 19)
			GLCD_DisplayChar(ln, 4, 0, '0');
		else {
			int x = yTicks[i/3];
			unsigned char y = (unsigned char)((unsigned int)'0' + x/10);
			unsigned char z = (unsigned char)((unsigned int)'0' + x%10);
			GLCD_DisplayChar(ln, 3, 0, y);
			GLCD_DisplayChar(ln, 4, 0, '.');
			GLCD_DisplayChar(ln, 5, 0, z);
		}	
	}
}

void updateLCDIR0VoltPlot(msgLCD* msgBuffer)
{
	float histBuffer[NUMBER_SAMPLES] = {0};
	char lineBuffer[lcdCHAR_IN_LINE + 1];
	char currentVoltage[lcdCHAR_IN_LINE + 1];
   	
	copyMsgString(lineBuffer, msgBuffer, lcdCHAR_IN_LINE);
	if(lineBuffer == "0x11")
		sprintf(currentVoltage, "No data...");
	sprintf(currentVoltage, "Current Voltage = %s V", lineBuffer);

	float voltage = strtof(lineBuffer, NULL);

    // Display the current voltage.
	unsigned int offset = centerStr(currentVoltage, 0, 1);
	GLCD_ClearLn(3, 0);
	GLCD_DisplayString(3, offset, 0, (unsigned char*)currentVoltage);

	// Clear the old history buffer display.
	unsigned int i;
	GLCD_ClearWindow(X_OFFSET + 1, Y_OFFSET, X_WIDTH, Y_WIDTH - 3, Blue);	

	// Add the voltage to the history buffer.
	for(i = 0; i < NUMBER_SAMPLES - 1; i++)
		histBuffer[i] = histBuffer[i + 1];
	histBuffer[NUMBER_SAMPLES - 1] = voltage;

	//for(i = 0; i < NUMBER_SAMPLES; i++)
	//{
		//GLCD_ClearWindow(mapTime2Pix((float)((float)(i)*(float)((float)(ir0WRITE_RATE_BASE)/1000))), mapVolt2Pix(histBuffer[i]), 1, 1, Yellow);	
	//}

	float tempVolt = 2.5;
	float tempTime = 3.7;
	unsigned int y = mapVolt2Pix(tempVolt);
	unsigned int x = mapTime2Pix(tempTime);
	GLCD_ClearWindow(x, y, 4, 4, Yellow);
	
}

void drawLCDIR0DistPlot(char* tabName, unsigned int tabNameLine)
{
	tabName = "Sensor 0";

	char* graphTitle = "IR";
	char* graphLabelX = "Time (s)";
	char* graphLabelY = "Distance (cm)";

	unsigned int tabOffset;
	unsigned int graphTitleOffset;
	unsigned int graphLabelXOffset;
	unsigned int graphLabelYOffset;
	  
	tabOffset = centerStr(tabName, 1, 1);
	graphTitleOffset  = centerStr(graphTitle, 1, 1);
	graphLabelXOffset = centerStr(graphLabelX, 0, 1);
	graphLabelYOffset = centerStr(graphLabelY, 0, 0);

	unsigned int graphTitleLine  = 0;
	unsigned int graphLabelXLine = 25;
	unsigned int graphlabelYCol  = 1;

	GLCD_ClearLn(tabNameLine, 1);
	GLCD_ClearLn(graphTitleLine, 1);
	GLCD_ClearLn(graphLabelXLine, 0);

	// Draw the titles.
	GLCD_DisplayString(tabNameLine, tabOffset, 1, (unsigned char*)tabName);
	GLCD_DisplayString(graphTitleLine, graphTitleOffset, 1, (unsigned char*)graphTitle);

	// Draw vertical plot line.
	GLCD_ClearWindow(X_OFFSET, Y_OFFSET, 1, Y_WIDTH, Green);

	// Draw horizontal plot line.
	GLCD_ClearWindow(X_OFFSET - 2, SCREEN_HEIGHT - LARGE_PIX_HEIGHT - SMALL_PIX_HEIGHT*5, X_WIDTH, 1, Green);

	// Draw X label.
	GLCD_DisplayString(graphLabelXLine, graphLabelXOffset, 0, (unsigned char*)graphLabelX);

	// Draw Y label.
	unsigned int i;
	for(i = 0; i < strlen(graphLabelY); i++)
		GLCD_DisplayChar(i + graphLabelYOffset, graphlabelYCol, 0, (unsigned char)graphLabelY[i]);
	
	GLCD_SetTextColor(Green);

	// Draw the x ticks.
	int xTicks[9];
	for(i = 0; i < 9; i++)
		xTicks[i] = 8 - i;
	
	for(i = 0; i < 9; i++)
	{
		unsigned int ln = (SCREEN_HEIGHT - LARGE_PIX_HEIGHT - SMALL_PIX_HEIGHT*4)/SMALL_PIX_HEIGHT;
		int x = xTicks[i];
		unsigned char z = (unsigned char)((unsigned int)'0' + x%10);
		GLCD_DisplayChar(ln, 5 + 5*i, 0, '-');
		GLCD_DisplayChar(ln, 6 + 5*i, 0, z);	
	}
	
	// Draw the y ticks (0 -> 60 by 10).
	int yTicks[6];
	for(i = 0; i < 6 ; i++)
		yTicks[i] = 60 - 10*i;

	for(i = 1; i < 20; i = i + 3) {
		unsigned int ln = i + 3;
		if(i == 19)
			GLCD_DisplayChar(ln, 4, 0, '0');
		else {
			int x = yTicks[i/3];
			char yTicksStr[3] = {0};
			sprintf(yTicksStr, "%d", x);
			GLCD_DisplayString(ln, 4, 0, (unsigned char*)yTicksStr);
		}	
	}
}

void updateLCDIR0DistPlot(msgLCD* msgBuffer)
{
	float histBuffer[NUMBER_SAMPLES] = {0};
	histBuffer[0] = 1.0;
	histBuffer[1] = 1.1;
	histBuffer[2] = 1.2;
	histBuffer[3] = 1.3;
	histBuffer[4] = 1.4;
	histBuffer[5] = 1.5;
	histBuffer[6] = 1.6;
	char lineBuffer[lcdCHAR_IN_LINE + 1];
	char currentVoltage[lcdCHAR_IN_LINE + 1];
   	
	copyMsgString(lineBuffer, msgBuffer, lcdCHAR_IN_LINE);
	if(lineBuffer == "0x11")
		sprintf(currentVoltage, "No data...");
	sprintf(currentVoltage, "Current Voltage = %s V", lineBuffer);

	float voltage = strtof(lineBuffer, NULL);

    // Display the current voltage.
	unsigned int offset = centerStr(currentVoltage, 0, 1);
	GLCD_ClearLn(3, 0);
	GLCD_DisplayString(3, offset, 0, (unsigned char*)currentVoltage);

	// Clear the old history buffer display.
	unsigned int i;
	GLCD_ClearWindow(X_OFFSET + 1, Y_OFFSET, X_WIDTH, Y_WIDTH - 3, Blue);	

	// Add the voltage to the history buffer.
	for(i = 0; i < NUMBER_SAMPLES - 1; i++)
		histBuffer[i] = histBuffer[i + 1];
	histBuffer[NUMBER_SAMPLES - 1] = voltage;

	//for(i = 0; i < NUMBER_SAMPLES; i++)
	//{
		//unsigned int x = mapTime2Pix((float)((float)(i)*(float)((float)(ir0WRITE_RATE_BASE)/1000)));
		//unsigned int y = mapDist2Pix(mapVolt2Dist(histBuffer[i]));
		//GLCD_ClearWindow(x, y, 4, 4, Yellow);	
	//}

	float tempVolt3 = 1.3;	
	float tempDist3 = mapVolt2Dist(tempVolt3);
	float tempTime3 = 5.0;
	unsigned int y3 = mapDist2Pix(tempDist3);
	unsigned int x3 = mapTime2Pix(tempTime3);
	GLCD_ClearWindow(x3, y3, 4, 4, Yellow);

	float tempVolt = 1.5;
	float tempDist = mapVolt2Dist(tempVolt);
	float tempTime = 3.7;
	unsigned int y = mapDist2Pix(tempDist);
	unsigned int x = mapTime2Pix(tempTime);
	GLCD_ClearWindow(x, y, 4, 4, Yellow);

	float tempVolt2 = 2.5;	
	float tempDist2 = mapVolt2Dist(tempVolt2);
	float tempTime2 = 2.5;
	unsigned int y2 = mapDist2Pix(tempDist2);
	unsigned int x2 = mapTime2Pix(tempTime2);
	GLCD_ClearWindow(x2, y2, 4, 4, Yellow);
}

void drawLCDTabs(Tabs tab)
{
	char* tabName = 0;
	unsigned int tabNameLine = 9;

	switch(tab)
	{
		case sensorVoltPlotIR00:
		{
			drawLCDIR0VoltPlot(tabName, tabNameLine);
			break;
		}
		case sensorDistPlotIR00:
		{
			drawLCDIR0DistPlot(tabName, tabNameLine);
		}
		default:
		{
			break;
		}
	} 
}

void updateLCDTabs(Tabs tab, msgLCD* msgBuffer)
{		
	switch(tab)
	{
		case sensorVoltPlotIR00:
		{
			updateLCDIR0VoltPlot(msgBuffer);
			break;
		}
		case sensorDistPlotIR00:
		{  	
			updateLCDIR0DistPlot(msgBuffer);
			break;
		}
	}
}

// This is the actual task that is run.
static portTASK_FUNCTION(updateTaskLCD, pvParameters)
{
	unsigned timerCount = 0;

	msgLCD msgBuffer;
	structLCD *lcdPtr = (structLCD *) pvParameters;

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
	srand((unsigned) 55); // initialize the random number generator to the same seed for repeatability
	#endif

	Tabs curTab = sensorDistPlotIR00;
	drawLCDTabs(curTab);

	// This task should never exit.
	for(;;)
	{
		// Make sure there is enough room in the stack.	
		#ifdef INSPECT_STACK   
		CurrentStackLeft = uxTaskGetStackHighWaterMark(NULL);
		float remainingStack = CurrentStackLeft;
		remainingStack /= LCD_STACK_SIZE;
		if (remainingStack < 0.10) {
			// If the stack is really low, stop everything because we don't want it to run out
			VT_HANDLE_FATAL_ERROR(0);
		}
		#endif

		// Wait for a message
		if (xQueueReceive(lcdPtr->inQ, (void *) &msgBuffer, portMAX_DELAY) != pdTRUE) {
			VT_HANDLE_FATAL_ERROR(0);
		}
		
		// Log that we are processing a message.
		vtITMu8(vtITMPortLCDMsg, getMsgType(&msgBuffer));
		vtITMu8(vtITMPortLCDMsg, getMsgLength(&msgBuffer));

		// Take a different action depending on the type of the message that we received.
		switch(getMsgType(&msgBuffer))
		{

		case LCDMsgTypePrint:
		{
			updateLCDTabs(curTab, &msgBuffer);
			break;
		}
		case LCDMsgTypeTimer: {
			// Shift all the data back one second.
			if (timerCount == 0) {
				  	
			}
			else {

			}
			
			timerCount++;
			if (timerCount >= 1000) {
				
				timerCount = 0;
			}
			break;
		}
		default: {
			// In this configuration, we are only expecting to receive timer messages
			VT_HANDLE_FATAL_ERROR(getMsgType(&msgBuffer));
			break;
		}
		}
	}
}

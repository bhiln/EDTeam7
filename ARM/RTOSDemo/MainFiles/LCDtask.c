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

// Scheduler include files.
#include "FreeRTOS.h"
#include "task.h"

#include "GLCD.h"
#include "vtUtilities.h"
#include "LCDtask.h"
#include "string.h"

/*------------------------------------------------------------------------------
 * Preprocessor Commands
 */
// Defines a command for monitering the stack size to make sure that the stack
// does not overflow.
#define INSPECT_STACK 1

// Set to a larger stack size because of (a) using printf() and (b) the depth of
// function calls for some of the LCD operations.
#define baseStack 3
#if PRINTF_VERSION == 1
#define lcdSTACK_SIZE		((baseStack+5)*configMINIMAL_STACK_SIZE)
#else
#define lcdSTACK_SIZE		(baseStack*configMINIMAL_STACK_SIZE)
#endif

/*------------------------------------------------------------------------------
 * Private Data Structures
 */

#define vtLCDQLen 		10 	// Length of the queue to this task 
#define LCDMsgTypeTimer 1	// A timer message (not to be printed)
#define LCDMsgTypePrint 2 	// A message to be printed

// Actual data structure that is sent in a message.
typedef struct __vtLCDMsg {
	uint8_t msgType; 			// Type of message
	uint8_t	length;	 			// Length of the message to be printed
	uint8_t buf[vtLCDMaxLen + 1]; // On the way in, message to be sent, on the way out, message received (if any)
} vtLCDMsg;

// Definition for the LCD task.
static portTASK_FUNCTION_PROTO(vLCDUpdateTask, pvParameters);

void StartLCDTask(vtLCDStruct *ptr, unsigned portBASE_TYPE uxPriority)
{
	if (ptr == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}

	// Create the queue that will be used to talk to this task.
	if ((ptr->inQ = xQueueCreate(vtLCDQLen, sizeof(vtLCDMsg))) == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	// Start the task.
	portBASE_TYPE retval;
	if ((retval = xTaskCreate( vLCDUpdateTask, ( signed char * ) "LCD", lcdSTACK_SIZE, (void*)ptr, uxPriority, ( xTaskHandle * ) NULL )) != pdPASS) {
		VT_HANDLE_FATAL_ERROR(retval);
	}
}

portBASE_TYPE SendLCDTimerMsg(vtLCDStruct *lcdData, portTickType ticksElapsed, portTickType ticksToBlock)
{
	if (lcdData == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	vtLCDMsg lcdBuffer;
	lcdBuffer.length = sizeof(ticksElapsed);
	if (lcdBuffer.length > vtLCDMaxLen) {
		// no room for this message
		VT_HANDLE_FATAL_ERROR(lcdBuffer.length);
	}
	memcpy(lcdBuffer.buf,(char *)&ticksElapsed,sizeof(ticksElapsed));
	lcdBuffer.msgType = LCDMsgTypeTimer;
	return(xQueueSend(lcdData->inQ,(void *) (&lcdBuffer),ticksToBlock));
}

portBASE_TYPE SendLCDPrintMsg(vtLCDStruct *lcdData,int length,char *pString,portTickType ticksToBlock)
{
	if (lcdData == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	vtLCDMsg lcdBuffer;

	if (length > vtLCDMaxLen) {
		// no room for this message
		VT_HANDLE_FATAL_ERROR(lcdBuffer.length);
	}
	lcdBuffer.length = strnlen(pString,vtLCDMaxLen);
	lcdBuffer.msgType = LCDMsgTypePrint;
	strncpy((char *)lcdBuffer.buf,pString,vtLCDMaxLen);
	return(xQueueSend(lcdData->inQ,(void *) (&lcdBuffer),ticksToBlock));
}

// Private routines used to unpack the message buffers. I do not want to access
// the message buffer data structures outside of these routines.
portTickType unpackTimerMsg(vtLCDMsg *lcdBuffer)
{
	portTickType *ptr = (portTickType *) lcdBuffer->buf;
	return(*ptr);
}

int getMsgType(vtLCDMsg *lcdBuffer)
{
	return(lcdBuffer->msgType);
} 

int getMsgLength(vtLCDMsg *lcdBuffer)
{
	return(lcdBuffer->msgType);
}

void copyMsgString(char *target,vtLCDMsg *lcdBuffer,int targetMaxLen)
{
	strncpy(target,(char *)(lcdBuffer->buf),targetMaxLen);
}

/*------------------------------------------------------------------------------
 * LCD Task
 */

static unsigned short hsl2rgb(float H, float S, float L);

#define SCREEN_WIDTH 		320
#define SCREEN_HEIGHT 		240
#define SMALL_PIX_WIDTH 	6
#define SMALL_PIX_HEIGHT  	8
#define LARGE_PIX_WIDTH		16
#define LARGE_PIX_HEIGHT	24

#define X_OFFSET			SMALL_PIX_WIDTH*6
#define Y_OFFSET			LARGE_PIX_HEIGHT + SMALL_PIX_HEIGHT			

// Buffer in which to store the memory read from the LCD
#define MAX_RADIUS 15
#define BUF_LEN (((MAX_RADIUS*2) + 1)*((MAX_RADIUS*2) + 1))
static unsigned short int buffer[BUF_LEN];

typedef enum
{
	IRSensor
} Tabs;

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

void mapPixGraph(unsigned int x, unsigned int y, unsigned int* xMapped, unsigned int* yMapped)
{
	*xMapped = X_OFFSET + x;
	*yMapped = SCREEN_HEIGHT - LARGE_PIX_HEIGHT - SMALL_PIX_HEIGHT*5 + y;
}

void drawLCDTabs(Tabs tab)
{
	char* tabName;
	unsigned int tabNameLine = 9;

	switch(tab)
	{
		case IRSensor:
		{
			tabName = "Sensor 0";

			char* graphTitle = "IR";
			char* graphLabelX = "Time (s)";
			char* graphLabelY = "Voltage (V)";

			unsigned int tabOffset;
			unsigned int graphTitleOffset;
			unsigned int graphLabelXOffset;
			unsigned int graphLabelYOffset;
			  
			tabOffset 		  = centerStr(tabName, 1, 1);
			graphTitleOffset  = centerStr(graphTitle, 1, 1);
			graphLabelXOffset = centerStr(graphLabelX, 0, 1);
			graphLabelYOffset = centerStr(graphLabelY, 0, 0);

			unsigned int graphTitleLine  = 0;
			unsigned int graphLabelXLine = 25;
			unsigned int graphlabelYCol  = 1;

			GLCD_ClearLn(tabNameLine, 1);
			GLCD_ClearLn(graphTitleLine, 1);
			GLCD_ClearLn(graphLabelXLine, 0);

			GLCD_DisplayString(tabNameLine, tabOffset, 1, (unsigned char*)tabName);
			GLCD_DisplayString(graphTitleLine, graphTitleOffset, 1, (unsigned char*)graphTitle);

			// Draw vertical plot line.
			GLCD_ClearWindow(X_OFFSET, Y_OFFSET, 1, SCREEN_HEIGHT - LARGE_PIX_HEIGHT*2 - SMALL_PIX_HEIGHT*6 + 3, Green);

			// Draw horizontal plot line.
			GLCD_ClearWindow(X_OFFSET - 2, SCREEN_HEIGHT - LARGE_PIX_HEIGHT - SMALL_PIX_HEIGHT*5, SCREEN_WIDTH - SMALL_PIX_WIDTH*10, 1, Green);

			// Draw X label.
			GLCD_DisplayString(graphLabelXLine, graphLabelXOffset, 0, (unsigned char*)graphLabelX);

			// Draw Y label.
			unsigned int i;
			for(i = 0; i < strlen(graphLabelY); i++)
				GLCD_DisplayChar(i + graphLabelYOffset, graphlabelYCol, 0, (unsigned char)graphLabelY[i]);

			int xTicks[8];
			for(i = 0; i < 8; i++)
				xTicks[i] = 8 - i;
			
			int yTicks[6];
			for(i = 0; i < 6 ; i++)
				yTicks[i] = 30 - 5*i;

		   	GLCD_SetTextColor(Green);
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
			
			for(i = 0; i < 8; i++) {
				unsigned int ln = (SCREEN_HEIGHT - LARGE_PIX_HEIGHT - SMALL_PIX_HEIGHT*4)/SMALL_PIX_HEIGHT;
				int x = xTicks[i];
				unsigned char z = (unsigned char)((unsigned int)'0' + x%10);
				GLCD_DisplayChar(ln, 5 + 6*i, 0, '-');
				GLCD_DisplayChar(ln, 6 + 6*i, 0, z);	
			}
			
			/*unsigned int* xMapped;
			unsigned int* yMapped;
			unsigned int x = 10;
			unsigned int y = 10;
			mapPixGraph(x, y, xMapped, yMapped); 
			GLCD_ClearWindow(x, y, 1, 30, Green);*/

			break;
		}
		default:
		{
			break;
		}
	} 
}

// This is the actual task that is run
static portTASK_FUNCTION(vLCDUpdateTask, pvParameters)
{
	unsigned short screenColor = 0;
	unsigned short tscr;
	unsigned timerCount = 0;

	float hue = 0;
	float sat = 0.2;
	float light = 0.2;

	vtLCDMsg msgBuffer;
	vtLCDStruct *lcdPtr = (vtLCDStruct *) pvParameters;

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
	remainingStack /= lcdSTACK_SIZE;
	if (remainingStack < 0.10) {
		// If the stack is really low, stop everything because we don't want it to run out
		// The 0.10 is just leaving a cushion, in theory, you could use exactly all of it
		VT_HANDLE_FATAL_ERROR(0);
	}
	#endif

	// Initialize the LCD and set the initial colors.
	GLCD_Init();
	tscr = Red;
	screenColor = Blue;
	GLCD_SetTextColor(tscr);
	GLCD_SetBackColor(screenColor);
	GLCD_Clear(screenColor);

	// Note that srand() & rand() require the use of malloc() and should not be used unless you are using
	//   MALLOC_VERSION==1
	#if MALLOC_VERSION==1
	srand((unsigned) 55); // initialize the random number generator to the same seed for repeatability
	#endif

	Tabs curTab = IRSensor;
	drawLCDTabs(curTab);

	// This task should never exit.
	for(;;)
	{
		// Make sure there is enough room in the stack.	
		#ifdef INSPECT_STACK   
		CurrentStackLeft = uxTaskGetStackHighWaterMark(NULL);
		float remainingStack = CurrentStackLeft;
		remainingStack /= lcdSTACK_SIZE;
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

		case LCDMsgTypePrint: {

			break;
		}
		case LCDMsgTypeTimer: {
			// Note: if I cared how long the timer update was I would call my routine
			//    unpackTimerMsg() which would unpack the message and get that value
			// Each timer update will cause a circle to be drawn on the top half of the screen
			//   as explained below
			
			if (timerCount == 0) {

			} else {

			}
			
			timerCount++;
			if (timerCount >= 40) {	  
				// every so often, we reset timer count and start again
				// This isn't for any important reason, it is just to for this example code to do "stuff"
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

// Convert from HSL colormap to RGB values in this weird colormap
// H: 0 to 360
// S: 0 to 1
// L: 0 to 1
// The LCD has a funky bitmap.  Each pixel is 16 bits (a "short unsigned int")
//   Red is the most significant 5 bits
//   Blue is the least significant 5 bits
//   Green is the middle 6 bits
static unsigned short hsl2rgb(float H,float S,float L)
{
	float C = (1.0 - fabs(2.0*L-1.0))*S;
	float Hprime = H / 60;
	unsigned short t = Hprime / 2.0;
	t *= 2;
	float X = C * (1-abs((Hprime - t) - 1));
	unsigned short truncHprime = Hprime;
	float R1, G1, B1;

	switch(truncHprime) {
		case 0: {
			R1 = C; G1 = X; B1 = 0;
			break;
		}
		case 1: {
			R1 = X; G1 = C; B1 = 0;
			break;
		}
		case 2: {
			R1 = 0; G1 = C; B1 = X;
			break;
		}
		case 3: {
			R1 = 0; G1 = X; B1 = C;
			break;
		}
		case 4: {
			R1 = X; G1 = 0; B1 = C;
			break;
		}
		case 5: {
			R1 = C; G1 = 0; B1 = X;
			break;
		}
		default: {
			// make the compiler stop generating warnings
			R1 = 0; G1 = 0; B1 = 0;
			VT_HANDLE_FATAL_ERROR(Hprime);
			break;
		}
	}
	float m = L - 0.5*C;
	R1 += m; G1 += m; B1 += m;
	unsigned short red = R1*32; if (red > 31) red = 31;
	unsigned short green = G1*64; if (green > 63) green = 63;
	unsigned short blue = B1*32; if (blue > 31) blue = 31;
	unsigned short color = (red << 11) | (green << 5) | blue;
	return(color); 
}

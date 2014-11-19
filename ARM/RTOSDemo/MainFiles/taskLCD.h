/*------------------------------------------------------------------------------
 * File:		taskLCD.h
 * Authors: 	FreeRTOS, Igor Janjic
 * Description:	Specification file for the LCD task. Controls the LCD.
 *----------------------------------------------------------------------------*/

#ifndef LCD_TASK_H
#define LCD_TASK_H

/*------------------------------------------------------------------------------
 * Includes
 **/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "string.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

#include "GLCD.h"
#include "vtUtilities.h"

#include "debug.h"
#include "defs.h"

/*------------------------------------------------------------------------------
 * Definitions
 **/

// Length of the queue to this task.
#define queueLenLCD 	    15

// Maximum length of the buffer for this task.
#define maxLenLCD           50

// Screen properties.
#define SCREEN_WIDTH 		320
#define SCREEN_HEIGHT 		240

// Font properties.
#define FONT_SMALL_WIDTH    6
#define FONT_SMALL_HEIGHT  	8
#define FONT_LARGE_WIDTH	16
#define FONT_LARGE_HEIGHT	24

// Line properties.
#define LINES_SMALL         30
#define LINES_LARGE         30

// Tab properties
#define TAB_INIT_LINE       4
#define TAB_LINES           LINES_SMALL - TAB_INIT_LINE

/*------------------------------------------------------------------------------
 * Data Structures
 **/

typedef struct __structLCD {
	xQueueHandle inQ; 
} structLCD;

typedef struct __msgLCD
{
	uint8_t type;
	uint8_t	length;
	uint8_t buf[maxLenLCD + 1];
} msgLCD;


typedef enum
{
    debug,
	sensors,
	cmds,
    rover
} Tabs;

// Index of the current message for the corresponding tab.
unsigned int debugIndex;
unsigned int sensorsIndex;

// Line of the current message for the corresponding tab..
unsigned int debugCurLine;

/*------------------------------------------------------------------------------
 * Task API
 **/

void startTaskLCD(structLCD* dataLCD, unsigned portBASE_TYPE uxPriority);
portBASE_TYPE sendTimerMsgLCD(structLCD* dataLCD, portTickType ticksElapsed, portTickType ticksToBlock); 
portBASE_TYPE sendValueMsgLCD(structLCD* dataLCD, uint8_t type, uint8_t length, char* value, portTickType ticksToBlock);

/*------------------------------------------------------------------------------
 * Functions
 **/

void drawTabs(Tabs tab);
void updateTab(Tabs tab, msgLCD* msg);

void drawDebug();
void drawSensors();
void drawCmds();
void drawRover();

void updateDebug(msgLCD* msg);
void updateSensors(msgLCD* msg);
void updateCmds(msgLCD* msg);
void updateRover(msgLCD* msg);

unsigned int centerStr(unsigned char* uncentStr, uint8_t fontInd, uint8_t alignInd);
void parseSensorsMsg(unsigned char parsedSensorsMsg[11][5], unsigned char* sensorsMsg);

#endif

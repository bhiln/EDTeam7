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
#define QUEUE_LEN_LCD 	    15

// Maximum length of the buffer for this task.
#define QUEUE_BUF_LEN_LCD   50

// Screen properties.
#define SCREEN_WIDTH_PIX 	320
#define SCREEN_HEIGHT_PIX 	240
#define SCREEN_WIDTH_SMALL 	53
#define SCREEN_WIDTH_LARGE  20
#define SCREEN_HEIGHT_SMALL 30
#define SCREEN_HEIGHT_LARGE 10

// Font properties.
#define FONT_SMALL_WIDTH    6
#define FONT_SMALL_HEIGHT  	8
#define FONT_LARGE_WIDTH	16
#define FONT_LARGE_HEIGHT	24

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
	uint8_t buf[QUEUE_BUF_LEN_LCD];
} msgLCD;


typedef enum
{
    info,
    events,
	sensors,
	map
} Tab;

typedef struct __TabInfo
{
    char* motion;
    char* goalPrime;
    char* goalSec;
    char* sizeMap;
    char* cmd;
    char* sensorData0;
    char* sensorData1;

    uint8_t lineMotion;
    uint8_t lineGoalPrime;
    uint8_t lineGoalSec;
    uint8_t lineSizeMap;
    uint8_t lineCmd;
    uint8_t lineSensorData;
} TabInfo;

typedef struct __TabEvents
{
    uint8_t initLine;
    uint8_t totalLines;
    uint8_t curLine;
    uint16_t curIndex;
    char** header;
    char** body;
} TabEvents;

typedef struct __TabSensors
{
    uint8_t blah;
} TabSensors;

typedef struct __TabMap
{
    uint8_t bam;
} TabMap;

typedef struct __TabData
{
    Tab         curTab;
    TabInfo     tabInfo;
	TabEvents  	tabEvents;
	TabSensors	tabSensors;
	TabMap		tabMap;
} TabData;

// Index of the current message for the corresponding tab.
unsigned int debugIndex;

// Index of the sensor message.
unsigned int sensorsIndex;

// Line of the current message for the corresponding tab..
unsigned int debugCurLine;

/*------------------------------------------------------------------------------
 * Task API
 **/

void startTaskLCD(structLCD* dataLCD, unsigned portBASE_TYPE uxPriority);
void sendTimerMsgLCD(structLCD* dataLCD, portTickType ticksElapsed, portTickType ticksToBlock); 
void sendValueMsgLCD(structLCD* dataLCD, uint8_t type, uint8_t length, char* value, portTickType ticksToBlock);

/*------------------------------------------------------------------------------
 * Functions
 **/

void drawInfo(TabInfo* tabInfo);
void drawEvents(TabEvents* tabEvents);
void drawSensors(TabSensors* tabSensors);
void drawMap(TabMap* tabMap);


void updateTabInfo(TabData* tabData, msgLCD* msg, uint8_t type);
void updateTabEvents(TabData* tabData, msgLCD* msg);
void updateTabSensors(TabData* tabData, msgLCD* msg);
void updateTabMap(TabData* tabData, msgLCD* msg);

unsigned int centerStr(unsigned char* uncentStr, uint8_t fontInd, uint8_t alignInd);

#endif

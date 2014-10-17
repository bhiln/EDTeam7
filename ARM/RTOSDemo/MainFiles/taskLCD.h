#ifndef LCD_TASK_H
#define LCD_TASK_H
#include "queue.h"
#include "timers.h"

typedef struct __structLCD {
	xQueueHandle inQ; 
} structLCD;

/*------------------------------------------------------------------------------
 * Description:
 *    Starts the LCD task.
 * Args:
 *    *lcdData		LCD data structure.
 *    uxPriority	The priority of the task.
------------------------------------------------------------------------------*/
void startTaskLCD(structLCD* lcdData, unsigned portBASE_TYPE uxPriority);

portBASE_TYPE SendLCDTimerMsg(structLCD* lcdData, portTickType ticksElapsed, portTickType ticksToBlock);

portBASE_TYPE SendLCDPrintMsg(structLCD* lcdData, int length, char* pString, portTickType ticksToBlock);

void LCDTimerCallback(xTimerHandle);

#endif

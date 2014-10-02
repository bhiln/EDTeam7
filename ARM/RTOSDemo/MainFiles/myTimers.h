#ifndef MY_TIMERS_H
#define MY_TIMERS_H

#include "taskLCD.h"
#include "taskLocate.h"
#include "taskSensors.h"

#define LCD_UPDATE_TIME		300
#define lcdWRITE_RATE_BASE	((portTickType) LCD_UPDATE_TIME/portTICK_RATE_MS)

#define LOCATE_UPDATE_TIME	300
#define LOCATE_WRITE_RATE_BASE	((portTickType) LOCATE_UPDATE_TIME/portTICK_RATE_MS)

#define IR00_READS		8/8
#define IR00_WRITE_RATE_BASE	(((portTickType) (8000/(IR00_READS*8)))/portTICK_RATE_MS)

#define IR01_READS		8/8
#define IR01_WRITE_RATE_BASE	(((portTickType) (8000/(IR01_READS*8)))/portTICK_RATE_MS)

#define IR10_READS		8/8
#define IR10_WRITE_RATE_BASE	(((portTickType) (8000/(IR10_READS*8)))/portTICK_RATE_MS)

#define IR11_READS		8/8
#define IR11_WRITE_RATE_BASE	(((portTickType) (8000/(IR11_READS*8)))/portTICK_RATE_MS)

#define IR20_READS		8/8
#define IR20_WRITE_RATE_BASE	(((portTickType) (8000/(IR20_READS*8)))/portTICK_RATE_MS)

#define IR21_READS		8/8
#define IR21_WRITE_RATE_BASE	(((portTickType) (8000/(IR21_READS*8)))/portTICK_RATE_MS)

#define IR30_READS		8/8
#define IR30_WRITE_RATE_BASE	(((portTickType) (8000/(IR30_READS*8)))/portTICK_RATE_MS)

#define IR31_READS		8/8
#define IR31_WRITE_RATE_BASE	(((portTickType) (8000/(IR31_READS*8)))/portTICK_RATE_MS)

#define IR40_READS		8/8
#define IR40_WRITE_RATE_BASE	(((portTickType) (8000/(IR40_READS*8)))/portTICK_RATE_MS)

#define IR41_READS		8/8
#define IR41_WRITE_RATE_BASE	(((portTickType) (8000/(IR41_READS*8)))/portTICK_RATE_MS)

#define AC00_READS		8/8
#define AC00_WRITE_RATE_BASE	(((portTickType) (8000/(AC00_READS*8)))/portTICK_RATE_MS)

void startTimerLCD(structLCD* dataLCD);
void startTimerLocate(structLocate* dataLocate);
void startTimerSensor(uint8_t sensor, structSensor* dataIR00);

#endif

#ifndef MY_TIMERS_H
#define MY_TIMERS_H

#include "taskLCD.h"
#include "taskLocate.h"
#include "taskCommand.h"
#include "taskSensors.h"

#define LCD_UPDATE_TIME		300
#define lcdWRITE_RATE_BASE	((portTickType) LCD_UPDATE_TIME/portTICK_RATE_MS)

#define LOCATE_UPDATE_TIME	300
#define LOCATE_WRITE_RATE_BASE	((portTickType) LOCATE_UPDATE_TIME/portTICK_RATE_MS)

#define COMMAND_UPDATE_TIME	300
#define COMMAND_WRITE_RATE_BASE	((portTickType) COMMAND_UPDATE_TIME/portTICK_RATE_MS)

// Number of times to query slave PIC for sensor data (# reads/8 seconds)
#define SENSOR_READS		8/8
#define SENSOR_WRITE_RATE_BASE	(((portTickType) (8000/(SENSOR_READS*8)))/portTICK_RATE_MS)

void startTimerLCD(structLCD* dataLCD);
void startTimerLocate(structLocate* dataLocate);
void startTimerCommand(structCommand* dataCommand);
void startTimerSensor(structSensor* dataSensor);

#endif

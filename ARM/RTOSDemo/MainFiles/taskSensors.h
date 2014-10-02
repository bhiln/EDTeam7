#ifndef TASK_SENSORS_H
#define TASK_SENSORS_H

#include "vtI2C.h"
#include "taskLCD.h"

// Maximum length of a message that can be received by any IR task.
#define maxLenIR (sizeof(portTickType))

// Sensor ID.
#define sensorIR00	0x00
#define sensorIR01	0x01
#define sensorIR10	0x10
#define sensorIR11	0x11
#define sensorIR20	0x20
#define sensorIR21	0x21
#define sensorIR30	0x30
#define sensorIR31	0x31
#define sensorIR40	0x40
#define sensorIR41	0x41
#define sensorAC00	0x50

signed char sensorNameIR00[] = "IR00";
signed char sensorNameIR01[] = "IR01";
signed char sensorNameIR10[] = "IR10";
signed char sensorNameIR11[] = "IR11";
signed char sensorNameIR20[] = "IR20";
signed char sensorNameIR21[] = "IR21";
signed char sensorNameIR30[] = "IR30";
signed char sensorNameIR31[] = "IR31";
signed char sensorNameIR40[] = "IR40";
signed char sensorNameIR41[] = "IR41"; 
signed char sensorNameAC00[] = "AC00"; 

// Structure used to pass parameters to the task.
typedef struct __structSensor {
	vtI2CStruct *devI2C0;
	structLCD *dataLCD;
	xQueueHandle inQ;
} structSensor;

// Sensor sensor.
void startTaskSensor(uint8_t sensor, structSensor* dataSensor, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C, structLCD* dataLCD);
portBASE_TYPE sendTimerMsgSensor(structSensor* dataSensor, uint8_t msgType, portTickType ticksElapsed, portTickType ticksToBlock);
portBASE_TYPE sendValueMsgSensor(structSensor* dataSensor, uint8_t msgType, uint8_t value, portTickType ticksToBlock);

#endif

#ifndef I2CIR0_TASK_H
#define I2CIR0_TASK_H

#include "vtI2C.h"
#include "lcdTask.h"

// Structure used to pass parameters to the task
// Do not touch...
typedef struct __IR0Struct {
	vtI2CStruct *dev;
	vtLCDStruct *lcdData;
	xQueueHandle inQ;
} vtIR0Struct;

// Maximum length of a message that can be received by this task
#define vtIR0MaxLen   (sizeof(portTickType))

// Public API
//
// Start the task
// Args:
//   ir0Data: Data structure used by the task
//   uxPriority -- the priority you want this task to be run at
//   i2c: pointer to the data structure for an i2c task
//   lcd: pointer to the data structure for an LCD task (may be NULL)
void vStarti2cIR0Task(vtIR0Struct *ir0Data, unsigned portBASE_TYPE uxPriority, vtI2CStruct *i2c, vtLCDStruct *lcd);

// Send a timer message to the IR0 task
// Args:
//   ir0Data -- a pointer to a variable of type vtLCDStruct
//   ticksElapsed -- number of ticks since the last message (this will be sent in the message)
//   ticksToBlock -- how long the routine should wait if the queue is full
// Return:
//   Result of the call to xQueueSend()
portBASE_TYPE SendIR0TimerMsg(vtIR0Struct *ir0Data, portTickType ticksElapsed, portTickType ticksToBlock);

// Send a value message to the IR0 task
// Args:
//   ir0Data -- a pointer to a variable of type vtLCDStruct
//   msgType -- the type of the message to send
//   value -- The value to send
//   ticksToBlock -- how long the routine should wait if the queue is full
// Return:
//   Result of the call to xQueueSend()
portBASE_TYPE SendIR0ValueMsg(vtIR0Struct *ir0Data, uint8_t msgType, uint8_t value, portTickType ticksToBlock);
#endif
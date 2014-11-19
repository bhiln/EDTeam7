/*------------------------------------------------------------------------------
 * File:	    	taskLocate.h
 * Authors: 		FreeRTOS, Igor Janjic
 * Description:		Specification file for locate task. Determines location of
 *                  rover and ramps.
 *----------------------------------------------------------------------------*/

#ifndef TASK_LOCATE_H
#define TASK_LOCATE_H

/*------------------------------------------------------------------------------
 * Includes
 **/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "FreeRTOS.h"
#include "lpc17xx_gpio.h"
#include "projdefs.h"
#include "semphr.h"
#include "task.h"

#include "vtUtilities.h"
#include "vtI2C.h"

#include "defs.h"
#include "matrix.h"
#include "taskCommand.h"
#include "taskLCD.h"

/*------------------------------------------------------------------------------
 * Definitions
 **/

 // Length of the queue to this task.
#define queueLenLocate  10

// Maximum length of a message that can be received by the task.
#define bufLenLocate    11

// Max distance that will provide reliable sensor data (experimentally found).
#define SENS_DIST_CUTOFF    80

// Maximum radius of the map (experimentally found).
#define MAP_RADIUS_MAX        10000

// Initial size of the map (experimentally found).
#define MAP_RADIUS_INIT       101

// Units: number of cm per block.
#define UNITS                 1

/*------------------------------------------------------------------------------
 * Data Structures
 **/

// Structure used to pass parameters to the task.
typedef struct __structLocate
{
	structLCD*     dataLCD;
    structCommand* dataCommand;
	xQueueHandle   inQ;
} structLocate;

// Actual data structure that is sent in a message.
typedef struct __msgLocate
{
	uint8_t type;
	uint8_t	length;
	float buf[bufLenLocate];
} msgLocate;

/*------------------------------------------------------------------------------
 * State Enumerations
 **/

// Enum of the possible states of rover motion.
typedef enum 
{
	stop,
	moveForward,
	moveBackward,
	turnLeft,
	turnRight
} StateMotion;

// Enum of the possible states of prime goal.
typedef enum
{
    scan,
    roam,
    go,
    align,
    ramp,
	none,
} StatePrimeGoal;

typedef enum
{
    scanInit,
    scanExec,
    scanComp,
	scanError
} StateScanSecGoal;

typedef enum
{
    roamInit,
    roamExec,
    roamComp,
	roamError
} StateRoamSecGoal;

typedef enum
{
  	goInit,
	goExec,
	goComp,
	goError
} StateGoSecGoal;

typedef enum
{
   	alignInit,
	alignExec,
	alignComp,
	alignError
} StateAlignSecGoal;

typedef enum
{
   	rampInit,
	rampExec,
	rampComp,
	rampError
} StateRampSecGoal;

/*------------------------------------------------------------------------------
 * Data Structures
 **/

typedef struct __States
{
    StateMotion       curStateMotion;
    StatePrimeGoal    curStatePrimeGoal;
    StateScanSecGoal  curStateScanSecGoal;
    StateRoamSecGoal  curStateRoamSecGoal;
	StateGoSecGoal    curStateGoSecGoal;
	StateAlignSecGoal curStateAlignSecGoal;
	StateRampSecGoal  curStateRampSecGoal;
} States;

typedef struct __Map
{
    Matrix  map;
    Vector  radix;
    Vector  thresh;
    bool    grow;
} Map;

typedef struct __Rover
{
    States curStates;
    Matrix curObstacles;
    Matrix curCorners;
    Matrix curRamps;
    Vector curTarget;
    float  orient;
    bool   mapped;
    bool   ack;
} Rover;

// Initial states of the rover.
const StateMotion       initialStateMotion       = stop;
const StatePrimeGoal    initialStatePrimeGoal    = scan;
const StateScanSecGoal  initialStateScanSecGoal  = scanInit;
const StateRoamSecGoal  initialStateRoamSecGoal  = roamInit;
const StateGoSecGoal    initialStateGoSecGoal    = goInit;
const StateAlignSecGoal initialStateAlignSecGoal = alignInit;
const StateRampSecGoal  initialStateRampSecGoal  = rampInit;

/*------------------------------------------------------------------------------
 * Mapper Data Structures
 **/

/*------------------------------------------------------------------------------
 * Task API
 **/

void startTaskLocate(structLocate* dataLocate, unsigned portBASE_TYPE uxPriority, structLCD* dataLCD, structCommand* dataCommand);
void sendTimerMsgLocate(structLocate* dataLocate, portTickType ticksElapsed, portTickType ticksToBlock);
void sendValueMsgLocate(structLocate* dataLocate, uint8_t type, float* value, portTickType ticksToBlock);

/*------------------------------------------------------------------------------
 * Matrix Functions
 **/
void updateData(Rover* rover, Map* map, float* data);
void mapData(Rover* rover, Map* map);

#endif

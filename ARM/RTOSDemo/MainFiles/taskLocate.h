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
#define QUEUE_LEN_LOCATE        10

// Maximum length of a message that can be received by the task.
#define QUEUE_BUF_LEN_LOCATE    11

// Max distance that will provide reliable sensor data (experimentally found).
#define SENS_DIST_CUTOFF        50

// Maximum radius of the map (experimentally found).
#define MAP_RADIUS_MAX          10000

// Initial size of the map (experimentally found).
#define MAP_RADIUS_INIT         51

// Units: number of cm per block.
#define UNITS                   1

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
	float buf[QUEUE_BUF_LEN_LOCATE];
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
    States curStates;	 // State machine of the rover
    Matrix curObstacles; // Matrix of size (2 x 4) containing locations of obstacles orthogonally seperated from the front of the rover
    Matrix curCorners;	 // Matrix of size (2 x 4) containing data of current corners that have been discovered
    Matrix curRamps; 	 // Matrix of size (2 x 3) containing the locations of the last three ramps discovered
    Vector curTarget;	 // Vector of size 2 containing the location of the target ramp
	Vector curRawObst;   // Vector of size 8 containing the raw sensor data relevant for obstacle detection
	Vector curRawRamp; 	 // Vector of size 2 containing the raw sensor data relevant for ramp detection
    float  orient;		 // Orientation of the rover
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

char givenMap[] = "10 -12 -32 0 -32 0  -7 5  -7 5 -14 25 -14 25   0 0   0 0  47 -12  47 2 -8  43 0   1 1   0 15  -7 1   0 1   0 -8 -24";

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

/*------------------------------------------------------------------------------
 * State Functions
 **/

void execScan(vtI2CStruct* devI2C0, structCommand* dataCommand, Rover* rover, Map* map);
void execRoam(vtI2CStruct* devI2C0);
void execGo(vtI2CStruct* devI2C0);
void execAlign(vtI2CStruct* devI2C0);
void execRamp(vtI2CStruct* devI2C0);
void execDefault();

/*------------------------------------------------------------------------------
 * Helper Functions
 **/

// Secondary state execution functions.
bool execMoveAlong(vtI2CStruct* devI2C0);
bool execParallelize(vtI2CStruct* devI2C0);
bool execMoveZigzag(vtI2CStruct* devI2C0);
bool execTurnCorner(vtI2CStruct* devI2C0);
bool execTurnAround(vtI2CStruct* devI2C0);
bool execMoveToward(vtI2CStruct* devI2C0);
bool execMoveTowardRamp(vtI2CStruct* devI2C0);

#endif

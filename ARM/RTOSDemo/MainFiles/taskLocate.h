#ifndef TASK_LOCATE_H
#define TASK_LOCATE_H

#include "vtI2C.h"
#include "taskLCD.h"

// Maximum length of a message that can be received by any IR task.
#define maxLenLocate (sizeof(portTickType))

#define MAX_COMMAND_HISTORY 100
#define NUMBER_SENSORS 		11

#define roverFront	0
#define roverRight 	1
#define roverBack  	2
#define roverLeft  	3

signed char taskNameLocate[] = "Locate";

// Structure used to pass parameters to the task.
typedef struct __structLocate
{
	structLCD *dataLCD;
	xQueueHandle inQ;
} structLocate;

// Struct representing a command.
typedef struct __Command
{
	uint8_t commandType;
	uint8_t commandValue;
	uint8_t commandSpeed;
} Command;

// Struct containing the distance and angle for an object on a specific side of
// the rover.
typedef struct __Object
{
	int16_t distance;
	int16_t angle;
} Object;

// Enum of the possible states of rover motion.
typedef enum 
{
	stopped,
	emergencyStoppped,
	movingForward,
	movingBackward,
	turningLeft,
	turningRight,
	unknown
} StateMotion;

// Enum of the possible states of rover localization.
typedef enum
{
	none,
	roam,
	followWall,
	turnCornerLeft,
	turnCornerRight,
	turnAround,
	moveTowardObject,
	moveTowardRamp,
	alignWithRamp,
	moveOverRamp
} StateGoal;

// List of sensor distance values.
uint16_t locateData[NUMBER_SENSORS];

// History of the past commands.
Command commandHistory[MAX_COMMAND_HISTORY];

// List containing front, right, back, and left rover objects.
Object sideObject[3];

// Closest ramp.
Object ramp;

// State machine.
StateMotion curStateMotion = stopped;
StateGoal curStateGoal = none;

// Sensor sensor.
void startTaskLocate(structLocate* dataLocate, unsigned portBASE_TYPE uxPriority, structLCD* dataLCD);
portBASE_TYPE sendTimerMsgLocate(structLocate* dataLocate, portTickType ticksElapsed, portTickType ticksToBlock);
portBASE_TYPE sendValueMsgLocate(structLocate* dataLocate, uint8_t msgType, uint16_t value, portTickType ticksToBlock);

void locateRover();
void updateRamp();

void execNone();
void execRoam();
void execFollowWall();
void execTurnCornerLeft();
void execTurnCornerRight();
void execTurnAround();
void execMoveTowardObject();
void execMoveTowardRamp();
void execAlignWithRamp();
void execMoveOverRamp();

#endif

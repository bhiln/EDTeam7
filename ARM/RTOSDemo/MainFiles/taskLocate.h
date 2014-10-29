#ifndef TASK_LOCATE_H
#define TASK_LOCATE_H

#include "defs.h"
#include "taskLCD.h"
#include "vtI2C.h"

// Structure used to pass parameters to the task.
typedef struct __structLocate
{
    vtI2CStruct* devI2C0;
	structLCD*   dataLCD;
	xQueueHandle inQ;
} structLocate;

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
	turnRight,
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
    scanComp
} StateScanSecGoal;

typedef enum
{
    roamInit,
    roamExec,
    roamComp
} StateRoamSecGoal;

typedef enum
{
  	goInit,
	goExec,
	goComp,
} StateGoSecGoal;

typedef enum
{
   	alignInit,
	alignExec,
	alignComp
} StateAlignSecGoal;

typedef enum
{
   	rampInit,
	rampExec,
	rampComp
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

/*------------------------------------------------------------------------------
 * Task Functions
 **/

void startTaskLocate(structLocate* dataLocate, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0, structLCD* dataLCD);
portBASE_TYPE sendTimerMsgLocate(structLocate* dataLocate, portTickType ticksElapsed, portTickType ticksToBlock);
portBASE_TYPE sendValueMsgLocate(structLocate* dataLocate, uint8_t msgType, uint16_t* value, portTickType ticksToBlock);


/*------------------------------------------------------------------------------
 * State Functions
 **/

void execScan   (vtI2CStruct* devI2C0, States* curStates, uint16_t curObjs[SIDE_LEN][OBJ_LEN], uint16_t curRamp[OBJ_LEN], uint16_t targetRamp[OBJ_LEN], bool* ack);
void execRoam   (vtI2CStruct* devI2C0, States* curStates, uint16_t curObjs[SIDE_LEN][OBJ_LEN], uint16_t curRamp[OBJ_LEN], uint16_t targetRamp[OBJ_LEN], bool* ack);
void execGo     (vtI2CStruct* devI2C0, States* curStates, uint16_t curObjs[SIDE_LEN][OBJ_LEN], uint16_t curRamp[OBJ_LEN], bool* ack);
void execAlign  (vtI2CStruct* devI2C0, States* curStates, uint16_t curObjs[SIDE_LEN][OBJ_LEN], uint16_t curRamp[OBJ_LEN], bool* ack);
void execRamp   (vtI2CStruct* devI2C0, States* curStates, uint16_t curObjs[SIDE_LEN][OBJ_LEN], uint16_t curRamp[OBJ_LEN], bool* ack);
void execDefault();

/*------------------------------------------------------------------------------
 * Helper Functions
 **/

// Secondary state execution functions.
bool execMoveAlong     (vtI2CStruct* devI2C0, uint16_t curObjs[SIDE_LEN][OBJ_LEN]);
bool execParallelize   (vtI2CStruct* devI2C0);
bool execMoveZigzag    (vtI2CStruct* devI2C0, uint16_t curObjs[SIDE_LEN][OBJ_LEN]);
bool execTurnCorner    (vtI2CStruct* devI2C0);
bool execTurnAround    (vtI2CStruct* devI2C0);
bool execMoveToward    (vtI2CStruct* devI2C0);
bool execMoveTowardRamp(vtI2CStruct* devI2C0);

// Motion command function.
bool execCommand(vtI2CStruct* devI2C0, uint8_t type, uint8_t value, uint8_t speed);

// Locate nearest rover objects.
void locateObjs(uint16_t curObjs[SIDE_LEN][OBJ_LEN], uint16_t* locateData);

// Locate ramps on map.
void locateRamp(uint16_t curRamp[OBJ_LEN], uint16_t* locateData);

// Returns true if there are any ramps nearby.
bool anyRamps(uint16_t curRamp[OBJ_LEN]);

// Get the closest object near the rover.
void getClosestCurObj(uint8_t* side, uint16_t closestObj[OBJ_LEN], uint16_t curObjs[SIDE_LEN][OBJ_LEN]);

#endif

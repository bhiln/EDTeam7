/*------------------------------------------------------------------------------
 * File:		taskLocate.c
 * Authors: 		FreeRTOS, Igor Janjic
 * Description:		Determines location of rover and ramps.
 **---------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * Includes
 **/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "projdefs.h"
#include "semphr.h"
#include "lpc17xx_gpio.h"

#include "vtUtilities.h"
#include "taskLCD.h"
#include "taskLocate.h"
#include "I2CTaskMsgTypes.h"
#include "debug.h"

/*------------------------------------------------------------------------------
 * Configuration
 **/

 // Length of the queue to this task.
#define queueLenLocate 10

// Stack sizes.
#define BASE_STACK 3
#if PRINTF_VERSION == 1
#define I2C_STACK_SIZE		((BASE_STACK+5)*configMINIMAL_STACK_SIZE)
#else
#define I2C_STACK_SIZE		(BASE_STACK*configMINIMAL_STACK_SIZE)
#endif

// Actual data structure that is sent in a message.
typedef struct __msgLocate
{
	uint8_t msgType;
	uint8_t	length;
	uint16_t buf[maxLenLocate + 1];
} msgLocate;

/*------------------------------------------------------------------------------
 * Task Functions
 **/

int getMsgTypeLocate(msgLocate *Buffer) {return(Buffer->msgType);}

static portTASK_FUNCTION(updateTaskLocate, pvParameters);

void startTaskLocate(structLocate* dataLocate, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0, structLCD* dataLCD)
{
	// Create the queue that will be used to talk to this task.
	if ((dataLocate->inQ = xQueueCreate(queueLenLocate, sizeof(msgLocate))) == NULL)
		VT_HANDLE_FATAL_ERROR(0);

	// Create the task.
	portBASE_TYPE retval;
    dataLocate->devI2C0 = devI2C0;  
	dataLocate->dataLCD  = dataLCD;

	if ((retval = xTaskCreate(updateTaskLocate, taskNameLocate, I2C_STACK_SIZE, (void*)dataLocate, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
		VT_HANDLE_FATAL_ERROR(retval);
}

portBASE_TYPE sendTimerMsgLocate(structLocate* dataLocate, portTickType ticksElapsed, portTickType ticksToBlock)
{
	if (dataLocate == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	
	msgLocate bufferLocate;
	bufferLocate.length = sizeof(ticksElapsed);

	if (bufferLocate.length > maxLenLocate)
		VT_HANDLE_FATAL_ERROR(bufferLocate.length);

	memcpy(bufferLocate.buf, (char*)&ticksElapsed, sizeof(ticksElapsed));
	bufferLocate.msgType = msgTypeLocateTimer;
	return(xQueueSend(dataLocate->inQ, (void*) (&bufferLocate),ticksToBlock));
}

portBASE_TYPE sendValueMsgLocate(structLocate* dataLocate, uint8_t msgType, uint16_t* value, portTickType ticksToBlock)
{
	msgLocate bufferLocate;

	if (dataLocate == NULL)
		VT_HANDLE_FATAL_ERROR(0);

	bufferLocate.length = sizeof(value);

	if (bufferLocate.length > maxLenLocate)
		VT_HANDLE_FATAL_ERROR(bufferLocate.length);
	
	memcpy(bufferLocate.buf, (char*)&value, sizeof(value));
	bufferLocate.msgType = msgType;
	return(xQueueSend(dataLocate->inQ, (void*)(&bufferLocate), ticksToBlock));
}

static portTASK_FUNCTION(updateTaskLocate, pvParameters)
{
    // Task parameters.
	structLocate* param   = (structLocate*)pvParameters;
    vtI2CStruct*  devI2C0 = param->devI2C0;
	structLCD*    dataLCD = param->dataLCD;
	
	// String buffer for printing.
	char bufferLCD[maxLenLCD + 1];
	
	// Buffer for receiving messages.
	msgLocate msgBuffer;

    // Current states of the rover.
    States curStates;

	// Initial states of the rover.
	StateMotion      initialStateMotion      = stop;
	StatePrimeGoal   initialStatePrimeGoal   = none;
	StateScanSecGoal initialStateScanSecGoal = scanInit;
    StateRoamSecGoal initialStateRoamSecGoal = roamInit;

	curStates.curStateMotion      = initialStateMotion;
    curStates.curStatePrimeGoal   = initialStatePrimeGoal;
	curStates.curStateScanSecGoal = initialStateScanSecGoal;
	curStates.curStateRoamSecGoal = initialStateRoamSecGoal;
			
    // Closest objects on each side of the rover.
    uint16_t curObjs[SIDE_LEN][OBJ_LEN];

    // Ramp at the front of the rover.
    uint16_t curRamp[OBJ_LEN];

    // Current target ramp.
    uint16_t targetRamp[OBJ_LEN];

    // Ack that the ARM PIC sends back.
    bool ack = false;
    
	// Like all good tasks, this should never exit.
	for(;;)
	{
		// Wait for a message from either a timer or sensor task.
		if (xQueueReceive(param->inQ, (void*)&msgBuffer, portMAX_DELAY) != pdTRUE)
			VT_HANDLE_FATAL_ERROR(0);

		// Now, based on the type of the message and the state, do different things.
		switch(getMsgTypeLocate(&msgBuffer))
		{
		case msgTypeLocateTimer:
		{
            // Determine the primary goal of the rover.
            curStates.curStatePrimeGoal = initialStatePrimeGoal;

            switch(curStates.curStatePrimeGoal)
            {
            case scan:
            {
                execScan(devI2C0, &curStates, curObjs, curRamp, targetRamp, &ack);
                break; 
            }
            case roam:
            {
                execRoam(devI2C0, &curStates, curObjs, curRamp, targetRamp, &ack);
                break;
            }
            case go:
            {
                execGo(devI2C0, &curStates, curObjs, curRamp, &ack);
                break;
            }
            case align:
            {
                execAlign(devI2C0, &curStates, curObjs, curRamp, &ack);
                break;
            }
            case ramp:
            {
                execRamp(devI2C0, &curStates, curObjs, curRamp, &ack);               
                break;
            }
			case none:
			{
			   	break;
			}
            default:
            {
                execDefault();
                break;	
            }
	        }
        }
        case msgTypeLocate:
        {
			//locateObjs(curObjs, msgBuffer.buf);
            //locateRamp(curRamp, msgBuffer.buf);
            break;
        }
        case msgTypeCommand:
        {
            // Grab the acknowledgement.
            //ack = (bool)msgBuffer.buf[1];
            break;
        }
        default:
        {
            // Error...
            break;
        }
        }
    }
}

/*------------------------------------------------------------------------------
 * State Functions
 **/

void execScan(vtI2CStruct* devI2C0, States* curStates, uint16_t curObj[SIDE_LEN][OBJ_LEN], uint16_t curRamp[OBJ_LEN], uint16_t targetRamp[OBJ_LEN], bool* ack)
{
    switch(curStates->curStateScanSecGoal)
    {
    case scanInit:
    {
        // Send the command to initialize scanning.
        uint8_t command[CMD_LEN];
        uint8_t type = CMD_TR; 
        uint8_t value = 0;
        uint8_t speed = 25;

        command[CMD_TYPE]  = type;
        command[CMD_VALUE] = value;
        command[CMD_SPEED] = speed;
        
        bool xCommandSuccess = false;
        xCommandSuccess = execCommand(devI2C0, type, value, speed);
        if(xCommandSuccess)
            curStates->curStateScanSecGoal = scanExec;
        break;
    }
    case scanExec:
    {
        // Check to see if a ramp was found.
        if(anyRamps(curRamp))
        {
            targetRamp[OBJ_DIST]  = curRamp[OBJ_DIST];
            targetRamp[OBJ_ANGLE] = curRamp[OBJ_ANGLE];
        }

        // Change the local secondary state if the command was completed.
        if(*ack)
            curStates->curStateScanSecGoal = scanComp;
        break;
    }
    case scanComp:
    {
        // Change the primary state.
        if(targetRamp[OBJ_DIST] > 0)
            curStates->curStatePrimeGoal = go;
        else
            curStates->curStatePrimeGoal = roam;

        // Change the local secondary state.
        curStates->curStateScanSecGoal = scanInit;

        *ack = false;
        break; 
    }
    default:
    {
        // Error...
        break;
    }
    }
}

void execRoam(vtI2CStruct* devI2C0, States* curStates, uint16_t curObjs[SIDE_LEN][OBJ_LEN], uint16_t curRamp[OBJ_LEN], uint16_t targetRamp[OBJ_LEN], bool* ack)
{
    static bool (*roamType)(vtI2CStruct*, uint16_t[SIDE_LEN][OBJ_LEN]);
    static bool roamStop = false;
    switch(curStates->curStateRoamSecGoal)
    {
    case roamInit:
    {
        // Get the closest object to the rover and the side it's on.
        uint8_t side;
        uint16_t closestObject[OBJ_LEN];
        getClosestCurObj(&side, closestObject, curObjs);

        // Determine the roam type.
        uint8_t thresh = 2;
        if(closestObject[OBJ_DIST] < thresh)
            roamType = &execMoveAlong;
        else
            roamType = &execMoveZigzag;

        // Start roaming.
        roamStop = roamType(devI2C0, curObjs);

        // Change local secondary state.
        curStates->curStateRoamSecGoal = roamExec; 
        break;
    }
    case roamExec:
    {
        if(*ack && !roamStop)
        {
            *ack = false;
            roamStop = roamType(devI2C0, curObjs);
        }
        if(*ack && roamStop)
        {
            // Change the local secondary state.
            curStates->curStateRoamSecGoal = roamComp;
        }

        // Check to see if a ramp was found.
        if(anyRamps(curRamp))
        {
            targetRamp[OBJ_DIST]  = curRamp[OBJ_DIST];
            targetRamp[OBJ_ANGLE] = curRamp[OBJ_ANGLE];
            roamStop = true;
        }
        break;
    }
    case roamComp:
    {
        // Change the primary state.
        if(targetRamp[OBJ_DIST] > 0)
            curStates->curStatePrimeGoal = go;
        else
            curStates->curStatePrimeGoal = scan;

        // Change the local secondary state.
        curStates->curStateRoamSecGoal = roamInit;
        break;
    }
    default:
    {
        // Error.
        break;
    }
    }
}

void execGo(vtI2CStruct* devI2C0, States* curStates, uint16_t curObjs[SIDE_LEN][OBJ_LEN], uint16_t curRamp[OBJ_LEN], bool* ack)
{
    switch(curStates->curStateGoSecGoal)
    {
    default:
    {
        // Error...
        break;
    }
    } 
}

void execAlign(vtI2CStruct* devI2C0, States* curStates, uint16_t curObjs[SIDE_LEN][OBJ_LEN], uint16_t curRamp[OBJ_LEN], bool* ack)
{
    switch(curStates->curStateAlignSecGoal)
    {
    default:
    {
        // Error...
        break;
    }
    }
}

void execRamp(vtI2CStruct* devI2C0, States* curStates, uint16_t curObjs[SIDE_LEN][OBJ_LEN], uint16_t curRamp[OBJ_LEN], bool* ack)
{
    switch(curStates->curStateRampSecGoal)
    {
       	default:
		{
		 	// Error...
			break;
		}
    }
}

void execDefault()
{
    // Error:
}

/*------------------------------------------------------------------------------
 * Helper Functions
 **/

bool execMoveAlong(vtI2CStruct* devI2C0, uint16_t curObjs[SIDE_LEN][OBJ_LEN])
{
    bool moveAlongStop = false;

    // Get the closest object.
    uint8_t side;
    uint16_t closestObj[OBJ_LEN];
    getClosestCurObj(&side, closestObj, curObjs);

    // Determine the wall to move along.
    return moveAlongStop; 
}

bool execMoveZigzag(vtI2CStruct* devI2C0, uint16_t curObjs[SIDE_LEN][OBJ_LEN])
{
    bool moveZigzagStop = false;
    return moveZigzagStop;
}

bool execCommand(vtI2CStruct* devI2C0, uint8_t type, uint8_t value, uint8_t speed)
{
    GPIO_SetValue  (0, DEBUG_PIN16);
    GPIO_ClearValue(0, DEBUG_PIN16);

    uint8_t command[CMD_LEN];
    command[CMD_TYPE]  = type;
    command[CMD_VALUE] = value;
    command[CMD_SPEED] = speed;

    // Send an I2C moveBackward command to slave.
    portBASE_TYPE retI2C = vtI2CEnQ(devI2C0, msgTypeCommand, SLAVE_ADDR, sizeof(command), command, 1);
    return (retI2C == pdTRUE);
}

void locateObjs(uint16_t curObjs[SIDE_LEN][OBJ_LEN], uint16_t* locateData)
{
    uint8_t curRoverSide = 0;
	const float horizSensorDistance = 2;

	uint8_t i = 0;
    uint8_t notCareSensors = 3; // Last 3 don't affect current close objects.
	for(i = 0; i < SENS_LEN - notCareSensors - 1; i = i + 2)
	{
		float distanceSensor0 = locateData[i];
		float distanceSensor1 = locateData[i + 1];

		// Calculate the distance to the object.
		curObjs[curRoverSide][OBJ_DIST] = (distanceSensor0 + distanceSensor1)/2;

		// Calculate the angle to the object.
		if(distanceSensor0 > distanceSensor1)
			curObjs[curRoverSide][OBJ_ANGLE] =  90 + atan(abs(distanceSensor0 - distanceSensor1)/horizSensorDistance);	
		else
			curObjs[curRoverSide][OBJ_ANGLE] = atan(horizSensorDistance/(abs(distanceSensor0 - distanceSensor1)));

		// Now do the next rover side.
		if(curRoverSide < SIDE_LEN)
                curRoverSide = curRoverSide + 1;
	}
}

void locateRamp(uint16_t* curRamp, uint16_t* locateData)
{
    uint8_t  thresh = 2;
    uint16_t distanceSensorIR400 = locateData[SENS_IR40];
    uint16_t distanceSENSORIR401 = locateData[SENS_IR41];

    // Is there a ramp in front of us?
    if(distanceSensorIR400 - distanceSENSORIR401 > thresh)
    {
        // Choose the sensor closer to the floor.
        curRamp[OBJ_DIST]  = distanceSensorIR400;
        curRamp[OBJ_ANGLE] = 0;
    }
}

bool anyRamps(uint16_t* curRamp)
{
    // Check the ramp data for a nonzero.
    return curRamp[OBJ_DIST] > 0;  
}

void getClosestCurObj(uint8_t* side, uint16_t closestObj[OBJ_LEN], uint16_t curObjs[SIDE_LEN][OBJ_LEN])
{
    // Determine the closest side containing an object.

}

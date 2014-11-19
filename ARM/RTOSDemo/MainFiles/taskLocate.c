/*------------------------------------------------------------------------------
 * File:	    	taskLocate.c
 * Authors: 		FreeRTOS, Igor Janjic
 * Description:		Implementation file for locate task. Determines location of
 *                  rover and ramps.
 **---------------------------------------------------------------------------*/

#include "taskLocate.h"

static portTASK_FUNCTION_PROTO(updateTaskLocate, pvParameters);

void startTaskLocate(structLocate* dataLocate, unsigned portBASE_TYPE uxPriority, structLCD* dataLCD, structCommand* dataCommand)
{
    // Create the queue that will be used to talk to this task.
    dataLocate->inQ = xQueueCreate(queueLenLocate, sizeof(msgLocate));
    if(dataLocate->inQ == NULL)
    {
        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, errorQueueCreateLocate, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(0);
    }

    // Create the task.
    dataLocate->dataLCD = dataLCD;
    dataLocate->dataCommand = dataCommand;
    
    portBASE_TYPE retval = xTaskCreate(updateTaskLocate, taskNameLocate, LOCATE_STACK_SIZE, (void*)dataLocate, uxPriority, (xTaskHandle*)NULL);
    if(retval != pdPASS)
    {
        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, errorTaskCreateLocate, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(retval);
    }
}

void sendTimerMsgLocate(structLocate* dataLocate, portTickType ticksElapsed, portTickType ticksToBlock)
{
    msgLocate msg;
    msg.length = sizeof(ticksElapsed);

    memcpy(msg.buf, &ticksElapsed, msg.length);
    msg.type = MSG_TYPE_TIMER_LOCATE;

    portBASE_TYPE retval = xQueueSend(dataLocate->inQ, (void*)(&msg), ticksToBlock);
    if(retval != pdTRUE)
    {
        sendValueMsgLCD(dataLocate->dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, errorQueueSendLocate, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(retval);
    }
}

void sendValueMsgLocate(structLocate* dataLocate, uint8_t type, float* value, portTickType ticksToBlock)
{
    msgLocate msg;
    msg.length = sizeof(float)*bufLenLocate;

    memcpy(msg.buf, value, msg.length);
    msg.type = type;
    portBASE_TYPE retval = xQueueSend(dataLocate->inQ, (void*)(&msg), ticksToBlock);
    if(retval != pdTRUE)
    {
        sendValueMsgLCD(dataLocate->dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, errorQueueSendLocate, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(retval);
    }

}

static portTASK_FUNCTION(updateTaskLocate, pvParameters)
{
    // Task parameters.
	structLocate*  param       = (structLocate*)pvParameters;
    structCommand* dataCommand = param->dataCommand;
	structLCD*     dataLCD     = param->dataLCD;
	
	// Buffer for receiving messages.
	msgLocate msg;

    // Initialize map.
    Map map = {
        .map = {
            .allocated = false,
            .rows = MAP_RADIUS_INIT,
            .cols = MAP_RADIUS_INIT
        },
        .radix = {
            .allocated = false,
            .n = 2
        },
        .thresh = {
            .allocated = false,
            .n = 2
        },
        .grow = false
    };
    createMatrix(&(map.map));
    createVector(&(map.radix));
    createVector(&(map.thresh));
    map.radix.buf[0] = (MAP_RADIUS_MAX - 1)/2;
    map.radix.buf[1] = (MAP_RADIUS_MAX - 1)/2;
    map.thresh.buf[0] = 0.0;
    map.thresh.buf[1] = 0.0;

    // Initialize rover.
    Rover rover = {
        .curStates = {
            .curStateMotion       = initialStateMotion,
            .curStatePrimeGoal    = initialStatePrimeGoal,
            .curStateScanSecGoal  = initialStateScanSecGoal,
            .curStateRoamSecGoal  = initialStateRoamSecGoal,
            .curStateGoSecGoal    = initialStateGoSecGoal,
            .curStateAlignSecGoal = initialStateAlignSecGoal,
            .curStateRampSecGoal  = initialStateRampSecGoal
        },
        .curObstacles = {
            .allocated = false,
            .rows = 2,
            .cols = 4
        },
        .curCorners = {
            .allocated = false,
            .rows = 2,
            .cols = 4
        },
        .curRamps = {
            .allocated = false,
            .rows = 2,
            .cols = 3
        },
        .curTarget = {
            .allocated = false,
            .n = 4
        },
        .orient = 0.0,
        .mapped = false,
        .ack    = false
    };
    createMatrix(&(rover.curObstacles));
    createMatrix(&(rover.curCorners));
    createMatrix(&(rover.curRamps));
    createVector(&(rover.curTarget));

    // Like all good tasks, this should never exit.
	for(;;)
	{
        // Wait for a message from the queue.
        portBASE_TYPE retQueue = xQueueReceive(param->inQ, (void*)&msg, portMAX_DELAY);
        if (retQueue != pdTRUE)
        {
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, errorQueueReceiveLocate, portMAX_DELAY);
            VT_HANDLE_FATAL_ERROR(retQueue);
        }

	    // Now, based on the type of the message and the state, do different things.
		switch(msg.type)
		{
		case MSG_TYPE_TIMER_LOCATE:
		{
            switch(rover.curStates.curStatePrimeGoal)
            {
            case scan:
            {
                //execScan(devI2C0);
                break; 
            }
            case roam:
            {
                //execRoam(devI2C0);
                break;
            }
            case go:
            {
                //execGo(devI2C0);
                break;
            }
            case align:
            {
                //execAlign(devI2C0);
                break;
            }
            case ramp:
            {
                //execRamp(devI2C0);               
                break;
            }
			case none:
			{
			   	break;
			}
            default:
            {
                //execDefault();
                break;	
            }
	        }
			break;
        }
        case MSG_TYPE_LOCATE:
        {
            float data[8];
            uint8_t i;
            for(i = 0; i < 8; i++)
                data[i] = msg.buf[i + 2];

            updateData(&rover, &map, data);

            // Map updated data to global coordinates, allocating space if
            // need be.   
            mapData(&rover, &map);
             
            break;
        }
        case MSG_TYPE_ACK:
        {
            // Grab the acknowledgement from the motor controller.
            rover.ack = (bool)msg.buf[0];
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

void updateData(Rover* rover, Map* map, float* data)
{
    // Update the current obstacles.
    float B[4];
    uint8_t i;
    for(i = 0; i < 7; i = i + 2)
        B[i] = (data[i] + data[i + 1])/2;

    float IR1 = B[0]; 
    rover->curObstacles.rowVectors[0].buf[0] = 0;
    rover->curObstacles.rowVectors[1].buf[0] = IR1;

    float IR2 = B[1]; 
    rover->curObstacles.rowVectors[0].buf[1] = -1*IR2;
    rover->curObstacles.rowVectors[1].buf[1] = 0;

    float IR3 = B[2]; 
    rover->curObstacles.rowVectors[0].buf[2] = 0;
    rover->curObstacles.rowVectors[1].buf[2] = -1*IR3;

    float IR4 = B[3]; 
    rover->curObstacles.rowVectors[0].buf[3] = IR4;
    rover->curObstacles.rowVectors[1].buf[3] = 0;

    // Update the current ramps.
}

void mapData(Rover* rover, Map* map)
{
    // Rotates the data to the normal coordinate system.
    Matrix T = {
        .allocated = false,
        .rows = 2,
        .cols = 2
    };
    Matrix result = {
        .allocated = false,
        .rows = 2,
        .cols = 4
    };
    createMatrix(&T);
    createMatrix(&result);
    T.rowVectors[0].buf[0] = cos(rover->orient);
    T.rowVectors[0].buf[1] = sin(rover->orient)*(-1);
    T.rowVectors[1].buf[0] = sin(rover->orient);
    T.rowVectors[1].buf[1] = cos(rover->orient);

    // Map front sensors.
    uint16_t direction = 0;
	uint16_t i;
    for(i = 0; i < 4; i++)
    {
        multiplyMatrix2Vector(&(result.rowVectors[direction]), &T, &(rover->curObstacles.rowVectors[direction])); 
        if(result.rowVectors[i].buf[0] < SENS_DIST_CUTOFF && result.rowVectors[i].buf[1] < SENS_DIST_CUTOFF)
        {
            int r = (int)(round(result.rowVectors[i].buf[1]));    
            int c = (int)(round(result.rowVectors[i].buf[0])); 

            // Convert to global coordinates.
            int rp = r + map->radix.buf[0];
            int cp = c + map->radix.buf[1];

            // See if the new point fits on the map, otherwise reallocate.
            if(rp < 0) 
            {
                recreateMatrix(&(map->map), SIDE_BACK);
                rp = rp + 2*(map->radix.buf[0]);
                cp = cp;
                map->radix.buf[0] = 3*(map->radix.buf[0]);
                map->radix.buf[1] = map->radix.buf[1]; 
            }
            if(rp > map->map.rows)
            {
                recreateMatrix(&(map->map), SIDE_FRONT);
                rp = rp;
                cp = cp;
                map->radix.buf[0] = map->radix.buf[0];
                map->radix.buf[1] = map->radix.buf[1];
            }
            if(cp < 0)
            {
                recreateMatrix(&(map->map), SIDE_LEFT);
                rp = rp;
                cp = cp + 2*(map->radix.buf[0]);
                map->radix.buf[0] = map->radix.buf[0];
                map->radix.buf[1] = 3*(map->radix.buf[1]);

            }
            if(cp > map->map.cols)
            {
                recreateMatrix(&(map->map), SIDE_RIGHT);
                map->radix.buf[0] = map->radix.buf[0];
                map->radix.buf[1] = map->radix.buf[1];
            }
        }
    }

    // Fit the new data to the map.
    for(i = 0; i < 4; i++)
    {
        uint16_t r = (uint16_t)(round(result.rowVectors[0].buf[i]));
        uint16_t c = (uint16_t)(round(result.rowVectors[1].buf[i]));
        map->map.rowVectors[r].buf[c] = 1.0;
    }
}

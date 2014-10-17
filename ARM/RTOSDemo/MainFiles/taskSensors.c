/*------------------------------------------------------------------------------
 * File:        taskSensors.c
 * Authors:         FreeRTOS, Igor Janjic
 * Description:     Reads the sensor data, processes it, and sends it to the
 *          relevant tasks.
 *----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * Includes
 */
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
#include "vtI2C.h"
#include "taskLCD.h"
#include "taskSensors.h"
#include "taskLocate.h"
#include "I2CTaskMsgTypes.h"
#include "debug.h"

/*------------------------------------------------------------------------------
 * Configuration
 **/

// Length of the queue to this task.
#define queueLenSensors 10

// Stack sizes.
#define BASE_STACK 3
#if PRINTF_VERSION == 1
#define I2C_STACK_SIZE      ((BASE_STACK+5)*configMINIMAL_STACK_SIZE)
#else
#define I2C_STACK_SIZE      (BASE_STACK*configMINIMAL_STACK_SIZE)
#endif

// Actual data structure that is sent in a message.
typedef struct __msgSensors {
    uint8_t msgType;
    uint8_t length;
    uint16_t buf[maxLenSensors + 1];
} msgSensors;

// I2C commands for reading sensor data.
const uint8_t recvNoReply = 0x00;
const uint8_t queryReadSensors[] = {0x0A};

static portTASK_FUNCTION_PROTO(updateTaskSensors, pvParameters);

int getMsgTypeSensors(msgSensors *Buffer) {return(Buffer->msgType);}

void startTaskSensors(structSensors* dataSensors, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0, structLCD* dataLCD)
{
    // Create the queue that will be used to talk to this task.
    if ((dataSensors->inQ = xQueueCreate(queueLenSensors, sizeof(msgSensors))) == NULL)
        VT_HANDLE_FATAL_ERROR(0);

    // Create the task.
    portBASE_TYPE retval;
    dataSensors->devI2C0 = devI2C0;
    dataSensors->dataLCD = dataLCD;
    
    if ((retval = xTaskCreate(updateTaskSensors, taskNameSensors, I2C_STACK_SIZE, (void*)dataSensors, uxPriority, (xTaskHandle*)NULL)) != pdPASS)
        VT_HANDLE_FATAL_ERROR(retval);
}

portBASE_TYPE sendTimerMsgSensors(structSensors* dataSensors, portTickType ticksElapsed, portTickType ticksToBlock)
{
    if (dataSensors == NULL)
        VT_HANDLE_FATAL_ERROR(0);
    
    msgSensors bufferSensors;
    bufferSensors.length = sizeof(ticksElapsed);

    if (bufferSensors.length > maxLenSensors)
        VT_HANDLE_FATAL_ERROR(bufferSensors.length);

    memcpy(bufferSensors.buf, (char*)&ticksElapsed, sizeof(ticksElapsed));
    bufferSensors.msgType = msgTypeSensorsTimer;
    return(xQueueSend(dataSensors->inQ, (void*) (&bufferSensors),ticksToBlock));
}

portBASE_TYPE sendValueMsgSensors(structSensors* dataSensors, uint8_t msgType, uint8_t* value, portTickType ticksToBlock)
{
    msgSensors bufferSensors;

    if (dataSensors == NULL)
        VT_HANDLE_FATAL_ERROR(0);

    bufferSensors.length = sizeof(value);

    if (bufferSensors.length > maxLenSensors)
        VT_HANDLE_FATAL_ERROR(bufferSensors.length);
    
    memcpy(bufferSensors.buf, (char*)&value, sizeof(value));
    bufferSensors.msgType = msgType;
    return(xQueueSend(dataSensors->inQ, (void*)(&bufferSensors), ticksToBlock));
}

static portTASK_FUNCTION(updateTaskSensors, pvParameters)
{
    // Get pointers to sensor message fields.
    structSensors* param = (structSensors*)pvParameters;
    vtI2CStruct* devI2C0 = param->devI2C0;
    structLCD* dataLCD = param->dataLCD;
    structLocate* dataLocate = param->dataLocate;
    
    // String buffer for printing.
    char bufferLCD[maxLenLCD + 1];
    
    // Buffer for receiving messages.
    msgSensors msgBuffer;

    // Like all good tasks, this should never exit.
    for(;;)
    {
        // Wait for a message from the queue.
        if (xQueueReceive(param->inQ, (void*)&msgBuffer, portMAX_DELAY) != pdTRUE)
            VT_HANDLE_FATAL_ERROR(0);
        
        // Now, based on the type of the message and the state, we decide on the new state and action to take.
        switch(getMsgTypeSensors(&msgBuffer))
        {
        case msgTypeSensorsTimer:
        {
            // Debug Pin15
            GPIO_SetValue  (0, DEBUG_PIN15);
            GPIO_ClearValue(0, DEBUG_PIN15);
            // Query for all sensor data.
			portBASE_TYPE retI2C = vtI2CEnQ(devI2C0, msgTypeSensors, SLAVE_ADDR, sizeof(queryReadSensors), queryReadSensors, 22);
            if(retI2C != pdTRUE)
           		VT_HANDLE_FATAL_ERROR(0);
			break;
        }
        case msgTypeSensors:
        {
            // Convert the IR voltage to a distance.
            uint16_t processedSensorsData[SENS_LEN] = {0};
            
            // Send the processed sensor data to the locate task.
            //sendValueMsgLocate(dataLocate, msgSensors, processedSensorsData, portMAX_DELAY);
            break;
        }
        default:
        {
            // error
            break;
        }
        }
    }
}

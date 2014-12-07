/*------------------------------------------------------------------------------
 * File:            taskSensors.c
 * Authors:         FreeRTOS, Igor Janjic
 * Description:     Reads the sensor data, processes it, and sends it to the
 *                  relevant tasks.
 *----------------------------------------------------------------------------*/

#include "taskSensors.h"

static portTASK_FUNCTION_PROTO(updateTaskSensors, pvParameters);

void startTaskSensors(structSensors* dataSensors, unsigned portBASE_TYPE uxPriority, vtI2CStruct* devI2C0, structLCD* dataLCD, structLocate* dataLocate)
{
    char eventsMsg[QUEUE_BUF_LEN_LCD];

    // Create the queue that will be used to talk to this task.
    dataSensors->inQ = xQueueCreate(QUEUE_LEN_SENS, sizeof(msgSensors));
    if(dataSensors->inQ == NULL)
    {
        sprintf(eventsMsg, "Error: failed creating queue for task %s", taskNameSensors);
        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, errorQueueCreateSensors, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(0);
    }

    // Create the task.
    dataSensors->devI2C0    = devI2C0;
    dataSensors->dataLCD    = dataLCD;
    dataSensors->dataLocate = dataLocate;
    
    portBASE_TYPE retval = xTaskCreate(updateTaskSensors, taskNameSensors, SENSORS_STACK_SIZE, (void*)dataSensors, uxPriority, (xTaskHandle*)NULL);
    if(retval != pdPASS)
    {
        sprintf(eventsMsg, "Error: failed creating task %s", taskNameSensors);
        sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, errorTaskCreateSensors, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(retval);
    }
}

void sendTimerMsgSensors(structSensors* dataSensors, portTickType ticksElapsed, portTickType ticksToBlock)
{
    char eventsMsg[QUEUE_BUF_LEN_LCD];

    msgSensors msg;
    msg.length = sizeof(ticksElapsed);

    memcpy(msg.buf, &ticksElapsed, msg.length);
    msg.type = MSG_TYPE_TIMER_SENSORS;

    portBASE_TYPE retval = xQueueSend(dataSensors->inQ, (void*)(&msg), ticksToBlock);
    if(retval != pdTRUE)
    {
        sprintf(eventsMsg, "Error: failed sending to timer queue for task %s", taskNameSensors);
        sendValueMsgLCD(dataSensors->dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(retval);
    }
}

void sendValueMsgSensors(structSensors* dataSensors, uint8_t type, uint8_t* value, portTickType ticksToBlock)
{
    char eventsMsg[QUEUE_BUF_LEN_LCD];

    msgSensors msg;
    msg.length = sizeof(uint8_t)*QUEUE_BUF_LEN_SENS;

    memcpy(msg.buf, value, msg.length);
    msg.type = type;
    portBASE_TYPE retval = xQueueSend(dataSensors->inQ, (void*)(&msg), ticksToBlock);
    if(retval != pdTRUE)
    {
        sprintf(eventsMsg, "Error: failed sending to value queue for task %s", taskNameSensors);
        sendValueMsgLCD(dataSensors->dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, errorQueueSendSensors, portMAX_DELAY);
        VT_HANDLE_FATAL_ERROR(retval);
    }
}

static portTASK_FUNCTION(updateTaskSensors, pvParameters)
{
    // Get pointers to parameter fields.
    structSensors* param      = (structSensors*)pvParameters;
    vtI2CStruct*   devI2C0    = param->devI2C0;
    structLCD*     dataLCD    = param->dataLCD;
    structLocate*  dataLocate = param->dataLocate;
    
    // Buffer for receiving messages.
    msgSensors msg;

    char eventsMsg[QUEUE_BUF_LEN_LCD];
    char infoMsg[QUEUE_BUF_LEN_LCD];

    // Like all good tasks, this should never exit.
    for(;;)
    {
        toggleLED(PIN_LED_2);

        // Wait for a message from the queue.
        portBASE_TYPE retQueue = xQueueReceive(param->inQ, (void*)&msg, portMAX_DELAY);
        if (retQueue != pdTRUE)
        {
            sprintf(eventsMsg, "Error: failed receiving from queue for task %s", taskNameSensors);
            sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
            VT_HANDLE_FATAL_ERROR(retQueue);
        }
        
        // Now, based on the type of the message and the state, we decide on the new state and action to take.
        switch(msg.type)
        {
        case MSG_TYPE_TIMER_SENSORS:
        {
            sprintf(eventsMsg, "Sending query for sensor data");
			sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);

			uint8_t query = 0x0A;
            portBASE_TYPE retI2C = vtI2CEnQ(devI2C0, MSG_TYPE_I2C_SENSORS, SLAVE_ADDR, sizeof(query), &query, QUEUE_BUF_LEN_SENS);
            if(retI2C != pdTRUE)
            {
                sprintf(eventsMsg, "Error: unable to communicate over I2C");
                sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
                VT_HANDLE_FATAL_ERROR(retI2C);
            }
            break;
        }
        case MSG_TYPE_SENSORS:
        {
            uint16_t sensorsRaw[SENS_LEN];
            uint16_t sensorsProcessedI[SENS_LEN];
            float    sensorsProcessedF[SENS_LEN];

            sprintf(eventsMsg, "Processing sensor data");
			sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
                
            // Combine the two bytes of sensor data into a single value.
			uint8_t i;
            for(i = 0; i < QUEUE_BUF_LEN_SENS - 2; i = i + 2)
            {
                int j = 0;
                uint8_t byte0 = msg.buf[i];
                uint8_t byte1 = msg.buf[i + 1];
                uint16_t curSensorRaw = byte1 << 8;
                curSensorRaw = curSensorRaw | byte0;
                sensorsRaw[j] = curSensorRaw;
                j++;
            }

            // Convert the IR sensor data into distances.
            for(i = 0; i < SENS_LEN - 1; i++)
            {
                sensorsProcessedF[i] = 47.25530465*sensorsRaw[i]*sensorsRaw[i] - 193.4144014*sensorsRaw[i] + 216.5086058;
                uint8_t intPart = 10*(uint8_t)sensorsProcessedF[i];
                uint8_t decPart = (uint8_t)(sensorsProcessedF[i]*10);
                sensorsProcessedI[i] = (uint16_t)(intPart + decPart);
            }

            // Convert the accelerometer sensor data into a meaningful value.
            sensorsProcessedI[10] = sensorsRaw[10]; // Fix this later.
            sensorsProcessedF[10] = sensorsRaw[10];

            for (i = 0 ; i < QUEUE_BUF_LEN_SENS ; i++)
                infoMsg[i] = msg.buf[i] + '0';
            infoMsg[QUEUE_BUF_LEN_SENS] = '\0';
			sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_SENS_DATA, QUEUE_BUF_LEN_LCD, infoMsg, portMAX_DELAY);

            // Send the processed sensor data to the locate task.
           	//sendValueMsgLocate(dataLocate, MSG_TYPE_LOCATE, sensorsProcessedF, portMAX_DELAY);
            
            break;
        }
        default:
        {
            VT_HANDLE_FATAL_ERROR(0);
            break;
        }
        }
    }
}

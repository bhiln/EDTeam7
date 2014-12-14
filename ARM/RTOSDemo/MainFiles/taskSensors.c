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
            portBASE_TYPE retI2C = vtI2CEnQ(devI2C0, MSG_TYPE_I2C_SENSORS, SLAVE_ADDR, sizeof(query), &query, QUEUE_BUF_LEN_SENS + 2);
            if(retI2C != pdTRUE)
            {
                sprintf(eventsMsg, "Error: unable to communicate over I2C");
                sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD + 1, eventsMsg, portMAX_DELAY);
                VT_HANDLE_FATAL_ERROR(retI2C);
            }
            break;
        }
        case MSG_TYPE_SENSORS:
        {
            uint16_t sensorsRaw[SENS_LEN] = {0};
            double   sensorsProcessedF[SENS_LEN] = {0};

            sprintf(eventsMsg, "Processing sensor data");
			sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_EVENTS, QUEUE_BUF_LEN_LCD, eventsMsg, portMAX_DELAY);
                            
            // Combine the two bytes of sensor data into a single value.
			uint8_t i;
            int j = 0;
            for(i = 0; i < QUEUE_BUF_LEN_SENS - 2; i = i + 2)
            {
                uint8_t byte0 = msg.buf[i];
                uint8_t byte1 = msg.buf[i + 1];
                uint16_t curSensorRaw = byte0 | (byte1 << 8);
                sensorsRaw[j] = curSensorRaw;
                j++;
            }

            double first = pow(5.5085320959466787, 2);
            double second = (-1)*pow(1.0261454415387730, 3);
            double third = pow(7.1555982778098962, 2);
            double fourth = (-1)*pow(2.1128075555690111, 2);
            double fifth = pow(2.2264016814490361, 1);

            // Convert the IR sensor data into distances.
            for(i = 0; i < SENS_LEN - 1; i++)
            {
                double analogVolt = ((double)sensorsRaw[i])/310;
                sensorsProcessedF[i] = first  * pow(analogVolt, 0)
                                     + second * pow(analogVolt, 1)
                                     + third  * pow(analogVolt, 2)
                                     + fourth * pow(analogVolt, 3)
                                     + fifth  * pow(analogVolt, 4);

                //if(sensorsProcessedF[i] > 50)
                    //sensorsProcessedF[i] = 50;
            }

            // Convert the accelerometer sensor data into a meaningful value.
            sensorsProcessedF[10] = sensorsRaw[10];

            // Send the sensor data to the info tab on the LCD.
            infoMsg[0] = '\0';
            for (i = 0; i < 12; i = i + 2)
            {
                uint8_t byte0 = msg.buf[i + 1];
                uint8_t byte1 = msg.buf[i];
                char res[10];
                sprintf(&res[0], "0x%02x%02x ", byte0, byte1);
                strcat(infoMsg, res);
            }
			sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_TAB_SENS_0, QUEUE_BUF_LEN_LCD, infoMsg, portMAX_DELAY);
            infoMsg[0] = '\0';
            for (i = 12; i < QUEUE_BUF_LEN_SENS - 1; i = i + 2)
            {
                uint8_t byte0 = msg.buf[i + 1];
                uint8_t byte1 = msg.buf[i];
                char res[10];
                sprintf(&res[0], "0x%02x%02x ", byte0, byte1);
                strcat(infoMsg, res);
            }
            //sprintf(infoMsg, "%d : %d", (uint16_t)sensorsProcessedF[2], (uint16_t)sensorsProcessedF[3]);
			sendValueMsgLCD(dataLCD, MSG_TYPE_LCD_TAB_SENS_1, QUEUE_BUF_LEN_LCD, infoMsg, portMAX_DELAY);

            // Send the processed sensor data to the locate task.
           	sendValueMsgLocate(dataLocate, MSG_TYPE_LOCATE, (float*)sensorsProcessedF, portMAX_DELAY);
            
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

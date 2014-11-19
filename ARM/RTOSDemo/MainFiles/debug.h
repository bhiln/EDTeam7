#ifndef DEBUG_H
#define DEBUG_H

/*------------------------------------------------------------------------------
 * Includes
 **/

#include "lpc17xx_gpio.h"

#include "defs.h"

/*------------------------------------------------------------------------------
 * Definitions
 **/

#define DEBUG_PORT          0
#define DEBUG_PIN15 		0x00008000
#define DEBUG_PIN16 		0x00010000
#define DEBUG_PIN17 		0x00020000
#define DEBUG_PIN18 		0x00040000

/*------------------------------------------------------------------------------
 * Global Variables
 **/

// Debug information.
char debugInitSystem[]         = "Initializing system";
char debugInitLEDs[]           = "Initializing LEDs";
char debugInitHardware[]       = "Initializing hardware";
char debugStartTaskLCD[]       = "Starting task LCD";
char debugStartTaskI2C[]       = "Starting task I2C";
char debugStartTaskSensors[]   = "Starting task SENORS";
char debugStartTaskLocate[]    = "Starting task LOCATE";
char debugStartTaskCommand[]   = "Starting task COMMAND";
char debugStartTaskConductor[] = "Starting task CONDUCTOR";
char debugInitScheduler[]      = "Initializing SCHEDULER";
char debugStartTimerLCD[]      = "Starting timer LCD";
char debugStartTimerSensors[]  = "Starting timer SENSORS";
char debugStartTimerLocate[]   = "Starting timer LOCATE";
char debugStartTimerCommand[]  = "Starting timer COMMAND";
char debugRcvdSensorData[]     = "Received new sensor data from slave";
char debugNoRcvdSensorData[]   = "No new sensor data from slave";
char debugRcvdAck[]            = "Received command acki from slave";
char debugNoRcvdAck[]          = "No command ack from slave";

// Error information.
char errorTaskCreateI2C[]         = "Error: task create I2C";
char errorTaskCreateConductor[]   = "Error: task create CONDUCTOR";
char errorTaskCreateSensors[]     = "Error: task create SENSORS";
char errorTaskCreateLocate[]      = "Error: task create LOCATE";
char errorTaskCreateCommand[]     = "Error: task create COMMAND";
char errorQueueCreateSensors[]    = "Error: queue create for task SENSORS";
char errorQueueCreateLocate[]     = "Error: queue create for task LOCATE";
char errorQueueCreateCommand[]    = "Error: queue create for task COMMAND";
char errorQueueSendSensors[]      = "Error: queue send for task SENSORS";
char errorQueueSendLocate[]       = "Error: queue send for task LOCATE";
char errorQueueSendCommand[]      = "Error: queue send for task COMMAND";
char errorQueueReceiveConductor[] = "Error: queue rcv for task CONDUCTOR";
char errorQueueReceiveSensors[]   = "Error: queue rcv for task SENSORS";
char errorQueueReceiveLocate[]    = "Error: queue rcv for task LOCATE";
char errorQueueReceiveCommand[]   = "Error: queue rcv for task COMMAND";
char errorI2CDequeConductor[]     = "Error: I2C deque for task CONDUCTOR";
char errorI2CEnqueSensors[]       = "Error: I2C enque for task SENSORS";
char errorI2C[]                   = "Error: bad I2C connection";

/*------------------------------------------------------------------------------
 * Functions
 **/

// Simple function for writing values to the GPIO pins.
bool writeDebug(uint32_t pin, uint8_t value)
{
    bool succ = false;
    if((pin == DEBUG_PIN15) || (pin == DEBUG_PIN16) || (pin == DEBUG_PIN17) || (pin == DEBUG_PIN18))
    {
        uint8_t i;
        for(i = 0; i < value; i++)
        {
            GPIO_SetValue  (0, pin);
            GPIO_ClearValue(0, pin);
        }
        succ = true;
    }
    return succ;
}

#endif

#ifndef DEBUG_H
#define DEBUG_H

/*------------------------------------------------------------------------------
 * Includes
 **/

#include "lpc17xx_gpio.h"
#include "defs.h"

// Debug information.
char debugStartTimerLCD[]         = "Starting timer LCD";
char debugStartTimerSensors[]     = "Starting timer SENSORS";
char debugStartTimerLocate[]      = "Starting timer LOCATE";
char debugStartTimerCommand[]     = "Starting timer COMMAND";
char debugSensorsQuery[]          = "Sending sensors query";
char debugCommand[]               = "Sending command";

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
    if((pin == PIN_DEBUG_0) || (pin == PIN_DEBUG_1) || (pin == PIN_DEBUG_2) || (pin == PIN_DEBUG_3))
    {
        uint8_t i;
        for(i = 0; i < value; i++)
        {
            GPIO_SetValue  (PORT_DEBUG, pin);
            GPIO_ClearValue(PORT_DEBUG, pin);
        }
        succ = true;
    }
    return succ;
}

void toggleLED(uint32_t pinLED)
{
    static bool stateLED[8] = {false};

    switch(pinLED)
    {
    case PIN_LED_0:
    {
        if(stateLED[0])
            GPIO_ClearValue(PORT_LED_0, pinLED);
        else
            GPIO_SetValue(PORT_LED_0, pinLED);
        stateLED[0] = !stateLED[0];
        break;
    }
    case PIN_LED_1:
    {
        if(stateLED[1])
            GPIO_ClearValue(PORT_LED_1, pinLED);
        else
            GPIO_SetValue(PORT_LED_1, pinLED);
        stateLED[1] = !stateLED[1];
        break;
    }
    case PIN_LED_2:
    {
        if(stateLED[2])
            GPIO_ClearValue(PORT_LED_2, pinLED);
        else
            GPIO_SetValue(PORT_LED_2, pinLED);
        stateLED[2] = !stateLED[2];
        break;
    }
    case PIN_LED_3:
    {
        if(stateLED[3])
            GPIO_ClearValue(PORT_LED_3, pinLED);
        else
            GPIO_SetValue(PORT_LED_3, pinLED);
        stateLED[3] = !stateLED[3];
        break;
    }
    case PIN_LED_4:
    {
        if(stateLED[4])
            GPIO_ClearValue(PORT_LED_4, pinLED);
        else
            GPIO_SetValue(PORT_LED_4, pinLED);
        stateLED[4] = !stateLED[4];
        break;
    }
    case PIN_LED_5:
    {
        if(stateLED[5])
            GPIO_ClearValue(PORT_LED_5, pinLED);
        else
            GPIO_SetValue(PORT_LED_5, pinLED);
        stateLED[5] = !stateLED[5];
        break;
    }
    case PIN_LED_6:
    {
        if(stateLED[6])
            GPIO_ClearValue(PORT_LED_6, pinLED);
        else
            GPIO_SetValue(PORT_LED_6, pinLED);
        stateLED[6] = !stateLED[6];
        break;
    }
    case PIN_LED_7:
    {
        if(stateLED[7])
            GPIO_ClearValue(PORT_LED_7, pinLED);
        else
            GPIO_SetValue(PORT_LED_7, pinLED);
        stateLED[7] = !stateLED[7];
        break;
    }
    default:
    {
	    // Error...
    }
    }
}

#endif

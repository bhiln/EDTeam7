/*------------------------------------------------------------------------------
 * File:		main.c
 * Authors: 	FreeRTOS, Igor Janjic
 * Description:	Application main entry point.
 *----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * Includes
 **/

#include <stdio.h>

#include "FreeRTOS.h"
#include "system_LPC17xx.h"
#include "syscalls.h"
#include "task.h"

#include "partest.h"
#include "vtI2C.h"
#include "vtUtilities.h"

#include "debug.h"
#include "defs.h"
#include "taskCommand.h"
#include "taskConductor.h"
#include "taskLCD.h"
#include "taskLocate.h"
#include "taskSensors.h"
#include "taskTimers.h"

/*------------------------------------------------------------------------------
 * Definitions
 **/

// The rate at which I2C runs.
#define I2C_BITRATE                 100000

// The time between cycles of the 'check' hook.
#define CHECK_DELAY			        ((portTickType)5000/portTICK_RATE_MS)

// Task priorities.
#define COMMAND_TASK_PRIORITY       (tskIDLE_PRIORITY)
#define CONDUCTOR_TASK_PRIORITY		(tskIDLE_PRIORITY)
#define LCD_TASK_PRIORITY		    (tskIDLE_PRIORITY)
#define LOCATE_TASK_PRIORITY		(tskIDLE_PRIORITY)
#define MONITOR_TASK_PRIORITY		(tskIDLE_PRIORITY)
#define SENSOR_TASK_PRIORITY		(tskIDLE_PRIORITY)

/*------------------------------------------------------------------------------
 * Task Data Structures
 **/

static vtI2CStruct     vtI2C0;
static structLCD       dataLCD;
static structCommand   dataCommand;
static structLocate    dataLocate;
static structSensors   dataSensors;
static structConductor dataConductor;

/*------------------------------------------------------------------------------
 * Application Main Entry Point
 **/

// Configure the hardware.
static void setupHardware(void);

int main()
{
	// Initialize system.
	init_syscalls();

	// Set up the LED ports.
	vtInitLED();

	// Set up hardware.
	setupHardware();

	// LCD task.
	startTaskLCD(&dataLCD, LCD_TASK_PRIORITY);
	startTimerLCD(&dataLCD);

    // Debug - initialize base.
    sendValueMsgLCD(&dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugInitSystem, portMAX_DELAY);
    sendValueMsgLCD(&dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugInitLEDs, portMAX_DELAY);
    sendValueMsgLCD(&dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugInitHardware, portMAX_DELAY);
    sendValueMsgLCD(&dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugStartTaskLCD, portMAX_DELAY);
    sendValueMsgLCD(&dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugStartTimerLCD, portMAX_DELAY);
	
	// I2C task.
    sendValueMsgLCD(&dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugStartTaskI2C, portMAX_DELAY);
	if(vtI2CInit(&vtI2C0, 0, MONITOR_TASK_PRIORITY, I2C_BITRATE) != vtI2CInitSuccess)
    {
        sendValueMsgLCD(&dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, errorTaskCreateI2C, portMAX_DELAY);
		VT_HANDLE_FATAL_ERROR(0);
    }
        
    // Command task.
    sendValueMsgLCD(&dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugStartTaskCommand, portMAX_DELAY);
	startTaskCommand(&dataCommand, COMMAND_TASK_PRIORITY, &vtI2C0, &dataLCD);
    sendValueMsgLCD(&dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugStartTimerCommand, portMAX_DELAY);
	startTimerCommand(&dataCommand);

    // Locate task.
    sendValueMsgLCD(&dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugStartTaskLocate, portMAX_DELAY);
	startTaskLocate(&dataLocate, SENSOR_TASK_PRIORITY, &dataLCD, &dataCommand);
    sendValueMsgLCD(&dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugStartTimerLocate, portMAX_DELAY);
	startTimerLocate(&dataLocate);

	// Sensors task.
    sendValueMsgLCD(&dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugStartTaskSensors, portMAX_DELAY);
	startTaskSensors(&dataSensors, SENSOR_TASK_PRIORITY, &vtI2C0, &dataLCD, &dataLocate);
    sendValueMsgLCD(&dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugStartTimerSensors, portMAX_DELAY);
	startTimerSensors(&dataSensors);

    // Conductor task.
    sendValueMsgLCD(&dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugStartTaskConductor, portMAX_DELAY);
	startTaskConductor(&dataConductor, CONDUCTOR_TASK_PRIORITY, &vtI2C0, &dataLCD, &dataSensors, &dataLocate);

	// Schedlar task.
    sendValueMsgLCD(&dataLCD, MSG_TYPE_LCD_DEBUG, maxLenLCD, debugInitScheduler, portMAX_DELAY);
	vTaskStartScheduler();
       // Shouldn't reach this part unless insufficient memory.
	for(;;);
}

void vApplicationTickHook(void)
{
	static unsigned long ulTicksSinceLastDisplay = 0;

	// Have enough ticks passed to make it	time to perform our health status
	// check again?
	ulTicksSinceLastDisplay++;
	if(ulTicksSinceLastDisplay >= CHECK_DELAY)
	{
		// Reset counter.
		ulTicksSinceLastDisplay = 0;
	}
}

void setupHardware(void)
{
	// Disable peripherals power.
	SC->PCONP = 0;

	// Enable GPIO power.
	SC->PCONP = PCONP_PCGPIO;

	// Disable TPIU.
	PINCON->PINSEL10 = 0;

	//  Setup the peripheral bus to be the same as the PLL output (64 MHz).
	SC->PCLKSEL0 = 0x05555555;

	// Configure the LEDs.
	vParTestInitialise();

	// Enable debug pins.
	GPIO_SetDir(DEBUG_PORT, DEBUG_PIN15 | DEBUG_PIN16 | DEBUG_PIN17 | DEBUG_PIN18, 1);

    // Enable joystick.
    GPIO_SetDir(JOYSTICK_PORT, JOYSTICK_LEFT | JOYSTICK_RIGHT | JOYSTICK_UP | JOYSTICK_DOWN | JOYSTICK_SELECT, 0);
}

// This function will get called if a task overflows its stack.
void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName)
{
	(void) pxTask;
	(void) pcTaskName;
	VT_HANDLE_FATAL_ERROR(0);
	for(;;);
}

void vConfigureTimerForRunTimeStats(void)
{
	const unsigned long TCR_COUNT_RESET = 2, CTCR_CTM_TIMER = 0x00, TCR_COUNT_ENABLE = 0x01;

	/* This function configures a timer that is used as the time base when
	collecting run time statistical information - basically the percentage
	of CPU time that each task is utilising.  It is called automatically when
	the scheduler is started (assuming configGENERATE_RUN_TIME_STATS is set
	to 1). */

	/* Power up and feed the timer. */
	SC->PCONP |= 0x02UL;
	SC->PCLKSEL0 = (SC->PCLKSEL0 & (~(0x3<<2))) | (0x01 << 2);

	/* Reset Timer 0 */
	TIM0->TCR = TCR_COUNT_RESET;

	/* Just count up. */
	TIM0->CTCR = CTCR_CTM_TIMER;

	/* Prescale to a frequency that is good enough to get a decent resolution,
	but not too fast so as to overflow all the time. */
	TIM0->PR =  (configCPU_CLOCK_HZ/10000UL) - 1UL;

	/* Start the counter. */
	TIM0->TCR = TCR_COUNT_ENABLE;
}

void vApplicationIdleHook( void )
{
	vtITMu8(vtITMPortIdle,SCB->SCR);
	__WFI(); // Go to sleep until an interrupt occurs
	// DO NOT DO THIS... It is not compatible with the debugger: __WFE();
	// Go into low power until some (not quite sure what...) event occurs
	vtITMu8(vtITMPortIdle,SCB->SCR+0x10);
}

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            

/*------------------------------------------------------------------------------
 * File:		main.c
 * Authors: 	FreeRTOS, Igor Janjic
 * Description:	Application main entry point.
 *----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * Includes
 */
#include "FreeRTOS.h"
#include "task.h"
#include "system_LPC17xx.h"

#include "partest.h"
#include "vtUtilities.h"
#include "taskLCD.h"
#include "taskSensors.h"
#include "vtI2C.h"
#include "myTimers.h"
#include "taskConductor.h"
#include "debug.h"

/*------------------------------------------------------------------------------
 * Syscalls Initialization
 */
#include "syscalls.h"
#include <stdio.h>

// The time between cycles of the 'check' hook.
#define CHECK_DELAY			((portTickType)5000/portTICK_RATE_MS)

// Task priorities.
#define LCD_TASK_PRIORITY		(tskIDLE_PRIORITY)
#define IR_TASK_PRIORITY		(tskIDLE_PRIORITY)
#define MONITOR_TASK_PRIORITY		(tskIDLE_PRIORITY)
#define CONDUCTOR_TASK_PRIORITY		(tskIDLE_PRIORITY)

// Configure the hardware.
static void setupHardware(void);

/*------------------------------------------------------------------------------
 * Task Datastructures
 */
static vtI2CStruct vtI2C0;
static structSensor dataIR00;
static structSensor dataIR01;
static structSensor dataIR10;
static structSensor dataIR11;
static structSensor dataIR20;
static structSensor dataIR21;
static structSensor dataIR30;
static structSensor dataIR31;
static structSensor dataIR40;
static structSensor dataIR41;
static structSensor dataAC00;

static structConductor dataConductor;
static structLCD dataLCD;

/*------------------------------------------------------------------------------
 * Application Main Entry Point
 */
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
	
	// I2C task.
	if (vtI2CInit(&vtI2C0, 0, MONITOR_TASK_PRIORITY,100000) != vtI2CInitSuccess)
		VT_HANDLE_FATAL_ERROR(0);

	// IR00 sensor task.
	startTaskSensor(sensorIR00, &dataIR00, IR_TASK_PRIORITY, &vtI2C0, &dataLCD);
	startTimerSensor(sensorIR00, &dataIR00);

	// IR01 sensor task.
	startTaskSensor(sensorIR01, &dataIR01, IR_TASK_PRIORITY, &vtI2C0, &dataLCD);
	startTimerSensor(sensorIR01, &dataIR01);

	// IR10 sensor task.
	startTaskSensor(sensorIR10, &dataIR10, IR_TASK_PRIORITY, &vtI2C0, &dataLCD);
	startTimerSensor(sensorIR10, &dataIR10);

	// IR11 sensor task.
	startTaskSensor(sensorIR11, &dataIR11, IR_TASK_PRIORITY, &vtI2C0, &dataLCD);
	startTimerSensor(sensorIR11, &dataIR11);

	// IR20 sensor task.
	startTaskSensor(sensorIR20, &dataIR20, IR_TASK_PRIORITY, &vtI2C0, &dataLCD);
	startTimerSensor(sensorIR20, &dataIR20);

	// IR21 sensor task.
	startTaskSensor(sensorIR21, &dataIR21, IR_TASK_PRIORITY, &vtI2C0, &dataLCD);
	startTimerSensor(sensorIR21, &dataIR21);

	// IR30 sensor task.
	startTaskSensor(sensorIR30, &dataIR30, IR_TASK_PRIORITY, &vtI2C0, &dataLCD);
	startTimerSensor(sensorIR30, &dataIR30);

	// IR30 sensor task.
	startTaskSensor(sensorIR31, &dataIR31, IR_TASK_PRIORITY, &vtI2C0, &dataLCD);
	startTimerSensor(sensorIR31, &dataIR31);

	// IR40 sensor task.
	startTaskSensor(sensorIR40, &dataIR40, IR_TASK_PRIORITY, &vtI2C0, &dataLCD);
	startTimerSensor(sensorIR40, &dataIR40);

	// IR41 sensor task.
	startTaskSensor(sensorIR41, &dataIR41, IR_TASK_PRIORITY, &vtI2C0, &dataLCD);
	startTimerSensor(sensorIR41, &dataIR41);

	// AC00 sensor task.
	startTaskSensor(sensorAC00, &dataAC00, IR_TASK_PRIORITY, &vtI2C0, &dataLCD);
	startTimerSensor(sensorAC00, &dataAC00);

	// Conductor task.
	startTaskConductor(&dataConductor, CONDUCTOR_TASK_PRIORITY, &vtI2C0,
		&dataIR00, &dataIR01,
		&dataIR10, &dataIR11,
		&dataIR20, &dataIR21,
		&dataIR30, &dataIR31,
		&dataIR40, &dataIR41,
		&dataAC00);
	
	// Schedlar task.
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
	GPIO_SetDir(0, DEBUG_PIN15 | DEBUG_PIN16 | DEBUG_PIN17 | DEBUG_PIN18, 1);
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
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                

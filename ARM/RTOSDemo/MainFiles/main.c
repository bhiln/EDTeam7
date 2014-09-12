/*------------------------------------------------------------------------------
 * File:		main.c
 * Authors: 	FreeRTOS, Igor Janjic
 * Description:	Application main entry point.
 *----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * Includes
 */
// Schedular includes.
#include "FreeRTOS.h"
#include "task.h"
#include "system_LPC17xx.h"

#include "partest.h"
#include "vtUtilities.h"
#include "lcdTask.h"
#include "i2cTemp.h"
#include "vtI2C.h"
#include "myTimers.h"
#include "conductor.h"

/*------------------------------------------------------------------------------
 * Syscalls Initialization
 */
#include "syscalls.h"
#include <stdio.h>

// The time between cycles of the 'check' hook.
#define mainCHECK_DELAY						( ( portTickType ) 5000 / portTICK_RATE_MS )

// Task priorities.
#define mainLCD_TASK_PRIORITY				( tskIDLE_PRIORITY)
#define mainI2CTEMP_TASK_PRIORITY			( tskIDLE_PRIORITY)
#define mainI2CMONITOR_TASK_PRIORITY		( tskIDLE_PRIORITY)
#define mainCONDUCTOR_TASK_PRIORITY			( tskIDLE_PRIORITY)

// Configure the hardware.
static void prvSetupHardware( void );

/*------------------------------------------------------------------------------
 * Task Datastructures
 */
static vtI2CStruct vtI2C0; 				// Data structure required for one I2C task.
static vtTempStruct tempSensorData; 	// Data structure required for one temperature sensor task
static vtConductorStruct conductorData;	// Data structure required for conductor task
static vtLCDStruct vtLCDdata; 			// Data structure required for LCDtask API

/*------------------------------------------------------------------------------
 * Application Main Entry Point
 */
int main( void )
{
	// Initialize system.
	init_syscalls();

	// Set up the LED ports.
	vtInitLED();

	// Set up hardware.
	prvSetupHardware();

	// LCD task.
	StartLCDTask(&vtLCDdata,mainLCD_TASK_PRIORITY);
	startTimerForLCD(&vtLCDdata);
	
	// I2CTemp task.
	if (vtI2CInit(&vtI2C0,0,mainI2CMONITOR_TASK_PRIORITY,100000) != vtI2CInitSuccess) {
		VT_HANDLE_FATAL_ERROR(0);
	}

	// Temperature sensor task.
	vStarti2cTempTask(&tempSensorData,mainI2CTEMP_TASK_PRIORITY,&vtI2C0,&vtLCDdata);
	startTimerForTemperature(&tempSensorData);

	// Conductor task.
	vStartConductorTask(&conductorData,mainCONDUCTOR_TASK_PRIORITY,&vtI2C0,&tempSensorData);
	
	// Schedlar task.
	vTaskStartScheduler();

    // Shouldn't reach this part unless insufficient memory.
	for( ;; );
}

void vApplicationTickHook( void )
{
	static unsigned long ulTicksSinceLastDisplay = 0;

	// Have enough ticks passed to make it	time to perform our health status
	// check again?
	ulTicksSinceLastDisplay++;
	if( ulTicksSinceLastDisplay >= mainCHECK_DELAY )
	{
		// Reset counter.
		ulTicksSinceLastDisplay = 0;
	}
}

void prvSetupHardware( void )
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
}

// This function will get called if a task overflows its stack.
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName )
{
	( void ) pxTask;
	( void ) pcTaskName;
	VT_HANDLE_FATAL_ERROR(0);
	for( ;; );
}

void vConfigureTimerForRunTimeStats( void )
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
	TIM0->PR =  ( configCPU_CLOCK_HZ / 10000UL ) - 1UL;

	/* Start the counter. */
	TIM0->TCR = TCR_COUNT_ENABLE;
}
/*-----------------------------------------------------------*/
void vApplicationIdleHook( void )
{
	vtITMu8(vtITMPortIdle,SCB->SCR);
	__WFI(); // Go to sleep until an interrupt occurs
	// DO NOT DO THIS... It is not compatible with the debugger: __WFE();
	// Go into low power until some (not quite sure what...) event occurs
	vtITMu8(vtITMPortIdle,SCB->SCR+0x10);
}

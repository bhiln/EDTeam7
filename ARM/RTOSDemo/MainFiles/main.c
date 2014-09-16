/*------------------------------------------------------------------------------
 * File:		main.c
 * Authors: 	FreeRTOS, Igor Janjic
 * Description:	Application main entry point.
 *----------------------------------------------------------------------------*/

/*
    FreeRTOS V6.1.1 - Copyright (C) 2011 Real Time Engineers Ltd.

    ***************************************************************************
    *                                                                         *
    * If you are:                                                             *
    *                                                                         *
    *    + New to FreeRTOS,                                                   *
    *    + Wanting to learn FreeRTOS or multitasking in general quickly       *
    *    + Looking for basic training,                                        *
    *    + Wanting to improve your FreeRTOS skills and productivity           *
    *                                                                         *
    * then take a look at the FreeRTOS books - available as PDF or paperback  *
    *                                                                         *
    *        "Using the FreeRTOS Real Time Kernel - a Practical Guide"        *
    *                  http://www.FreeRTOS.org/Documentation                  *
    *                                                                         *
    * A pdf reference manual is also available.  Both are usually delivered   *
    * to your inbox within 20 minutes to two hours when purchased between 8am *
    * and 8pm GMT (although please allow up to 24 hours in case of            *
    * exceptional circumstances).  Thank you for your support!                *
    *                                                                         *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    ***NOTE*** The exception to the GPL is included to allow you to distribute
    a combined work that includes FreeRTOS without being obliged to provide the
    source code for proprietary components outside of the FreeRTOS kernel.
    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public 
    License and the FreeRTOS license exception along with FreeRTOS; if not it 
    can be viewed here: http://www.freertos.org/a00114.html and also obtained 
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/

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
#include "i2cIR0.h"
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
#define mainLCD_TASK_PRIORITY				(tskIDLE_PRIORITY)
#define mainI2CTEMP_TASK_PRIORITY			(tskIDLE_PRIORITY)
#define mainI2CIR0_TASK_PRIORITY			(tskIDLE_PRIORITY)
#define mainI2CMONITOR_TASK_PRIORITY		(tskIDLE_PRIORITY)
#define mainCONDUCTOR_TASK_PRIORITY			(tskIDLE_PRIORITY)

// Configure the hardware.
static void prvSetupHardware( void );

/*------------------------------------------------------------------------------
 * Task Datastructures
 */
static vtI2CStruct vtI2C0; 				// Data structure required for one I2C task
static vtIR0Struct ir0SensorData;		// Data structure required for one IR0 sensor task
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
	
	// I2C task.
	if (vtI2CInit(&vtI2C0,0,mainI2CMONITOR_TASK_PRIORITY,100000) != vtI2CInitSuccess) {
		VT_HANDLE_FATAL_ERROR(0);
	}

	// IR0 sensor task.
	vStarti2cIR0Task(&ir0SensorData, mainI2CIR0_TASK_PRIORITY, &vtI2C0, &vtLCDdata);
	startTimerForIR0(&ir0SensorData);

	// Conductor task.
	vStartConductorTask(&conductorData,mainCONDUCTOR_TASK_PRIORITY,&vtI2C0, &ir0SensorData);
	
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

void vApplicationIdleHook( void )
{
	vtITMu8(vtITMPortIdle,SCB->SCR);
	__WFI(); // Go to sleep until an interrupt occurs
	// DO NOT DO THIS... It is not compatible with the debugger: __WFE();
	// Go into low power until some (not quite sure what...) event occurs
	vtITMu8(vtITMPortIdle,SCB->SCR+0x10);
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
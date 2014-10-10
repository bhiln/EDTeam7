#include "FreeRTOS.h"
#include "task.h"
#include "projdefs.h"
#include "timers.h"

#include "vtUtilities.h"
#include "myTimers.h"
#include "I2CTaskMsgTypes.h"
#include "debug.h"

void LCDTimerCallback(xTimerHandle pxTimer)
{
	if (pxTimer == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		structLCD *ptr = (structLCD*)pvTimerGetTimerID(pxTimer);
		if (SendLCDTimerMsg(ptr, lcdWRITE_RATE_BASE,0) == errQUEUE_FULL)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

void startTimerLCD(structLCD* dataLCD) {
	if (sizeof(long) != sizeof(structLCD*))
		VT_HANDLE_FATAL_ERROR(0);

	xTimerHandle LCDTimerHandle = xTimerCreate((const signed char*)"LCD Timer", lcdWRITE_RATE_BASE, pdTRUE, (void*) dataLCD, LCDTimerCallback);
	if (LCDTimerHandle == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		if (xTimerStart(LCDTimerHandle,0) != pdPASS)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

void timerCallbackLocate(xTimerHandle pxTimer)
{
	if (pxTimer == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		structLocate* ptr = (structLocate*)pvTimerGetTimerID(pxTimer);
		if (sendTimerMsgLocate(ptr, LOCATE_WRITE_RATE_BASE, 0) == errQUEUE_FULL)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

void startTimerLocate(structLocate* dataLocate) {
	if (sizeof(long) != sizeof(structLocate*))
		VT_HANDLE_FATAL_ERROR(0);

	xTimerHandle timerHandleLocate = xTimerCreate((const signed char*)taskNameLocate, LOCATE_WRITE_RATE_BASE, pdTRUE, (void*)dataLocate, timerCallbackLocate);
	if (timerHandleLocate == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		if (xTimerStart(timerHandleLocate, 0) != pdPASS)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

void timerCallbackCommand(xTimerHandle pxTimer)
{
	if (pxTimer == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		structCommand* ptr = (structCommand*)pvTimerGetTimerID(pxTimer);
		if (sendTimerMsgCommand(ptr, COMMAND_WRITE_RATE_BASE, 0) == errQUEUE_FULL)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

void startTimerCommand(structCommand* dataCommand)
{
	if (sizeof(long) != sizeof(structCommand*))
		VT_HANDLE_FATAL_ERROR(0);

	xTimerHandle timerHandleCommand = xTimerCreate((const signed char*)taskNameCommand, COMMAND_WRITE_RATE_BASE, pdTRUE, (void*)dataCommand, timerCallbackCommand);
	if (timerHandleCommand == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		if (xTimerStart(timerHandleCommand, 0) != pdPASS)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

void timerCallbackSensor(xTimerHandle pxTimer)
{
	if (pxTimer == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		structSensor *ptr = (structSensor*) pvTimerGetTimerID(pxTimer);
		if (sendTimerMsgSensor(ptr, msgSensorTimer, SENSOR_WRITE_RATE_BASE, 0) == errQUEUE_FULL)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

void startTimerSensor(structSensor* dataSensor)
{
	if (sizeof(long) != sizeof(structSensor*))
		VT_HANDLE_FATAL_ERROR(0);

	xTimerHandle timerHandleSensor = xTimerCreate((const signed char*)taskNameSensor, SENSOR_WRITE_RATE_BASE, pdTRUE, (void*)dataSensor, timerCallbackSensor);
	if (timerHandleSensor == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		if (xTimerStart(timerHandleSensor, 0) != pdPASS)
			VT_HANDLE_FATAL_ERROR(0);
	}
}


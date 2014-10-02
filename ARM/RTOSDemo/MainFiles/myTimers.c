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

	xTimerHandle timerHandleLocate = xTimerCreate((const signed char*)locateName, LOCATE_WRITE_RATE_BASE, pdTRUE, (void*)dataLocate, timerCallbackLocate);
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

	xTimerHandle timerHandleCommand = xTimerCreate((const signed char*)commandName, COMMAND_WRITE_RATE_BASE, pdTRUE, (void*)dataCommand, timerCallbackCommand);
	if (timerHandleCommand == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		if (xTimerStart(timerHandleCommand, 0) != pdPASS)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

// Sensor IR00.
void timerCallbackIR00(xTimerHandle pxTimer)
{
	if (pxTimer == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		structSensor *ptr = (structSensor*) pvTimerGetTimerID(pxTimer);
		if (sendTimerMsgSensor(ptr, msgTypeTimerIR00, IR00_WRITE_RATE_BASE, 0) == errQUEUE_FULL)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

// Sensor IR01
void timerCallbackIR01(xTimerHandle pxTimer)
{
	if (pxTimer == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		structSensor *ptr = (structSensor*) pvTimerGetTimerID(pxTimer);
		if (sendTimerMsgSensor(ptr, msgTypeTimerIR01, IR01_WRITE_RATE_BASE, 0) == errQUEUE_FULL)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

// Sensor IR10.
void timerCallbackIR10(xTimerHandle pxTimer)
{
	if (pxTimer == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		structSensor *ptr = (structSensor*) pvTimerGetTimerID(pxTimer);
		if (sendTimerMsgSensor(ptr, msgTypeTimerIR10, IR10_WRITE_RATE_BASE, 0) == errQUEUE_FULL)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

// Sensor IR11.
void timerCallbackIR11(xTimerHandle pxTimer)
{
	if (pxTimer == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		structSensor *ptr = (structSensor*) pvTimerGetTimerID(pxTimer);
		if (sendTimerMsgSensor(ptr, msgTypeTimerIR11, IR11_WRITE_RATE_BASE, 0) == errQUEUE_FULL)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

// Sensor IR20.
void timerCallbackIR20(xTimerHandle pxTimer)
{
	if (pxTimer == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		structSensor *ptr = (structSensor*) pvTimerGetTimerID(pxTimer);
		if (sendTimerMsgSensor(ptr, msgTypeTimerIR20, IR20_WRITE_RATE_BASE, 0) == errQUEUE_FULL)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

// Sensor IR21.
void timerCallbackIR21(xTimerHandle pxTimer)
{
	if (pxTimer == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		structSensor *ptr = (structSensor*) pvTimerGetTimerID(pxTimer);
		if (sendTimerMsgSensor(ptr, msgTypeTimerIR21, IR21_WRITE_RATE_BASE, 0) == errQUEUE_FULL)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

// Sensor IR30.
void timerCallbackIR30(xTimerHandle pxTimer)
{
	if (pxTimer == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		structSensor *ptr = (structSensor*) pvTimerGetTimerID(pxTimer);
		if (sendTimerMsgSensor(ptr, msgTypeTimerIR30, IR30_WRITE_RATE_BASE, 0) == errQUEUE_FULL)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

// Sensor IR31.
void timerCallbackIR31(xTimerHandle pxTimer)
{
	if (pxTimer == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		structSensor *ptr = (structSensor*) pvTimerGetTimerID(pxTimer);
		if (sendTimerMsgSensor(ptr, msgTypeTimerIR31, IR31_WRITE_RATE_BASE, 0) == errQUEUE_FULL)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

// Sensor IR40.
void timerCallbackIR40(xTimerHandle pxTimer)
{
	if (pxTimer == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		structSensor *ptr = (structSensor*) pvTimerGetTimerID(pxTimer);
		if (sendTimerMsgSensor(ptr, msgTypeTimerIR40, IR40_WRITE_RATE_BASE, 0) == errQUEUE_FULL)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

// Sensor IR41.
void timerCallbackIR41(xTimerHandle pxTimer)
{
	if (pxTimer == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		structSensor *ptr = (structSensor*) pvTimerGetTimerID(pxTimer);
		if (sendTimerMsgSensor(ptr, msgTypeTimerIR41, IR41_WRITE_RATE_BASE, 0) == errQUEUE_FULL)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

// Sensor AC00.
void timerCallbackAC00(xTimerHandle pxTimer)
{
	if (pxTimer == NULL)
		VT_HANDLE_FATAL_ERROR(0);
	else
	{
		structSensor *ptr = (structSensor*) pvTimerGetTimerID(pxTimer);
		if (sendTimerMsgSensor(ptr, msgTypeTimerAC00, AC00_WRITE_RATE_BASE, 0) == errQUEUE_FULL)
			VT_HANDLE_FATAL_ERROR(0);
	}
}

void startTimerSensor(uint8_t sensor, structSensor* dataSensor)
{
	if (sizeof(long) != sizeof(structSensor*))
		VT_HANDLE_FATAL_ERROR(0);

	switch(sensor)
	{
		case sensorIR00:
		{
			xTimerHandle timerHandleIR00 = xTimerCreate((const signed char*)sensorNameIR00, IR00_WRITE_RATE_BASE, pdTRUE, (void*)dataSensor, timerCallbackIR00);
			if (timerHandleIR00 == NULL)
				VT_HANDLE_FATAL_ERROR(0);
			else
			{
				if (xTimerStart(timerHandleIR00, 0) != pdPASS)
					VT_HANDLE_FATAL_ERROR(0);
			}
		}
		case sensorIR01:
		{
			xTimerHandle timerHandleIR01 = xTimerCreate((const signed char*)sensorNameIR01, IR01_WRITE_RATE_BASE, pdTRUE, (void*)dataSensor, timerCallbackIR01);
			if (timerHandleIR01 == NULL)
				VT_HANDLE_FATAL_ERROR(0);
			else
			{
				if (xTimerStart(timerHandleIR01, 0) != pdPASS)
					VT_HANDLE_FATAL_ERROR(0);
			}
		}
		case sensorIR10:
		{
			xTimerHandle timerHandleIR10 = xTimerCreate((const signed char*)sensorNameIR10, IR10_WRITE_RATE_BASE, pdTRUE, (void*)dataSensor, timerCallbackIR10);
			if (timerHandleIR10 == NULL)
				VT_HANDLE_FATAL_ERROR(0);
			else
			{
				if (xTimerStart(timerHandleIR10, 0) != pdPASS)
					VT_HANDLE_FATAL_ERROR(0);
			}
		}
		case sensorIR11:
		{
			xTimerHandle timerHandleIR11 = xTimerCreate((const signed char*)sensorNameIR11, IR11_WRITE_RATE_BASE, pdTRUE, (void*)dataSensor, timerCallbackIR11);
			if (timerHandleIR11 == NULL)
				VT_HANDLE_FATAL_ERROR(0);
			else
			{
				if (xTimerStart(timerHandleIR11, 0) != pdPASS)
					VT_HANDLE_FATAL_ERROR(0);
			}
		}
		case sensorIR20:
		{
			xTimerHandle timerHandleIR20 = xTimerCreate((const signed char*)sensorNameIR20, IR20_WRITE_RATE_BASE, pdTRUE, (void*)dataSensor, timerCallbackIR20);
			if (timerHandleIR20 == NULL)
				VT_HANDLE_FATAL_ERROR(0);
			else
			{
				if (xTimerStart(timerHandleIR20, 0) != pdPASS)
					VT_HANDLE_FATAL_ERROR(0);
			}
		}
		case sensorIR21:
		{
			xTimerHandle timerHandleIR21 = xTimerCreate((const signed char*)sensorNameIR21, IR21_WRITE_RATE_BASE, pdTRUE, (void*)dataSensor, timerCallbackIR21);
			if (timerHandleIR21 == NULL)
				VT_HANDLE_FATAL_ERROR(0);
			else
			{
				if (xTimerStart(timerHandleIR21, 0) != pdPASS)
					VT_HANDLE_FATAL_ERROR(0);
			}
		}
		case sensorIR30:
		{
			xTimerHandle timerHandleIR30 = xTimerCreate((const signed char*)sensorNameIR30, IR30_WRITE_RATE_BASE, pdTRUE, (void*)dataSensor, timerCallbackIR30);
			if (timerHandleIR30 == NULL)
				VT_HANDLE_FATAL_ERROR(0);
			else
			{
				if (xTimerStart(timerHandleIR30, 0) != pdPASS)
					VT_HANDLE_FATAL_ERROR(0);
			}
		}
		case sensorIR31:
		{
			xTimerHandle timerHandleIR31 = xTimerCreate((const signed char*)sensorNameIR31, IR31_WRITE_RATE_BASE, pdTRUE, (void*)dataSensor, timerCallbackIR31);
			if (timerHandleIR31 == NULL)
				VT_HANDLE_FATAL_ERROR(0);
			else
			{
				if (xTimerStart(timerHandleIR31, 0) != pdPASS)
					VT_HANDLE_FATAL_ERROR(0);
			}
		}
		case sensorIR40:
		{
			xTimerHandle timerHandleIR40 = xTimerCreate((const signed char*)sensorNameIR40, IR40_WRITE_RATE_BASE, pdTRUE, (void*)dataSensor, timerCallbackIR40);
			if (timerHandleIR40 == NULL)
				VT_HANDLE_FATAL_ERROR(0);
			else
			{
				if (xTimerStart(timerHandleIR40, 0) != pdPASS)
					VT_HANDLE_FATAL_ERROR(0);
			}
		}
		case sensorIR41:
		{
			xTimerHandle timerHandleIR41 = xTimerCreate((const signed char*)sensorNameIR41, IR41_WRITE_RATE_BASE, pdTRUE, (void*)dataSensor, timerCallbackIR41);
			if (timerHandleIR41 == NULL)
				VT_HANDLE_FATAL_ERROR(0);
			else
			{
				if (xTimerStart(timerHandleIR41, 0) != pdPASS)
					VT_HANDLE_FATAL_ERROR(0);
			}
		}
		case sensorAC00:
		{
			xTimerHandle timerHandleAC00 = xTimerCreate((const signed char*)sensorNameAC00, AC00_WRITE_RATE_BASE, pdTRUE, (void*)dataSensor, timerCallbackAC00);
			if (timerHandleAC00 == NULL)
				VT_HANDLE_FATAL_ERROR(0);
			else
			{
				if (xTimerStart(timerHandleAC00, 0) != pdPASS)
					VT_HANDLE_FATAL_ERROR(0);
			}
		}
	}
}

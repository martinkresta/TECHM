/*
 * APP.c
 *
 *  Created on: Aug 14, 2021
 *      Author: Martin
 *      Brief:  Main application and compile time configuration
 *
 */

#include "main.h"
#include "APP.h"
#include "scheduler.h"
#include "leds.h"

#include "OW.h"
#include "TEMP.h"
#include "VARS.h"
#include "SCOM.h"
#include "MCAN.h"
#include "COM.h"
#include "DO.h"
#include "ADC.h"
#include "ELHEATER.h"
#include "WM.h"
#include "watchdog.h"
#include "RTC.h"
#include "HEATING.h"



// public methods
void APP_Init(void)
{
	Scheduler_Init();
	LED_Init();
	OW_Init();
	TEMP_Init();
	VAR_Init();
	SCOM_Init(&huart1);
	MCAN_Init(&hcan1, THIS_NODE);
  COM_Init(THIS_NODE);
  ELH_Init();
	ADC_Init(&hadc1, &hdma_adc1);
	DO_Init();
	WM_Init();
	WDG_Init(3000);
	HC_Init();
	//RTC_Init()
}

void APP_Start(void)
{

	sDateTime now;
	now.Day = 22;
	now.Hour = 21;
	now.Minute = 10;
	now.Month = 9;
	now.Second = 0;
	now.Year = 2021;

	RTC_SetTime(now);

	DO_SetElv(1);        // open watter supply valve
	DO_SetServoRad(0);   // close servo valves
	DO_SetServoWall(0);

	DO_SetPumps(0x06);

	MCAN_Start();

	VAR_SetVariable(VAR_BAT_VOLTAGE_V10, 520, 1);


	while (1)   // endless loop
	{
		Scheduler_Check_Flag();

	}
}






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
	//RTC_Init();



}

void APP_Start(void)
{

	sDateTime now;
	now.Day = 21;
	now.Hour = 16;
	now.Minute = 10;
	now.Month = 8;
	now.Second = 0;
	now.Year = 2021;

	RTC_SetTime(now);

	DO_SetElv(1);        // open watter supply valve
	DO_SetServoRad(0);   // close servo valves
	DO_SetServoWall(0);

	MCAN_Start();


	while (1)   // endless loop
	{
		Scheduler_Check_Flag();

	}
}






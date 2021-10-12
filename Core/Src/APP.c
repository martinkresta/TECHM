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


	/*Config temperature measurement*/

	// define hardware OW busses
	TEMP_AddHwBus(0,OW1_GPIO_Port, OW1_Pin);
	TEMP_AddHwBus(1,OW2_GPIO_Port, OW2_Pin);
	// TEMP_AddHwBus(2,OW3_GPIO_Port, OW3_Pin);  // not used so far


	// assign sensors on OW1 :
	TEMP_AssignSensor(T_TECHM, VAR_TEMP_TECHM_BOARD, 0);
	TEMP_AssignSensor(T108, VAR_TEMP_TANK_IN_H, 0);
	TEMP_AssignSensor(T109, VAR_TEMP_TANK_OUT_H, 0);
	TEMP_AssignSensor(T301, VAR_TEMP_TANK_1, 0);
	TEMP_AssignSensor(T302, VAR_TEMP_TANK_2, 0);
	TEMP_AssignSensor(T103, VAR_TEMP_TANK_3, 0);
	TEMP_AssignSensor(T104, VAR_TEMP_TANK_4, 0);
	TEMP_AssignSensor(T105, VAR_TEMP_TANK_5, 0);
	TEMP_AssignSensor(T306, VAR_TEMP_TANK_6, 0);

	// assign sensors on OW2 :
	TEMP_AssignSensor(T303, VAR_TEMP_BOILER, 1);
	TEMP_AssignSensor(T110, VAR_TEMP_BOILER_IN, 1);
	TEMP_AssignSensor(T107, VAR_TEMP_BOILER_OUT, 1);
	TEMP_AssignSensor(T101, VAR_TEMP_RAD_H, 1);
	TEMP_AssignSensor(T106, VAR_TEMP_RAD_C, 1);
	TEMP_AssignSensor(T102, VAR_TEMP_TANK_IN_C, 1);
	TEMP_AssignSensor(T8, VAR_TEMP_TANK_OUT_C, 1);
	TEMP_AssignSensor(T2, VAR_TEMP_WALL_IN, 1);
	TEMP_AssignSensor(T3, VAR_TEMP_WALL_OUT, 1);

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






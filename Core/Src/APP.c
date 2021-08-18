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

}

void APP_Start(void)
{

	DO_SetElv(1);
	MCAN_Start();

	//DO_SetServoRad(30);    // testing only
	//DO_SetServoWall(70);

	while (1)   // endless loop
	{
		Scheduler_Check_Flag();
	}
}






/*
 * APP.c
 *
 *  Created on: Aug 14, 2021
 *      Author: Martin
 *      Brief:  Main application and compile time configuration
 *
 */

#include <RTC.h>
#include "UI.h"
#include "main.h"
#include "APP.h"
#include "scheduler.h"
#include "OW.h"
#include "TEMP.h"
#include "VARS.h"
#include "SCOM.h"
#include "COM.h"
#include "DO.h"
#include "ADC.h"
#include "ELHEATER.h"
#include "WM.h"
#include "watchdog.h"
#include "HEATING.h"
#include "TEMPCON.h"
#include "BOILER_AIR.h"
#include "GEST.h"
#include "di.h"
#include "AVC.h"



s_CanRxMsg rmsg;

static void ProcessMessage(s_CanRxMsg* msg);

uint8_t mBoilerCleaningMode;

// public methods
void APP_Init(void)
{

	sUIHwInit uihw;
	uint8_t gi = 0;   //gesture index
	sGestInit gestInit;
	mBoilerCleaningMode = 0;

	Scheduler_Init();

	DI_Init();
	OW_Init();
	TEMP_Init();
	VAR_Init();
	SCOM_Init(&huart1);
	MCAN_Init(&hcan1, THIS_NODE);
  COM_Init(THIS_NODE);
  ELH_Init();
	ADC_Init(&hadc1, &hdma_adc1, NUM_OF_ADC_CHANNELS);
	DO_Init();
	WM_Init();
	WDG_Init(3000);
	HC_Init();
	TC_Init();





	/*Assign pins for onboard UI  */
	uihw.Led_Life.Pin = LED_Life_Pin;
	uihw.Led_Life.Port = LED_Life_GPIO_Port;
	uihw.Led_Life.Logic = eUIL_POSITIVE;

	uihw.Led_R.Pin = LED_R_Pin;
	uihw.Led_R.Port = LED_R_GPIO_Port;
	uihw.Led_R.Logic = eUIL_POSITIVE;

	uihw.Led_G.Pin = LED_G_Pin;
	uihw.Led_G.Port = LED_G_GPIO_Port;
	uihw.Led_G.Logic = eUIL_POSITIVE;

	uihw.Led_B.Pin = LED_B_Pin;
	uihw.Led_B.Port = LED_B_GPIO_Port;
	uihw.Led_B.Logic = eUIL_POSITIVE;

	uihw.Buzzer.Pin = BUZZ_Pin;
	uihw.Buzzer.Port = BUZZ_GPIO_Port;
	uihw.Buzzer.Logic = eUIL_POSITIVE;

	UI_Init(&uihw);
	UI_LED_Life_SetMode(eUI_BLINKING_SLOW);
	UI_LED_B_SetMode(eUI_ON);


	/*Gestures definition*/

	  gestInit.id = gi++;
    gestInit.action = GEST_STOVE_AV_TOGGLE;
    gestInit.type = egt_MultiTouch;
    gestInit.num_of_buttons = 1;
    gestInit.timeout = 500;
    gestInit.num_of_touches = 2;
    gestInit.btnIds[0] = IN1_STOVE_BTN;
    GEST_AddGesture(&gestInit);

	  gestInit.id = gi++;
	  gestInit.action = GEST_STOVE_AV_AUTO;
	  gestInit.type = egt_MultiTouch;
	  gestInit.num_of_buttons = 1;
	  gestInit.timeout = 500;
	  gestInit.num_of_touches = 3;
	  gestInit.btnIds[0] = IN1_STOVE_BTN;
	  GEST_AddGesture(&gestInit);

    gestInit.id = gi++;
    gestInit.action = GEST_STOVE_CELANING;
    gestInit.type = egt_MultiTouch;
    gestInit.num_of_buttons = 1;
    gestInit.timeout = 500;
    gestInit.num_of_touches = 4;
    gestInit.btnIds[0] = IN1_STOVE_BTN;
    GEST_AddGesture(&gestInit);

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

	/* Configure CAN streamed variables */

	COM_AddStreamedVariable(VAR_TEMP_TECHM_BOARD,1000);
	COM_AddStreamedVariable(VAR_EL_HEATER_STATUS, 3000);
	COM_AddStreamedVariable(VAR_EL_HEATER_POWER, 3000);
	COM_AddStreamedVariable(VAR_EL_HEATER_CURRENT, 3000);
	COM_AddStreamedVariable(VAR_EL_HEATER_CONS, 3000);

	COM_AddStreamedVariable(VAR_FLOW_COLD, 3000);
	COM_AddStreamedVariable(VAR_FLOW_HOT, 3000);
	COM_AddStreamedVariable(VAR_CONS_COLD, 3000);
	COM_AddStreamedVariable(VAR_CONS_HOT, 3000);


	COM_AddStreamedVariable(VAR_TEMP_BOILER, 3000);
	COM_AddStreamedVariable(VAR_TEMP_BOILER_IN, 3000);
	COM_AddStreamedVariable(VAR_TEMP_BOILER_OUT, 3000);
	COM_AddStreamedVariable(VAR_TEMP_TANK_IN_H, 3000);
	COM_AddStreamedVariable(VAR_TEMP_TANK_OUT_H, 3000);
	COM_AddStreamedVariable(VAR_TEMP_TANK_1, 3000);
	COM_AddStreamedVariable(VAR_TEMP_TANK_2, 3000);
	COM_AddStreamedVariable(VAR_TEMP_TANK_3, 3000);
	COM_AddStreamedVariable(VAR_TEMP_TANK_4, 3000);
	COM_AddStreamedVariable(VAR_TEMP_TANK_5, 3000);
	COM_AddStreamedVariable(VAR_TEMP_TANK_6, 3000);
	COM_AddStreamedVariable(VAR_TEMP_WALL_IN, 3000);
	COM_AddStreamedVariable(VAR_TEMP_WALL_OUT, 3000);
	COM_AddStreamedVariable(VAR_TEMP_BOILER_EXHAUST, 3000);
	COM_AddStreamedVariable(VAR_TEMP_RAD_H, 3000);
	COM_AddStreamedVariable(VAR_TEMP_RAD_C, 3000);
	COM_AddStreamedVariable(VAR_TEMP_TANK_IN_C, 3000);
	COM_AddStreamedVariable(VAR_TEMP_TANK_OUT_C, 3000);

	COM_AddStreamedVariable(VAR_HEAT_HEATING_WH, 3000);
	COM_AddStreamedVariable(VAR_HEAT_TOTAL_WH, 3000);

	COM_AddStreamedVariable(VAR_BOILER_POWER, 3000);
	COM_AddStreamedVariable(VAR_BOILER_HEAT, 3000);

}

void APP_Start(void)
{

	DO_SetElv(1);        // open watter supply valve

	AVC_Init();
	BAC_SetAutoMode();

	MCAN_Start();

	VAR_SetVariable(VAR_BAT_VOLTAGE_V10, 520, 1);


	while (1)   // endless loop
	{
		Scheduler_Check_Flag();

		// Process received COM messages

		s_CanRxMsg rmsg;
		while(1 == COM_GetRxMessage(&rmsg))  // process all messages in buffer
		{
				ProcessMessage(&rmsg);
		}

	}
}

void APP_Update_1s(void)
{
	// check midnight
	static uint8_t dayNumber = 0;
	uint8_t newDayNumber = 0;
	newDayNumber = RTC_GetTime().Day;
	if (dayNumber != newDayNumber)
	{
		dayNumber = newDayNumber;

		// Reset counters
		WM_ResetConsupmtions();
		ELH_Midnight();
		HC_Midnight();
	}


}


// map the digital input to the button ID and propagate the information
void APP_DiInputChanged(uint8_t inputId, eDI state)
{
  switch (inputId)
  {
    case IN1_STOVE_BTN:
      if(state == eDI_HI)
      {
        // start heating by simple button press
        BAC_StartHeating();
      }
      else
      {
        // button released, do nothing
      }
      break;
    case IN2_STOVE_DOOR:
      if(state == eDI_HI) // door closed
      {
        // inform heating controller
        HC_DoorClosed();
        if (mBoilerCleaningMode == 1)
        {
          mBoilerCleaningMode = 0;
          // cancel RECU remote request
          COM_SendRecuRemoteRequest(errm_AutoControl, 0);
        }
      }
      else // door opened
      {
        // inform heating controller
        HC_DoorOpened();
        // inform  Air valve control
        BAC_DoorOpenDetected();
      }
      break;
    case IN3_AV_HOMEPOS:
      // do nothing, intput is processed asynchronously in AVC.c
      break;
  }
}

void APP_GestureDetected(uint8_t action)
{
  switch(action)
  {
    case GEST_STOVE_AV_TOGGLE:
      BAC_ManualToggle();
      break;
    case GEST_STOVE_AV_AUTO:
      BAC_SetAutoMode();
      break;
    case GEST_STOVE_CELANING:

      if(mBoilerCleaningMode == 0)
      {
        mBoilerCleaningMode = 1;
        // send RECU remote request
        COM_SendRecuRemoteRequest(errm_MaxOverpressure, 600);
      }
      else if(mBoilerCleaningMode == 1)
      {
        mBoilerCleaningMode = 0;
        // cancel RECU remote request
        COM_SendRecuRemoteRequest(errm_AutoControl, 0);
      }
      break;
  }
 // SendGesture(action);
}



/*Private methods*/

static void ProcessMessage(s_CanRxMsg* msg)
{
	uint16_t cmd = msg->header.StdId & 0xFF0;  // maskout nodeid
	//uint8_t producer = msg->header.StdId & 0x00F;  // maskout cmd
	int16_t par1,par2,par3; //,par4;
	uint32_t unixtime = 0;
	par1 = (msg->data[0] << 8) | msg->data[1];
	par2 = (msg->data[2] << 8) | msg->data[3];
	par3 = (msg->data[4] << 8) | msg->data[5];
//	par4 = (msg->data[6] << 8) | msg->data[7];

	switch (cmd)
	{
		case CMD_BUTTON_STATE:
			break;
		case  CMD_VAR_VALUE:
			VAR_SetVariable(par1, par2, par3);  // tbd check valid flag
			break;
		case CMD_RTC_SYNC: // set RTC time
			unixtime |= msg->data[0] << 24;
			unixtime |= msg->data[1] << 16;
			unixtime |= msg->data[2] << 8;
			unixtime |= msg->data[3];
			RTC_SetUnixTime(unixtime);
			UI_LED_B_SetMode(eUI_OFF);  // debug: when blue led is OFF RTC was set
			break;
		case CMD_BALANCE_INFO: // balancing information from ELECON to ELHEATER
			ELH_SetBalanceInfo(par1, par2);
			break;
	}
	return;
}



void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == WM1_Pin || GPIO_Pin == WM2_Pin )
  {
    WM_ExtiCallback(GPIO_Pin);
  }

 /* if(GPIO_Pin == AV_ENC1_Pin)   // OBSOLETE, used only for AVC_V1
  {
    AVC_ExtiCallback(GPIO_Pin);
  }*/
}


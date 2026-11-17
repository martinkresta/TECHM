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
#include "VARS2.h"   /* Vars2: APP.c wires the groups to the modules */
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
#include "UHAMON.h"
#include "LOG.h"
#include "CMD.h"



s_CanRxMsg rmsg;

static uint32_t mAppUpTime;

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
	MCAN_Init(&hcan1, THIS_NODE);
  COM_Init(THIS_NODE);
  ELH_Init(&Vars2.ElHeater, &Vars2.Temperatures.Techm, &Vars2.GlobalEnergy.Battery, &Vars2.Mppt);
	ADC_Init(&hadc1, &hdma_adc1, NUM_OF_ADC_CHANNELS);
	DO_Init();
	WM_Init(&Vars2.Water);
	WDG_Init(3000);
	HC_Init(&Vars2.Boiler, &Vars2.Temperatures.Techm);
	TC_Init(&Vars2.Tank, &Vars2.Temperatures);
	BAC_Init(&Vars2.Temperatures.Techm);
	UHAMON_Init();
  VARS2_SetGroupValid(&Vars2.States.Techm, 1);   // states valid from now on (values: APP_UpdateStates_1s())


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
	TEMP_AssignSensor(T_TECHM, VID_TEMP_TECHM_BOARD_C, 0);
	TEMP_AssignSensor(T108, VID_TEMP_TANK_IN_H_C, 0);
	TEMP_AssignSensor(T109, VID_TEMP_TANK_OUT_H_C, 0);
	TEMP_AssignSensor(T301, VID_TEMP_TANK_1_C, 0);
	TEMP_AssignSensor(T302, VID_TEMP_TANK_2_C, 0);
	TEMP_AssignSensor(T103, VID_TEMP_TANK_3_C, 0);
	TEMP_AssignSensor(T104, VID_TEMP_TANK_4_C, 0);
	TEMP_AssignSensor(T105, VID_TEMP_TANK_5_C, 0);
	TEMP_AssignSensor(T306, VID_TEMP_TANK_6_C, 0);

	// assign sensors on OW2 :
	TEMP_AssignSensor(T303, VID_TEMP_BOILER_C, 1);
	TEMP_AssignSensor(T110, VID_TEMP_BOILER_IN_C, 1);
	TEMP_AssignSensor(T107, VID_TEMP_BOILER_OUT_C, 1);
	TEMP_AssignSensor(T101, VID_TEMP_RAD_H_C, 1);
	TEMP_AssignSensor(T106, VID_TEMP_RAD_C_C, 1);
	TEMP_AssignSensor(T102, VID_TEMP_TANK_IN_C_C, 1);
	TEMP_AssignSensor(T8, VID_TEMP_TANK_OUT_C_C, 1);
	TEMP_AssignSensor(T2, VID_TEMP_WALL_IN_C, 1);
	TEMP_AssignSensor(T3, VID_TEMP_WALL_OUT_C, 1);

	/* CAN: VARS2 sends this node's variables (Temperatures.Techm, ElHeater, Water, Tank, Boiler) by the YAML period_ms */

}

void APP_Start(void)
{

	DO_SetElv(1);        // open watter supply valve

	AVC_Init();
	BAC_SetAutoMode();

	MCAN_Start();

	LOG_Startup();

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

	mAppUpTime++;
}


/* VARS2 states of TECHM (States.Techm): only reads what the control modules set in this 1 s cycle.
 * Called from the scheduler after ELH, HC, TC and BAC. Writes Vars2 only, no control. */
void APP_UpdateStates_1s(void)
{
	sVarsStatesTechm* s = &Vars2.States.Techm;
	uint8_t pumps = DO_GetPumps();
	uint8_t heaters = DO_GetElHeaters();
	uint32_t coils = 0;
	uint8_t bit;

	for (bit = 0; bit < 6; bit++)   // coils switched on = bits set in the mask given to DO_SetElHeaters()
	{
		coils += (heaters >> bit) & 1u;
	}

	s->UpReqTempDay_C.value = mUpReqTempDay;
	s->UpReqTempNight_C.value = mUpReqTempNight;
	s->DownReqTempDay_C.value = mDownReqTempDay;
	s->DownReqTempNight_C.value = mDownReqTempNight;
	s->ExhaustSetpoint_C.value = mBacControlTemp_C;
	s->UpReqTemp_C.value = mUpReqTemp_C;
	s->DownReqTemp_C.value = mDownReqTemp_C;
	s->RadOn.value = mRadOn ? 1 : 0;
	s->WallOn.value = mWallOn ? 1 : 0;
	s->PumpBoiler.value = (pumps & PUMP_BOILER) ? 1 : 0;
	s->PumpWall.value = (pumps & PUMP_WALL) ? 1 : 0;
	s->PumpRad.value = (pumps & PUMP_RAD) ? 1 : 0;
	s->ElHeaterCoils.value = coils;
	s->BoilerState.value = (int32_t)HC_GetStatus();
	s->ValveWall_pct.value = (float)DO_GetServoWall();
	s->ValveRad_pct.value = (float)DO_GetServoRad();
	s->AirValveSignal_pct.value = (float)DO_GetServoAirValve();
	s->ReqWaterRad_C.value = mReqWaterRad_C;
	s->ReqWaterWall_C.value = mReqWaterWall_C;
}


// commands for this node (CMD_spec.md, CAN 0x203): setpoints only, clamped in the module setters;
// feedback = Vars2.States.Techm (APP_UpdateStates_1s)
void APP_OnCommand(eCmdId id, float value)
{
	switch (id)
	{
		case CMDID_UP_REQ_DAY:     TC_SetUpReqTempDay(value);     break;
		case CMDID_UP_REQ_NIGHT:   TC_SetUpReqTempNight(value);   break;
		case CMDID_DOWN_REQ_DAY:   TC_SetDownReqTempDay(value);   break;
		case CMDID_DOWN_REQ_NIGHT: TC_SetDownReqTempNight(value); break;
		case CMDID_EXHAUST_SET:    BAC_SetControlTemp(value);     break;
		default:
			break;
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
	int16_t par1,par2; //,par3,par4;
	uint32_t unixtime = 0;
	par1 = (msg->data[0] << 8) | msg->data[1];
	par2 = (msg->data[2] << 8) | msg->data[3];
//	par4 = (msg->data[6] << 8) | msg->data[7];

	switch (cmd)
	{
		case CMD_BUTTON_STATE:
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
		case CMD_UHAMON_RECIEVE:  // uhamon message received
		  // set the SEND function:
		  UHAMON_SetSendFunction(&COM_SendUhamonMessage);
		  UHAMON_ProcessInput(msg->data);
		  break;
		case CMD_SYSINFO_REQ:
      if(msg->data[0] == COM_GetNodeId())
      {
         switch (msg->data[1])
         {
           case esit_ASW_version:
             COM_SendSysInfo(esit_ASW_version, (uint32_t)ASW_VERSION);
             break;
           case esit_UpTime:
             COM_SendSysInfo(esit_UpTime, mAppUpTime);
             break;
           case esit_HWID:
             COM_SendSysInfo(esit_HWID, (uint32_t)HWID);
             break;
         }
      }
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


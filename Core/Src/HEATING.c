/*
 * HEATING.c
 *
 *  Created on: Aug 29, 2021
 *      Author: Martin
 *      Brief: Control logic of home heating system and solid fuel boiler
 */

#include "HEATING.h"
#include "VARS2.h"
#include "DO.h"
#include "UI.h"
#include "COM.h"


eBoilerState mBoilerState;
eBoilerError mBoilerError;
eBoilerState mLastState;

uint8_t mPumpFailure;
uint8_t mPumpMask;
uint16_t mLastWarningTime;
uint8_t mBeepCount;
uint32_t mTodayHeat_Ws;
uint16_t mTodayHeat_Wh;
uint32_t mPumpOnTime_s;

uint8_t mBoilerLoading; // flag indicating that doors were opened while heating (loading wood)

static sVarsBoiler* mpVars;                    /* produced */
static const sVarsTemperaturesTechm* mpTemps;  /* read */



void HC_Init(sVarsBoiler* pVars, const sVarsTemperaturesTechm* pTemps)
{
	mpVars = pVars;
	mpTemps = pTemps;
	mBoilerState = eBs_Idle;
	mLastState = eBs_Idle;
	mBoilerError = eBe_NoError;
	mPumpMask = 0x06;
	mPumpFailure = 0;
	mBeepCount = 0;
	mTodayHeat_Ws = 0;
	mBoilerLoading = 0;
}



uint16_t HC_GetStatus(void)
{
	return mBoilerState;
}


void HC_DoorOpened(void)
{
  if(mBoilerLoading == 0 && mBoilerState != eBs_Idle) // doors were opened
  {
    mBoilerLoading = 1;
    // send RECU remote request
    COM_SendRecuRemoteRequest(errm_MaxOverpressure, 30);
  }

}
void HC_DoorClosed(void)
{
  if(mBoilerLoading == 1)  // doors closed
  {
    mBoilerLoading = 0;
    // cancel RECU remote request
    COM_SendRecuRemoteRequest(errm_AutoControl, 0);
  }
}




void HC_Update_1s(void)
{
	// inputs in degC. The old code compared whole degC (truncated); the comparisons below switch at the
	// same values (e.g. "trunc > 86" is ">= 87")
	float boilerTemp_C;
	float boilerExhaust_C;
	float boilerIn_C;
	float boilerOut_C;
	float TankInHot_C;
	float Tank_C;
	float Tank1_C;
	float boilerDiff_C;
	float power_W;

	// collect the variables
	boilerTemp_C = mpTemps->Boiler_C.value;
	boilerExhaust_C = mpTemps->BoilerExhaust_C.value;
	boilerIn_C = mpTemps->BoilerIn_C.value;
	boilerOut_C = mpTemps->BoilerOut_C.value;
	TankInHot_C = mpTemps->TankInHot_C.value;
	Tank_C = mpTemps->Tank6_C.value;
	Tank1_C = mpTemps->Tank1_C.value;

	if (!(mpTemps->Boiler_C.valid && mpTemps->BoilerExhaust_C.valid && mpTemps->BoilerIn_C.valid &&
	      mpTemps->BoilerOut_C.valid && mpTemps->TankInHot_C.valid && mpTemps->TankOutCold_C.valid &&
	      mpTemps->Tank6_C.valid && mpTemps->Tank1_C.valid))
	{
		return;   // invalid inputs: no action, state kept (as before)
	}

	boilerDiff_C = boilerOut_C - boilerIn_C;



	/* Calculate power and today energy*/

	// expected water flow is 750l/hod,  the pump has to be set to p = 1.5m, => P = 10W
	// => Temperature difference of 0.1C equals to 92W of heating power
	if (mBoilerState == eBs_HeatUp || mBoilerState == eBS_Heating)
	{
		power_W = boilerDiff_C * BOILER_W_PER_C;
	}
	else
	{
		power_W = 0;
	}
	if (power_W > 0)   // negative power (exchanger colder at the outlet) is not counted
	{
		mTodayHeat_Ws += (uint32_t)power_W;
	}
	mTodayHeat_Wh = mTodayHeat_Ws / 3600;

	mpVars->Power_W.value = power_W;
	mpVars->HeatToday_Wh.value = (float)mTodayHeat_Wh;
	VARS2_SetGroupValid(mpVars, 1);


	switch (mBoilerState)
	{
		case eBs_Idle:
			mBeepCount = 0;
			if (boilerTemp_C >= TEMP_PUMP_ON)
			{
			  COM_SendACRemoteRequest(0,1,0xFFFF);  // keep AC On for ~ 18 hours
				DO_SetPumpBoiler(1); // turn on pump
				mPumpOnTime_s = 0;
				mBoilerState = eBs_HeatUp;
			}
			break;
		case eBs_HeatUp:
			if (TankInHot_C >= 50)
			{
				mBoilerState = eBS_Heating;
			}
			if (boilerTemp_C < (TEMP_PUMP_ON - 10) && mPumpOnTime_s > MIN_PUMP_ON_TIME)
			{
				DO_SetPumpBoiler(0);  // turn off pump
				COM_SendACRemoteRequest(0,0,1);  // Cancel the remote request
				mBoilerState = eBs_Idle;
			}
			break;
		case eBS_Heating:
			// boilerDiff_C < 0.95: whole degC diff <= 0 (old: 0.1 degC diff truncated to degC)
			if (boilerExhaust_C < 110  && boilerDiff_C < 0.95f && boilerTemp_C < (TEMP_BOILER_OVERHEAT-1)  && mPumpOnTime_s > MIN_PUMP_ON_TIME) // if chimney is cooling down and exchanger does not put heat to water
			{
				DO_SetPumpBoiler(0);  // turn off pump
				UI_Buzzer_SetMode(eUI_OFF);
				COM_SendACRemoteRequest(0,0,1);  // Cancel the remote request
				mBoilerState = eBS_CoolDown;
			}
			break;
		case eBS_CoolDown:
			{
				if (boilerTemp_C < (TEMP_PUMP_ON - 1)) // wait until exchanger is cold
				{
					mBoilerState = eBs_Idle;
				}
				//if ((boilerTemp_C > 75 && boilerExhaust_C > 110)|| boilerTemp_C > TEMP_BOILER_OVERHEAT || (boilerTemp_C > (Tank1_C + 2)) ) // if it gets warm again, turn on the pump
				if (boilerTemp_C >= (TEMP_BOILER_OVERHEAT + 1) || ( boilerTemp_C >= 70 && (boilerTemp_C > (Tank1_C + 2))) ) // if it gets warm again, turn on the pump
				{
				  COM_SendACRemoteRequest(0,1,0xFFFF);  // keep AC On for ~ 18 hours
					DO_SetPumpBoiler(1);  // turn on pump
					mPumpOnTime_s = 0;
					mBoilerState = eBs_HeatUp;
				}
			}
			break;
		case eBS_Overheat:
			// TBD warning
			break;
		case eBS_Error:
			break;
		case eBS_InvalidInputs:
			break;
		default:
			break;
	}

	/* Detection of faults and signalization */
	mBoilerError = eBe_NoError;

	if (Tank_C >= (MAX_TANK_TEMP_C + 1)) // overheated tank (whole degC > MAX_TANK_TEMP_C)
	{
		mBoilerError = eBe_OverheatedTank;
	}

	if (boilerTemp_C >= (TEMP_BOILER_OVERHEAT + 1)) // overheated boiler (whole degC > TEMP_BOILER_OVERHEAT)
	{
		mBoilerError = eBe_OverheatedBoiler;
	}

	if(mBoilerState == eBs_HeatUp  || mBoilerState == eBS_Heating)  // check pump only in states where pump should be ON
	{
	  mPumpOnTime_s ++ ;
		if (boilerTemp_C >= 78 && (boilerTemp_C > (boilerOut_C + 3)))  // pump failure (whole degC > 77)
		{
		  if(mPumpOnTime_s > 60)
		  {
		    mBoilerError = eBe_PumpFailure;
		  }
		}
	}


	switch (mBoilerError)
	{
		case eBe_OverheatedTank:
			UI_Buzzer_SetMode(eUI_BLINKING_SLOW);
			UI_LED_R_SetMode(eUI_ON);
			break;
		case eBe_OverheatedBoiler:
			UI_Buzzer_SetMode(eUI_BLINKING_FAST);
			UI_LED_R_SetMode(eUI_BLINKING_SLOW);
			break;
		case eBe_PumpFailure:
			UI_Buzzer_SetMode(eUI_BLINKING_FAST);
			UI_LED_R_SetMode(eUI_BLINKING_FAST);
			break;
		case eBe_NoError:
		default:
			UI_Buzzer_SetMode(eUI_OFF);
			UI_LED_R_SetMode(eUI_OFF);
	}

	/* Low fuel warning */

	mLastWarningTime++;
	if (mBoilerError == eBe_NoError && mBoilerState == eBS_Heating && mLastWarningTime > 60)
	{
		mLastWarningTime = 0;
		if (boilerExhaust_C < 150 && boilerExhaust_C >= 126 && Tank1_C < 70 )   // whole degC exhaust > 125
		{
			if (mBeepCount < 3)
			{
				UI_Buzzer_SetMode(eUI_BEEP_ONCE);
				mBeepCount++;
			}
		}
		else
		{
			mBeepCount = 0;
		}
	}
}


void HC_Midnight(void)
{
	mTodayHeat_Ws = 0;  // reset boiler energy counter
	mTodayHeat_Wh = 0;
}

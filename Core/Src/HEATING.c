/*
 * HEATING.c
 *
 *  Created on: Aug 29, 2021
 *      Author: Martin
 *      Brief: Control logic of home heating system and solid fuel boiler
 */

#include "HEATING.h"
#include "VARS.h"
#include "DO.h"
#include "UI.h"


eBoilerState mBoilerState;
eBoilerError mBoilerError;

uint8_t mPumpFailure;
uint8_t mPumpMask;
uint16_t mLastWarningTime;
uint8_t mBeepCount;
uint32_t mTodayHeat_Ws;
uint16_t mTodayHeat_Wh;



void HC_Init(void)
{
	mBoilerState = eBs_Idle;
	mBoilerError = eBe_NoError;
	mPumpMask = 0x06;
	mPumpFailure = 0;
	mBeepCount = 0;
	mTodayHeat_Ws = 0;
}



uint16_t HC_GetStatus(void)
{
	return mBoilerState;
}


void HC_Update_1s(void)
{
	uint16_t invalid;
	int16_t boilerTemp_C;
	int16_t boilerExhaust_C;
	int16_t boilerIn_C;
	int16_t boilerOut_C;
	int16_t TankInHot_C;
	int16_t TankOutCold_C;
	int16_t Tank_C;
	int16_t Tank1_C;
	int16_t boilerDiff;
	uint16_t power_W;

	// collect the variables
	invalid = 0;

	boilerTemp_C = VAR_GetVariable(VAR_TEMP_BOILER,&invalid)/10;
	boilerExhaust_C = VAR_GetVariable(VAR_TEMP_BOILER_EXHAUST,&invalid)/10;
	boilerIn_C= VAR_GetVariable(VAR_TEMP_BOILER_IN,&invalid);  ///10;
	boilerOut_C= VAR_GetVariable(VAR_TEMP_BOILER_OUT,&invalid); ///10;
	TankInHot_C= VAR_GetVariable(VAR_TEMP_TANK_IN_H,&invalid)/10;
	TankOutCold_C = VAR_GetVariable(VAR_TEMP_TANK_OUT_C,&invalid)/10;
	Tank_C = VAR_GetVariable(VAR_TEMP_TANK_6,&invalid)/10;
	Tank1_C = VAR_GetVariable(VAR_TEMP_TANK_1,&invalid)/10;

	boilerDiff = boilerOut_C - boilerIn_C;


	/* Calculate power and today energy*/

	// expected water flow is 750l/hod,  the pump has to be set to p = 1.5m, => P = 10W
	// => Temperature difference of 0.1C equals to 92W of heating power
	power_W = boilerDiff * 92;
	mTodayHeat_Ws += power_W;
	mTodayHeat_Wh = mTodayHeat_Ws / 3600;

	VAR_SetVariable(VAR_BOILER_POWER, power_W, 1);
	VAR_SetVariable(VAR_BOILER_HEAT, mTodayHeat_Wh, 1);


	// rescale to full degrees of C for further conditions
	boilerDiff /= 10;
	boilerOut_C /=10;
	boilerIn_C /=10;

	if (invalid)
	{
		mBoilerState = eBS_InvalidInputs;
		DO_SetPumpBoiler(1);
		return;
	}
	else
	{
		if (mBoilerState == eBS_InvalidInputs)
		{
			mBoilerState = eBs_Idle;
			DO_SetPumpBoiler(0); // turn off pump
		}
	}

	switch (mBoilerState)
	{
		case eBs_Idle:
			mBeepCount = 0;
			if (boilerTemp_C >= TEMP_PUMP_ON)
			{
				DO_SetPumpBoiler(1); // turn on pump
				mBoilerState = eBs_HeatUp;
			}
			break;
		case eBs_HeatUp:
			if (TankInHot_C >= 50)
			{
				mBoilerState = eBS_Heating;
			}
			if (boilerTemp_C < (TEMP_PUMP_ON - 5))
			{
				DO_SetPumpBoiler(0);  // turn off pump
				mBoilerState = eBs_Idle;
			}
			break;
		case eBS_Heating:
			if (boilerExhaust_C < 110  && boilerDiff <= 0) // if chimney is cooling down and exchanger does not put heat to water
			{
				DO_SetPumpBoiler(0);  // turn off pump
				UI_Buzzer_SetMode(eUI_OFF);
				mBoilerState = eBS_CoolDown;
			}
			break;
		case eBS_CoolDown:
			{
				if (boilerTemp_C < (TEMP_PUMP_ON - 1)) // wait until exchanger is cold
				{
					mBoilerState = eBs_Idle;
				}
				if (boilerTemp_C > 75 && boilerExhaust_C > 110) // if it gets warm again, turn on the pump
				{
					DO_SetPumpBoiler(1);  // turn on pump
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

	if (Tank_C > MAX_TANK_TEMP_C) // overheated tank
	{
		mBoilerError = eBe_OverheatedTank;
	}

	if (Tank_C > TEMP_BOILER_OVERHEAT) // overheated boiler
	{
		mBoilerError = eBe_OverheatedBoiler;
	}

	if(mBoilerState == eBs_HeatUp  || mBoilerState == eBS_Heating)  // check pump only in states where pump should be ON
	{
		if (boilerTemp_C > 77 && (boilerTemp_C > (boilerOut_C + 3)))  // pump failure
		{
			mBoilerError = eBe_PumpFailure;
		}
	}


	switch (mBoilerError)
	{
		case eBe_OverheatedTank:
			UI_Buzzer_SetMode(eUI_BLINKING_SLOW);
			UI_LED_R_SetMode(eUI_ON);
			break;
		case eBe_OverheatedBoiler:
			UI_Buzzer_SetMode(eUI_BLINKING_SLOW);
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
		if (boilerExhaust_C < 150  && Tank1_C < 70)
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
}

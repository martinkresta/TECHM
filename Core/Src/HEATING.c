/*
 * HEATING.c
 *
 *  Created on: Aug 29, 2021
 *      Author: Martin
 *      Brief: Control logic of home heating system and solid fuel boiler
 */

#include "heating.h"
#include "VARS.h"
#include "DO.h"


eBoilerState mBoilerState;

uint8_t mPumpMask;

void HC_Init(void)
{
	mBoilerState = eBs_Idle;
	mPumpMask = 0x06;
}



uint16_t HC_GetStatus(void)
{
	return mBoilerState;
}


void HC_Update_1s(void)
{
	int16_t invalid;
	int16_t boilerTemp_C;
	int16_t boilerExhaust_C;
	int16_t boilerIn_C;
	int16_t boilerOut_C;
	int16_t TankInHot_C;
	int16_t TankOutCold_C;
	int16_t Tank_C;
	int16_t boilerDiff;


// collect the all informations to make decision about the power




	// collect the variables
	invalid = 0;

	boilerTemp_C = VAR_GetVariable(VAR_TEMP_BOILER,&invalid)/10;
	boilerExhaust_C = VAR_GetVariable(VAR_TEMP_BOILER_EXHAUST,&invalid)/10;
	boilerIn_C= VAR_GetVariable(VAR_TEMP_BOILER_IN,&invalid)/10;
	boilerOut_C= VAR_GetVariable(VAR_TEMP_BOILER_OUT,&invalid)/10;
	TankInHot_C= VAR_GetVariable(VAR_TEMP_TANK_IN_H,&invalid)/10;
	TankOutCold_C = VAR_GetVariable(VAR_TEMP_TANK_OUT_C,&invalid)/10;
	Tank_C = VAR_GetVariable(VAR_TEMP_TANK_6,&invalid)/10;

	boilerDiff = boilerOut_C - boilerIn_C;

	if (invalid)
	{
		mBoilerState = eBS_InvalidInputs;
		mPumpMask | PUMP_BOILER;
		DO_SetPumps(mPumpMask);
		return;
	}
	else
	{
		if (mBoilerState == eBS_InvalidInputs)
		{
			mBoilerState = eBs_Idle;
			mPumpMask &= ~PUMP_BOILER;  // turn off pump
			DO_SetPumps(mPumpMask);
		}
	}

	switch (mBoilerState)
	{
		case eBs_Idle:
			if (boilerTemp_C >= TEMP_PUMP_ON)
			{
				mPumpMask |= PUMP_BOILER;  // turn on pump
				DO_SetPumps(mPumpMask);
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
				mPumpMask &= ~PUMP_BOILER;  // turn off pump
				DO_SetPumps(mPumpMask);
				mBoilerState = eBs_Idle;
			}
			break;
		case eBS_Heating:
			if (Tank_C > MAX_TANK_TEMP_C)
			{
				// TBD warning
			}
			if (boilerExhaust_C < 110  && boilerDiff <= 0) // if chimney is cooling down and exchanger does not put heat to water
			{
				mPumpMask &= ~PUMP_BOILER;  // turn off pump
				DO_SetPumps(mPumpMask);

				mBoilerState = eBS_CoolDown;
			}
			break;
		case eBS_CoolDown:
			{
				if (boilerTemp_C < (TEMP_PUMP_ON - 1)) // wait until exchanger is cold
				{
					mBoilerState = eBs_Idle;
				}
				if (boilerTemp_C > 85 || boilerExhaust_C > 115) // if it gets warm again, turn on the pump
				{
					mPumpMask |= PUMP_BOILER;  // turn on pump
					DO_SetPumps(mPumpMask);
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

	// simples

}

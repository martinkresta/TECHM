/*
 * TEMPCON.c
 *
 *  Created on: Dec 5, 2021
 *      Author: Martin
 *      Brief: Automatic control of radiators and wall heating
 */



/*
 * Inputs:  Time of the day
 * 					Available heat energy in the tank
 * 					Indoor temperature
 * 					Outdoor temperature
 * 					State of the boiler
 * 					Requested temperature
 * */

/* Vysky cidel 	 20	, 55	, 97, 130, 163, 183/ */
 //		objemy[l]  116, 116 , 88, 94 , 40	,	0
//  kpacity[Wh/C] 133, 133, 101, 108, 46


#include "TEMPCON.h"
#include "VARS2.h"
#include "main.h"
#include "RTC.h"
#include "DO.h"
#include "COM.h"

/* Configuration */
float mDownReqTempDay;     // degC
float mDownReqTempNight;
float mUpReqTempDay;
float mUpReqTempNight;
uint16_t mDownNightBegin;
uint16_t mDownNightEnd;
uint16_t mUpNightBegin;
uint16_t mUpNightEnd;


/* Process variables */
float mAvailableEnergyWh;
float mTotalEnergyWh;
uint8_t mRadOn;
uint8_t mWallOn;
uint8_t mServoRadPct;
uint8_t mServoWallPct;
uint16_t mWallInTemp;
uint16_t mRadInTemp;
uint16_t mRadOnTime;
uint16_t mWallOnTime;
float mReqWaterRad_C;
float mReqWaterWall_C;

static sVarsTank* mpVars;                  /* produced */
static const sVarsTemperatures* mpTemps;   /* read */


static void TurnOffRadiators(void);
static void TurnOffWalls(void);

static void TurnOnRadiators(uint8_t init_valve);
static void TurnOnWalls(uint8_t init_valve);


void TC_Init(sVarsTank* pVars, const sVarsTemperatures* pTemps)
{
	mpVars = pVars;
	mpTemps = pTemps;

	mDownReqTempDay = 21.5f;
	mDownReqTempNight = 21.5f;
	mUpReqTempDay = 21.5f;
	mUpReqTempNight = 19.5f;
	mDownNightBegin = 20;
	mDownNightEnd = 5;
	mUpNightBegin = 19;
	mUpNightEnd = 6;

	mServoRadPct = 60;
	mServoWallPct = 60;
	DO_SetServoRad(mServoRadPct);
	DO_SetServoWall(mServoWallPct);

}

void TC_Update_1s(void)
{
	// collect all data (degC)
	const sVarsTemperaturesTechm* t = &mpTemps->Techm;
	float tempDown_C;
	float tempUp_C;
	float tank1_C;
	float tank2_C;
	float tank3_C;
	float tank4_C;
	float tank5_C;
	float tank6_C;
	float boilerExhaust_C;
	float wallIn_C;
	float radIn_C;
	float outside_C;

	uint8_t hour = RTC_GetTime().Hour;
	float upReqTemp;
	float downReqTemp;

	tempDown_C = mpTemps->IoboardD.Downstairs_C.value;
	wallIn_C = t->WallIn_C.value;

	tempUp_C = mpTemps->Elecon.Kidroom_C.value;
	radIn_C = t->RadHot_C.value;

	if (!(mpTemps->IoboardD.Downstairs_C.valid && t->WallIn_C.valid && mpTemps->Elecon.Kidroom_C.valid &&
	      t->RadHot_C.valid) || tempDown_C == 0.0f || tempUp_C == 0.0f)
	{
	 // pumps off
		TurnOffWalls();
		TurnOffRadiators();
		return;
	}

	// validity of the tank, exhaust and outside temperatures is not checked (as before)
	tank1_C = t->Tank1_C.value;
	tank2_C = t->Tank2_C.value;
	tank3_C = t->Tank3_C.value;
	tank4_C = t->Tank4_C.value;
	tank5_C = t->Tank5_C.value;
	tank6_C = t->Tank6_C.value;
	boilerExhaust_C = t->BoilerExhaust_C.value;
	outside_C = mpTemps->Elecon.Outside_C.value;


	// calculate available energy (heat capacity of the tank segments in Wh/degC)

	mAvailableEnergyWh = 0;
	mAvailableEnergyWh +=  ((tank1_C + tank2_C) / 2 - TANK_BASE_TEMP_C) * 133;   // tank segment 1
	mAvailableEnergyWh +=  ((tank2_C + tank3_C) / 2 - TANK_BASE_TEMP_C) * 133;  	// tank segment 2
	mAvailableEnergyWh +=  ((tank3_C + tank4_C) / 2 - TANK_BASE_TEMP_C) * 101;   // tank segment 3

	mTotalEnergyWh = mAvailableEnergyWh;
	mTotalEnergyWh +=  ((tank4_C + tank5_C) / 2 - TANK_TOTAL_BASE_TEMP_C) * 108;   // tank segment 4
	mTotalEnergyWh +=  ((tank5_C + tank6_C) / 2 - TANK_TOTAL_BASE_TEMP_C) * 46;  	// tank segment 5

	mpVars->AvailableEnergy_Wh.value = mAvailableEnergyWh;
	mpVars->TotalEnergy_Wh.value = mTotalEnergyWh;
	VARS2_SetGroupValid(mpVars, 1);


	// Set required temperature, based on actual time and daily program

	upReqTemp = mUpReqTempNight;
	downReqTemp = mDownReqTempNight;

	if (hour >= mUpNightEnd && hour < mUpNightBegin)  // Upstairs
	{
		upReqTemp = mUpReqTempDay;
	}

	if (hour >= mDownNightEnd && hour < mDownNightBegin) // downstairs
	{
		downReqTemp = mDownReqTempDay;
	}


	// Radiators on-off control
	if (mRadOn)     // radiators are on
	{
		mRadOnTime ++;
		if (mAvailableEnergyWh <= 0) //if  no energy
		{
			TurnOffRadiators();
		}
		if (tempUp_C >= upReqTemp)  // OR temperature is high enough
		{
			TurnOffRadiators();
		}
	}
	else            // radiators are off
	{
		mRadOnTime = 0;
		if (mAvailableEnergyWh >= MIN_ENERGY_RAD)  // if energy is available
		{
			if (tempUp_C <= (upReqTemp - RAD_HYST_C) + TEMP_TIE_C)  // AND it is cold
			{
				TurnOnRadiators(135 - (int16_t)tank4_C);  // initial valve position set based on tank temperature (whole degC)
			}
		}
	}


	// Walls on-off control
	if (mWallOn)     // walls are on
	{
		mWallOnTime ++;
		if (mAvailableEnergyWh <= 0) // if no energy
		{
			TurnOffWalls();
		}
		if (tempDown_C >= downReqTemp)  // OR temperature is high enough
		{
			TurnOffWalls();
		}
		if (boilerExhaust_C >= 120 && (tempDown_C >= (downReqTemp - 2.0f) - TEMP_TIE_C))  // OR boiler is on
		{
			TurnOffWalls();
		}
	}
	else            // walls are off
	{
    mWallOnTime = 0;
		if (mAvailableEnergyWh >= MIN_ENERGY_WALL)  // if energy is available
		{
			if (tempDown_C <= (downReqTemp - WALL_HYST_C) + TEMP_TIE_C)  // AND it is cold
			{
				if (boilerExhaust_C < 120 || (tempDown_C < (downReqTemp - 2.0f) - TEMP_TIE_C))   // AND boiler is off
				{
					TurnOnWalls(130 - (int16_t)tank4_C);  // initial valve position set based on tank temperature (whole degC)
				}
			}
		}
	}

	// Calculate required water temperature

	mReqWaterRad_C = 33.0f;
	if (outside_C < 0) mReqWaterRad_C -= outside_C / 2;  // compensate effect of outside temperature
	mReqWaterRad_C += (upReqTemp - tempUp_C) * 15;    // set action respective to error (15 degC water per degC room)
	if (mReqWaterRad_C > 60.0f)  mReqWaterRad_C = 60.0f;     // max 60C to radiators

	mReqWaterWall_C = 38.0f;
	mReqWaterWall_C	+= (downReqTemp - tempDown_C) * 10;   // 10 degC water per degC room
	if (mReqWaterWall_C > 48.0f)  mReqWaterWall_C = 50.0f;   // max 48C to walls (set to 50, as before)


 // Regulate the valves every VALVE_REG_PERIOD after the PUMP_ON_DELAY after pump is on

	if (mRadOnTime > PUMP_ON_DELAY && (mRadOnTime % VALVE_REG_PERIOD == 0))
	{
		if (radIn_C < mReqWaterRad_C - WATER_HYST_C - TEMP_TIE_C)  // water too cold
		{
			if (mServoRadPct < 100)
			{
				mServoRadPct ++;
				DO_SetServoRad(mServoRadPct);
			}
		}
		if (radIn_C > mReqWaterRad_C + WATER_HYST_C + TEMP_TIE_C)  // water too hot
		{
			if (mServoRadPct > 60)
			{
				mServoRadPct --;
				DO_SetServoRad(mServoRadPct);
			}
		}
	}

	if (mWallOnTime > PUMP_ON_DELAY && (mWallOnTime % VALVE_REG_PERIOD == 0))
	{
		if (wallIn_C < mReqWaterWall_C - WATER_HYST_C - TEMP_TIE_C)  // water too cold
		{
			if (mServoWallPct < 100)
			{
				mServoWallPct ++;
				DO_SetServoWall(mServoWallPct);
			}
		}
		if (wallIn_C > mReqWaterWall_C + WATER_HYST_C + TEMP_TIE_C)  // water too hot
		{
			if (mServoWallPct > 60)
			{
				mServoWallPct --;
				DO_SetServoWall(mServoWallPct);
			}
		}
	}

}


static void TurnOnRadiators(uint8_t init_valve)
{
	mServoRadPct  = 65;
	if (init_valve < 100)
	{
		mServoRadPct = init_valve;
	}
	DO_SetServoRad(mServoRadPct);  // set initial valve position
	mRadOn = 1;
	COM_SendACRemoteRequest(0,1,0xFFFF);  // keep AC On for ~ 18 hours
	DO_SetPumpRad(1);  // turn on radiators
}

static void TurnOnWalls(uint8_t init_valve)
{
	mServoWallPct  = 65;
	if (init_valve < 100)
	{
		mServoWallPct = init_valve;
	}
	DO_SetServoWall(mServoWallPct);   // set initial valve position
	mWallOn = 1;
	COM_SendACRemoteRequest(0,1,0xFFFF);  // keep AC On for ~ 18 hours
	DO_SetPumpWall(1);  // turn on walls
}


static void TurnOffRadiators(void)
{
	mRadOn = 0;
	DO_SetPumpRad(0);  // turn off radiators
	COM_SendACRemoteRequest(0,0,1);  // Cancel the remote request
	//mServoRadPct  = 65;
	//DO_SetServoRad(mServoRadPct);  // set default valve position
}

static void TurnOffWalls(void)
{
	mWallOn = 0;
	DO_SetPumpWall(0);  // turn off walls
	COM_SendACRemoteRequest(0,0,1);  // Cancel the remote request
	//mServoWallPct  = 65;
	//DO_SetServoWall(mServoWallPct);  // set default valve position
}

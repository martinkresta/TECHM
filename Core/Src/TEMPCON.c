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
#include "VARS.h"
#include "main.h"
#include "RTC.h"
#include "DO.h"

/* Configuration */
uint16_t mDownReqTempDay;
uint16_t mDownReqTempNight;
uint16_t mUpReqTempDay;
uint16_t mUpReqTempNight;
uint16_t mDownNightBegin;
uint16_t mDownNightEnd;
uint16_t mUpNightBegin;
uint16_t mUpNightEnd;


/* Process variables */
int16_t mAvailableEnergyWh;
int16_t mTotalEnergyWh;
uint8_t mRadOn;
uint8_t mWallOn;
uint8_t mServoRadPct;
uint8_t mServoWallPct;
uint16_t mWallInTemp;
uint16_t mRadInTemp;
uint16_t mRadOnTime;
uint16_t mWallOnTime;
uint16_t mReqWaterRad_C10;
uint16_t mReqWaterWall_C10;


static void TurnOffRadiators(void);
static void TurnOffWalls(void);

static void TurnOnRadiators(uint8_t init_valve);
static void TurnOnWalls(uint8_t init_valve);


void TC_Init(void)
{

	mDownReqTempDay = 221;
	mDownReqTempNight = 210;
	mUpReqTempDay = 221;
	mUpReqTempNight = 200;
	mDownNightBegin = 20;
	mDownNightEnd = 4;
	mUpNightBegin = 18;
	mUpNightEnd = 4;

	mServoRadPct = 60;
	mServoWallPct = 60;
	DO_SetServoRad(mServoRadPct);
	DO_SetServoWall(mServoWallPct);

}

void TC_Update_1s(void)
{

	// collect all data

	uint16_t invalid;
	int16_t tempDown_C10;
	int16_t tempUp_C10;
	int16_t tank1_C;
	int16_t tank2_C;
	int16_t tank3_C;
	int16_t tank4_C;
	int16_t tank5_C;
	int16_t tank6_C;
	int16_t boilerExhaust_C;
	int16_t wallIn_C10;
	int16_t radIn_C10;
	int16_t outside_C10;

	uint8_t hour = RTC_GetTime().Hour;
	uint16_t upReqTemp;
	uint16_t downReqTemp;

	invalid = 0;

	tempDown_C10 = VAR_GetVariable(VAR_TEMP_DOWNSTAIRS,&invalid);
	wallIn_C10 = VAR_GetVariable(VAR_TEMP_WALL_IN,&invalid);

	tempUp_C10 = VAR_GetVariable(VAR_TEMP_KIDROOM, &invalid);
	radIn_C10 = VAR_GetVariable(VAR_TEMP_RAD_H,&invalid);

	if (invalid || tempDown_C10 == 0 || tempUp_C10 == 0)
	{
	//  pumps off
	//	TurnOffWalls();
	//	TurnOffRadiators();
		return;
	}

	tank1_C = VAR_GetVariable(VAR_TEMP_TANK_1,&invalid);
	tank2_C = VAR_GetVariable(VAR_TEMP_TANK_2,&invalid);
	tank3_C = VAR_GetVariable(VAR_TEMP_TANK_3,&invalid);
	tank4_C = VAR_GetVariable(VAR_TEMP_TANK_4,&invalid);
	tank5_C = VAR_GetVariable(VAR_TEMP_TANK_5,&invalid);
	tank6_C = VAR_GetVariable(VAR_TEMP_TANK_6,&invalid);
	boilerExhaust_C = VAR_GetVariable(VAR_TEMP_BOILER_EXHAUST,&invalid)/10;
	outside_C10 = VAR_GetVariable(VAR_TEMP_OUTSIDE,&invalid);


	// calculate available energy

	mAvailableEnergyWh = 0;
	mAvailableEnergyWh +=  (((tank1_C + tank2_C) / 2 - TANK_BASE_TEMP) *  133)/10;   // tank segment 1
	mAvailableEnergyWh +=  (((tank2_C + tank3_C) / 2 - TANK_BASE_TEMP) *  133)/10;  	// tank segment 2
	mAvailableEnergyWh +=  (((tank3_C + tank4_C) / 2 - TANK_BASE_TEMP) *  101)/10;   // tank segment 3

	mTotalEnergyWh = mAvailableEnergyWh;
	mTotalEnergyWh +=  (((tank4_C + tank5_C) / 2 - 400) *  108)/10;   // tank segment 4
	mTotalEnergyWh +=  (((tank5_C + tank6_C) / 2 - 400) *  46)/10;  	// tank segment 5

	VAR_SetVariable(VAR_HEAT_HEATING_WH, mAvailableEnergyWh, 1);
	VAR_SetVariable(VAR_HEAT_TOTAL_WH, mTotalEnergyWh, 1);


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
		if (tempUp_C10 >= upReqTemp)  // OR temperature is high enough
		{
			TurnOffRadiators();
		}
	}
	else            // radiators are off
	{
		mRadOnTime = 0;
		if (mAvailableEnergyWh >= MIN_ENERGY_RAD)  // if energy is available
		{
			if (tempUp_C10 <= (upReqTemp - RAD_HYST_C10))  // AND it is cold
			{
				TurnOnRadiators(135 - tank4_C/10);  // initial valve position set based on tank temperature
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
		if (tempDown_C10 >= downReqTemp)  // OR temperature is high enough
		{
			TurnOffWalls();
		}
		if (boilerExhaust_C >= 75)  // OR boiler is on
		{
			TurnOffWalls();
		}
	}
	else            // walls are off
	{
		mWallOnTime = 0;
		if (mAvailableEnergyWh >= MIN_ENERGY_WALL)  // if energy is available
		{
			if (tempDown_C10 <= (downReqTemp - WALL_HYST_C10))  // AND it is cold
			{
				if (boilerExhaust_C < 70)   // AND boiler is off
				{
					TurnOnWalls(130 - tank4_C/10);  // initial valve position set based on tank temperature
				}
			}
		}
	}

	// Calculate required water temperature

	mReqWaterRad_C10 = 330;
	if (outside_C10 < 0) mReqWaterRad_C10 -= outside_C10/2;  // compensate effect of outside temperature
	mReqWaterRad_C10 += (upReqTemp - tempUp_C10) * 15;    // set action respective to error
	if (mReqWaterRad_C10 > 600)  mReqWaterRad_C10 = 600;     // max 60C to radiators

	mReqWaterWall_C10 = 380;
	//if (outside_C10 < 0) mReqWaterWall_C10 -= outside_C10;  // compensate effect of outside temperature
	mReqWaterWall_C10	+= (downReqTemp - tempDown_C10) * 10;
	if (mReqWaterWall_C10 > 480)  mReqWaterWall_C10 = 500;   // max 48C to walls


 // Regulate the valves every VALVE_REG_PERIOD after the PUMP_ON_DELAY after pump is on

	if (mRadOnTime > PUMP_ON_DELAY && (mRadOnTime % VALVE_REG_PERIOD == 0))
	{
		if (radIn_C10 < mReqWaterRad_C10 - WATER_HYST)  // water too cold
		{
			if (mServoRadPct < 100)
			{
				mServoRadPct ++;
				DO_SetServoRad(mServoRadPct);
			}
		}
		if (radIn_C10 > mReqWaterRad_C10 + WATER_HYST)  // water too hot
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
		if (wallIn_C10 < mReqWaterWall_C10 - WATER_HYST)  // water too cold
		{
			if (mServoWallPct < 100)
			{
				mServoWallPct ++;
				DO_SetServoWall(mServoWallPct);
			}
		}
		if (wallIn_C10 > mReqWaterWall_C10 + WATER_HYST)  // water too hot
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
	DO_SetPumpWall(1);  // turn on walls
}


static void TurnOffRadiators(void)
{
	mRadOn = 0;
	DO_SetPumpRad(0);  // turn off radiators
	//mServoRadPct  = 65;
	//DO_SetServoRad(mServoRadPct);  // set default valve position
}

static void TurnOffWalls(void)
{
	mWallOn = 0;
	DO_SetPumpWall(0);  // turn off walls
	//mServoWallPct  = 65;
	//DO_SetServoWall(mServoWallPct);  // set default valve position
}

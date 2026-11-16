/*
 * TEMPCON.h
 *
 *  Created on: Dec 5, 2021
 *      Author: Martin
 *       Automatic control of radiators and wall heating
 */

#ifndef INC_TEMPCON_H_
#define INC_TEMPCON_H_

#include "uha_vars_gen.h"   /* Vars2 group types */


#endif /* INC_TEMPCON_H_ */


#define TANK_BASE_TEMP_C	30.0f  // minimal usable temperature (heating energy)
#define TANK_TOTAL_BASE_TEMP_C	40.0f  // base temperature of the lower tank segments (total energy)
#define MIN_ENERGY_RAD	300			// minimal available energy to start radiators
#define MIN_ENERGY_WALL	800  // minimal available energy to start walls

#define RAD_HYST_C		0.2f   // degC
#define WALL_HYST_C		0.2f   // degC

#define PUMP_ON_DELAY		40   	// 40 seconds
#define VALVE_REG_PERIOD	30	// regulation period

#define WATER_HYST_C				1.0f   // degC
#define TEMP_TIE_C				0.01f  // sensor values are 0.1 degC steps: a tie with a computed threshold counts as before

/* produced: pVars (Vars2.Tank); read: pTemps (tank / water temperatures, rooms, outside) */
void TC_Init(sVarsTank* pVars, const sVarsTemperatures* pTemps);

void TC_Update_1s(void);

/* read only from outside (APP.c: VARS2 states). Setpoints, set in TC_Init() */
extern float mDownReqTempDay;
extern float mDownReqTempNight;
extern float mUpReqTempDay;
extern float mUpReqTempNight;
/* process values of the last TC_Update_1s() */
extern float mUpReqTemp_C;      // room request in effect (day or night value), upstairs
extern float mDownReqTemp_C;    // downstairs
extern uint8_t mRadOn;
extern uint8_t mWallOn;
extern float mReqWaterRad_C;
extern float mReqWaterWall_C;

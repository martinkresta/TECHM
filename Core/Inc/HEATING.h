/*
 * HEATING.h
 *
 *  Created on: Aug 29, 2021
 *      Author: Martin
 *      Brief: Control logic of home heating system and solid fuel boiler
 */

#ifndef INC_HEATING_H_
#define INC_HEATING_H_

#include "main.h"
#include "uha_vars_gen.h"   /* Vars2 group types */

typedef enum
{
	eBs_Idle,
	eBs_HeatUp,
	eBS_Heating,
	eBS_CoolDown,
	eBS_Overheat,
	eBS_Error,
	eBS_InvalidInputs
}eBoilerState;


typedef enum
{
	eBe_NoError,
	eBe_OverheatedTank,
	eBe_OverheatedBoiler,
	eBe_PumpFailure
}eBoilerError;

#define  TEMP_PUMP_ON							60
#define  TEMP_PUMP_OFF						50

#define  TEMP_BOILER_OVERHEAT			86
#define  MIXVALVE_ERR_TEMP				80

#define  MAX_TANK_TEMP_C					95
#define  MIN_SOC									10

#define MIN_PUMP_ON_TIME          60
#define BOILER_W_PER_C            920.0f   // heating power per degC of boiler out - in (92 W per 0.1 degC)


/* produced: pVars (Vars2.Boiler); read: pTemps (boiler, tank, exhaust temperatures) */
void HC_Init(sVarsBoiler* pVars, const sVarsTemperaturesTechm* pTemps);

void HC_Update_1s(void);

uint16_t HC_GetStatus(void);

void HC_DoorOpened(void);
void HC_DoorClosed(void);

void HC_Midnight(void);


#endif /* INC_HEATING_H_ */

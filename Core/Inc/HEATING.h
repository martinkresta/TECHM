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

#define  TEMP_PUMP_ON							40
#define  TEMP_PUMP_OFF						40

#define  TEMP_BOILER_OVERHEAT			95
#define  MIXVALVE_ERR_TEMP				80

#define  MAX_TANK_TEMP_C					95
#define  MIN_SOC									10


void HC_Init(void);

void HC_Update_1s(void);

uint16_t HC_GetStatus(void);


#endif /* INC_HEATING_H_ */

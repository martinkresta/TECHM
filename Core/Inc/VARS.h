/*
 * VARS.h
 *
 *  Created on: Jul 13, 2021
 *      Author: Martin
 */

#ifndef INC_VARS_H_
#define INC_VARS_H_

#include "main.h"


#define  VAR_NETWORK_STATUS  1


#define  VAR_BAT_SOC  10
#define  VAR_BAT_VOLTAGE_V10  11
#define  VAR_LOAD_A10  12
#define  VAR_CHARGING_A10  13

#define  VAR_TEMP_BOILER  100
#define  VAR_TEMP_BOILER_IN  101
#define  VAR_TEMP_BOILER_OUT  102
#define  VAR_TEMP_TANK_IN_H  103
#define  VAR_TEMP_TANK_OUT_H  104
#define  VAR_TEMP_TANK_1  105
#define  VAR_TEMP_TANK_2  106
#define  VAR_TEMP_TANK_3  107
#define  VAR_TEMP_TANK_4  108
#define  VAR_TEMP_TANK_5  109
#define  VAR_TEMP_TANK_6  110
#define  VAR_TEMP_WALL_IN  111
#define  VAR_TEMP_WALL_OUT  112
#define  VAR_TEMP_BOILER_EXHAUST  113
#define  VAR_TEMP_RAD_H  114
#define  VAR_TEMP_RAD_C  115
#define  VAR_TEMP_TANK_IN_C  116
#define  VAR_TEMP_TANK_OUT_C  117


#define  VAR_TEMP_TECHM_BOARD			120
#define  VAR_TEMP_IOBOARD_D				121
#define  VAR_TEMP_IOBOARD_U				122
#define  VAR_TEMP_ELECON_BOARD		123

#define  VAR_TEMP_DOWNSTAIRS			124

#define  VAR_EL_HEATER_STATUS  	80
#define  VAR_EL_HEATER_POWER  	81




#define NUM_OF_VARIABLES		255


#define INVALID_FLAG				0x8000

typedef struct
{
	uint8_t valid;
	uint8_t local;
	int16_t value;
} sVar;


void VAR_Init(void);

int16_t VAR_GetVariable(uint16_t varId, uint16_t* invalid);
int16_t* VAR_GetVariablePointer(uint8_t varId, uint16_t* invalid);
void VAR_SetVariable(uint8_t varId, int16_t value, uint8_t valid);

#endif /* INC_VARS_H_ */

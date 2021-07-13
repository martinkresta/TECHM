/*
 * VARS.h
 *
 *  Created on: Jul 13, 2021
 *      Author: Martin
 */

#ifndef INC_VARS_H_
#define INC_VARS_H_

#include "main.h"


#define  VAR_TEMP_BOILER  100
#define  VAR_TEMP_BOILER_IN  101
#define  VAR_TEMP_BOILER_OUT  102
#define  VAR_TEMP_TANK_IN  103
#define  VAR_TEMP_TANK_OUT  104
#define  VAR_TEMP_TANK_1  105
#define  VAR_TEMP_TANK_2  106
#define  VAR_TEMP_TANK_3  107
#define  VAR_TEMP_TANK_4  108
#define  VAR_TEMP_TANK_5  109
#define  VAR_TEMP_TANK_6  110
#define  VAR_TEMP_WALL_IN  111
#define  VAR_TEMP_WALL_OUT  112
#define  VAR_TEMP_BOILER_EXHAUST  113


int16_t VAR_GetVariable(uint8_t varId);
int16_t* VAR_GetVariablePointer(uint8_t varId);
void VAR_SetVariablePointer(uint8_t varId, int16_t* ptr);

#endif /* INC_VARS_H_ */

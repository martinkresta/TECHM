/*
 * TEMPCON.h
 *
 *  Created on: Dec 5, 2021
 *      Author: Martin
 *       Automatic control of radiators and wall heating
 */

#ifndef INC_TEMPCON_H_
#define INC_TEMPCON_H_


#endif /* INC_TEMPCON_H_ */


#define TANK_BASE_TEMP	30    // minimal usable temperature
#define MIN_ENERGY_RAD	0			// minimal available energy to start radiators
#define MIN_ENERGY_WALL	5000  // minimal available energy to start walls

#define RAD_HYST_C10		2
#define WALL_HYST_C10		2

#define PUMP_ON_DELAY		10   	// 30 seconds
#define VALVE_REG_PERIOD	30	// regulation period

#define RAD_WATER_TEMP_C10	500
#define WALL_WATER_TEMP_C10	450
#define WATER_HYST					15

void TC_Init(void);

void TC_Update_1s(void);

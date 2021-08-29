/*
 * DO.h
 *
 *  Created on: 15. 7. 2021
 *      Author: Martin
 */

#include "main.h"

#ifndef INC_DO_H_
#define INC_DO_H_


#define TIM_SERVOS		&htim1
#define CHANNEL_RAD		TIM_CHANNEL_2
#define CHANNEL_WALL	TIM_CHANNEL_1

#define PUMP_BOILER			0x01
#define PUMP_WALL				0x02
#define PUMP_RAD				0x04


void DO_Init();

void	DO_SetElHeaters(uint8_t heaterMask);

void  DO_SetPumps(uint8_t pumps);

void 	DO_SetElv(uint8_t elv);



void  DO_SetServoRad(int16_t pct);

void  DO_SetServoWall(int16_t pct);

#endif /* INC_DO_H_ */

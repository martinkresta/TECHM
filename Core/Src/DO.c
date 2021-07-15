/*
 * DO.c
 *
 *  Created on: 15. 7. 2021
 *      Author: Martin
 */

#include "main.h"
#include "do.h"


void	DO_SetOutput(uint8_t output, uint8_t value);

void  DO_SetPumps(uint8_t pumps)
{
	if (pumps & 0x01)  // boiler
	{
		HAL_GPIO_WritePin(PUMP_BOIL_GPIO_Port, PUMP_BOIL_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(PUMP_BOIL_GPIO_Port, PUMP_BOIL_Pin, GPIO_PIN_RESET);
	}
	if (pumps & 0x02)  // wall
	{
		HAL_GPIO_WritePin(PUMP_WALL_GPIO_Port, PUMP_WALL_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(PUMP_WALL_GPIO_Port, PUMP_WALL_Pin, GPIO_PIN_RESET);
	}
	if (pumps & 0x04)  // radiators
	{
		HAL_GPIO_WritePin(PUMP_RAD_GPIO_Port, PUMP_RAD_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(PUMP_RAD_GPIO_Port, PUMP_RAD_Pin, GPIO_PIN_RESET);
	}
}


void 	DO_SetElv(uint8_t elv)
{
	if (elv != 0)
	{
		 HAL_GPIO_WritePin(ELV_GPIO_Port, ELV_Pin, GPIO_PIN_SET);  // open the electronic valve
	}
	else
	{
		HAL_GPIO_WritePin(ELV_GPIO_Port, ELV_Pin, GPIO_PIN_RESET);  // open the electronic valve
	}
}

/*
 * DO.c
 *
 *  Created on: 15. 7. 2021
 *      Author: Martin
 */

#include "main.h"
#include "do.h"


void	DO_SetElHeaters(uint8_t heaters)
{

	// lower elements
	if (heaters & 0x01)
	{
		HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_RESET);
	}

	if (heaters & 0x02)
	{
		HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_RESET);
	}

	if (heaters & 0x04)
	{
		HAL_GPIO_WritePin(OUT3_GPIO_Port, OUT3_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(OUT3_GPIO_Port, OUT3_Pin, GPIO_PIN_RESET);
	}

	// upper elements
	if (heaters & 0x08)
	{
		HAL_GPIO_WritePin(OUT4_GPIO_Port, OUT4_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(OUT4_GPIO_Port, OUT4_Pin, GPIO_PIN_RESET);
	}

	if (heaters & 0x10)
	{
		HAL_GPIO_WritePin(OUT5_GPIO_Port, OUT5_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(OUT5_GPIO_Port, OUT5_Pin, GPIO_PIN_RESET);
	}

	if (heaters & 0x20)
	{
		HAL_GPIO_WritePin(OUT6_GPIO_Port, OUT6_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(OUT6_GPIO_Port, OUT6_Pin, GPIO_PIN_RESET);
	}



}

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
		HAL_GPIO_WritePin(ELV_GPIO_Port, ELV_Pin, GPIO_PIN_RESET);  // close the electronic valve
	}
}

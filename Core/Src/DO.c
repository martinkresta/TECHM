/*
 * DO.c
 *
 *  Created on: 15. 7. 2021
 *      Author: Martin
 */

#include "main.h"
#include "do.h"

static TIM_OC_InitTypeDef sConfigOC;

static TIM_HandleTypeDef * htim;

void DO_Init()
{
	// prepare structure for configuring OC channels
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

}

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

/*void  DO_SetPumps(uint8_t pumps)
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
}*/


void  DO_SetPumpBoiler(uint8_t state)
{
	if (state)
	{
		HAL_GPIO_WritePin(PUMP_BOIL_GPIO_Port, PUMP_BOIL_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(PUMP_BOIL_GPIO_Port, PUMP_BOIL_Pin, GPIO_PIN_RESET);
	}
}

void  DO_SetPumpWall(uint8_t state)
{
	if (state)  // wall
	{
		HAL_GPIO_WritePin(PUMP_WALL_GPIO_Port, PUMP_WALL_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(PUMP_WALL_GPIO_Port, PUMP_WALL_Pin, GPIO_PIN_RESET);
	}
}


void  DO_SetPumpRad(uint8_t state)
{
	if (state)  // radiators
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


void  DO_SetServoRad(int16_t pct)
{

	uint32_t dutycycle = 0;
	if (GPIO_PIN_RESET == HAL_GPIO_ReadPin(PG_48V_GPIO_Port,PG_48V_Pin))
	{
		// not available power suuply for servo valves
//		HAL_TIM_PWM_Stop(TIM_SERVOS, CHANNEL_RAD);
	//	return;
	}
	if (pct < 0) pct = 0;
	if (pct > 100) pct = 100;
	dutycycle = (pct * 940)  / 100;  // for analog voltage output 10.0V is needed PWM dutycycle 94%

	sConfigOC.Pulse = dutycycle;

	HAL_TIM_PWM_Stop(TIM_SERVOS, CHANNEL_RAD); // we have to stop here

	if (HAL_TIM_PWM_ConfigChannel(TIM_SERVOS, &sConfigOC, CHANNEL_RAD) != HAL_OK)  // because this fcn will stop it in register CCER - CCxE
	{
		//TBD_Beep
		Error_Handler();
	}
	HAL_TIM_PWM_Start(TIM_SERVOS, CHANNEL_RAD);  // and this fcn is then not able to start  (HAL HELL ! )
}


void  DO_SetServoWall(int16_t pct)
{

	uint32_t dutycycle = 0;
	if (GPIO_PIN_RESET == HAL_GPIO_ReadPin(PG_48V_GPIO_Port,PG_48V_Pin))
	{
		// not available power suuply for servo valves
//		HAL_TIM_PWM_Stop(TIM_SERVOS, CHANNEL_WALL);
//		return;
	}
	if (pct < 0) pct = 0;
	if (pct > 100) pct = 100;
	dutycycle = (pct * 940)  / 100;  // for analog voltage output 10.0V is needed PWM dutycycle 94%

	sConfigOC.Pulse = dutycycle;

	HAL_TIM_PWM_Stop(TIM_SERVOS, CHANNEL_WALL); // we have to stop here

	if (HAL_TIM_PWM_ConfigChannel(TIM_SERVOS, &sConfigOC, CHANNEL_WALL) != HAL_OK)  // because this fcn will stop it in register CCER - CCxE
	{
		Error_Handler();
	}
	HAL_TIM_PWM_Start(TIM_SERVOS, CHANNEL_WALL);  // and this fcn is then not able to start  (HAL HELL ! )
}



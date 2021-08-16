/*
 * ELHEATER.h
 *
 *  Created on: Aug 15, 2021
 *      Author: Martin
 *      Brief: Control logic of electrical heating elements in accumulation tank
 */

#ifndef INC_ELHEATER_H_
#define INC_ELHEATER_H_

typedef enum
{
	eElh_Heating,
	eELh_TempReached,
	eElh_NoFreePower,
	eElh_LowSOC,
	eElh_48VFail,
	eElh_TankOvertemperature,
	eElh_BoardOveremerature,
	eElh_InvalidInputs,
	eElh_SwitchError
}eElhState;

#define  MAX_BOARD_TEMP_C					40
#define  MAX_TANK_TEMP_C					70
#define  MIN_SOC									90
#define  MAX_LOAD_A								20  //

#define  ONE_COIL_LOAD_A					10

#define  INCREASE_PERIOD_S				10


void ELH_Init(void);

void ELH_Update_1s(void);

uint16_t ELH_GetStatus(void);

void ELH_SetTemp(int16_t tempTop, int16_t tempMiddle);

#endif /* INC_ELHEATER_H_ */

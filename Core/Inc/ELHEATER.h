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
	eElh_ELeconComError,
	eElh_SwitchError
}eElhState;

#define  MAX_BOARD_TEMP_C					55
#define  MIN_UTIL_TEMP_C					45
//#define  MIN_SOC									90
#define  MAX_LOAD_A								35  //
#define  SOC_ENABLE								100
#define  SOC_DISABLE							90
//#define  DEF_ENABLE_MASK					0x3E    // 0011 1110   // five uppest coils enabled
#define  DEF_ENABLE_MASK					0x3F    // 0011 1111   // all coils enabled

#define  ONE_COIL_LOAD_A					10
#define  NUM_OF_COILS							6

#define  INCREASE_PERIOD_S				10


void ELH_Init(void);

void ELH_Update_1s(void);

uint16_t ELH_GetStatus(void);

void ELH_SetTemp(int16_t tempTop, int16_t tempMiddle);

#endif /* INC_ELHEATER_H_ */

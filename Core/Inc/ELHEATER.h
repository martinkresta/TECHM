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
	eElh_SwitchError,
	eElh_BalanceSupport,
	eElh_ComfortHeat
}eElhState;

#define  MAX_BOARD_TEMP_C					65
#define  MIN_UTIL_TEMP_C					55
//#define  MIN_SOC									90
#define  MAX_LOAD_A								95  //
#define  SOC_ENABLE								100
#define  SOC_DISABLE							95
#define  DEF_ENABLE_MASK					0x3F    // 0011 1111   // all coils enabled

#define  ONE_COIL_LOAD_A					10
#define  NUM_OF_COILS							6

#define  INCREASE_PERIOD_S				7

#define  WINTER_REQ_TEMP				90
#define  SUMMER_REQ_TEMP				70

#define  COMFORT_SOC_ENABLE				43   // summer comfort heat: enable above this SOC
#define  COMFORT_SOC_DISABLE			40   // summer comfort heat: disable below this SOC
#define  COMFORT_REQ_TEMP				55   // summer comfort heat: target tank temperature
#define  COMFORT_COIL_MASK				0x38 // summer comfort heat: top 3 coils only (bits 3,4,5)


void ELH_Init(void);

void ELH_Update_1s(void);

uint16_t ELH_GetStatus(void);

void ELH_SetTemp(int16_t tempTop, int16_t tempMiddle);

void ELH_Midnight(void);

void ELH_SetBalanceInfo(uint8_t balancedToday, int16_t optBalancingCurrent);

#endif /* INC_ELHEATER_H_ */

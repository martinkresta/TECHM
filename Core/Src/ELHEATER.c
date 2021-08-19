/*
 * ELHEATER.c
 *
 *  Created on: Aug 15, 2021
 *      Author: Martin
 *       *      Brief: Control logic of electrical heating elements in accumulation tank
 */

#include "main.h"
#include "ELHEATER.h"
#include "COM.h"
#include "VARS.h"
#include "DO.h"
#include "ADC.h"


// module variables

uint16_t mHeaterMask = 0;
eElhState mState;
static int16_t mHeaterLoad_A;
int16_t mReqTankTemp;
uint16_t mIncreaseRequest_cnt;
uint16_t mDecreaseRequest_cnt;

uint16_t mHeaterCurrents_mA[NUM_OF_COILS];
uint16_t mOveralCurrent_10mA;



/* Private methods declarations */

void IncreasePower(void);
void DecreasePower(void);
void SwitchOffImmediatelly(void);
void CalculateHeaterLoad(void);
int16_t ConvertAdcToCurrent_mA(uint16_t adcval);


/*Global methods*/

void ELH_Init(void)
{
	mReqTankTemp = 50;
	mHeaterMask = 0;
	mState = eElh_NoFreePower;
	mHeaterLoad_A = 0;
	mIncreaseRequest_cnt= 0;
	mDecreaseRequest_cnt= 0;

}

void ELH_Update_1s(void)
{

	int16_t invalid;
	int16_t boartTemp_C;
	int16_t tankTemp_C;
	int16_t soc;
	int16_t charging_A;
	int16_t load_A;


	// measure the heater currents
	mHeaterCurrents_mA[0] = ConvertAdcToCurrent_mA( ADC_GetValue(ADC_CHANNEL_HEATER_1));
	mHeaterCurrents_mA[1] = ConvertAdcToCurrent_mA( ADC_GetValue(ADC_CHANNEL_HEATER_2));
	mHeaterCurrents_mA[2] = ConvertAdcToCurrent_mA( ADC_GetValue(ADC_CHANNEL_HEATER_3));
	mHeaterCurrents_mA[3] = ConvertAdcToCurrent_mA( ADC_GetValue(ADC_CHANNEL_HEATER_4));
	mHeaterCurrents_mA[4] = ConvertAdcToCurrent_mA( ADC_GetValue(ADC_CHANNEL_HEATER_5));
	mHeaterCurrents_mA[5] = ConvertAdcToCurrent_mA( ADC_GetValue(ADC_CHANNEL_HEATER_6));

	// insert actual variables to VARS module
	CalculateHeaterLoad();
	VAR_SetVariable(VAR_EL_HEATER_STATUS, mState, 1);
	VAR_SetVariable(VAR_EL_HEATER_POWER, mHeaterLoad_A, 1);
	VAR_SetVariable(VAR_EL_HEATER_CURRENT, mOveralCurrent_10mA, 1);


// collect the all informations to make decision about the power

// Over temperature input (emergency thermostate)
	if (GPIO_PIN_SET == HAL_GPIO_ReadPin(ETS_GPIO_Port,ETS_Pin))
	{ // overtemperature
		mState = eElh_TankOvertemperature;
		SwitchOffImmediatelly();
		return;
	}

// 48V Supply availability
	if (GPIO_PIN_RESET == HAL_GPIO_ReadPin(PG_48V_GPIO_Port,PG_48V_Pin))
	{ // 48V not available
		mState = eElh_48VFail;
		SwitchOffImmediatelly();
		return;
	}

	// collect the variables
	invalid = 0;
	boartTemp_C= VAR_GetVariable(VAR_TEMP_TECHM_BOARD,&invalid)/10;  // techm board temperature
	tankTemp_C = VAR_GetVariable(VAR_TEMP_TANK_6,&invalid)/10;  // top tank sensor
	soc				 = VAR_GetVariable(VAR_BAT_SOC,&invalid);  		// battery soc
	charging_A = VAR_GetVariable(VAR_CHARGING_A10,&invalid)/10;  // charging current
	load_A		 = VAR_GetVariable(VAR_LOAD_A10,&invalid)/10;  // load current
	if(invalid)
	{
		mState = eElh_InvalidInputs;
		SwitchOffImmediatelly();
		return;
	}
	// TECHM Board temperature
	if (boartTemp_C > MAX_BOARD_TEMP_C)
		{
			mState = eElh_BoardOveremerature;
			SwitchOffImmediatelly();
			return;
		}
// Actual tank temperature
	if (tankTemp_C > mReqTankTemp)
	{
		mState = eELh_TempReached;
		SwitchOffImmediatelly();
		return;
	}

// Battery SOC
	if (soc <  MIN_SOC)
	{
		mState = eElh_LowSOC;
		SwitchOffImmediatelly();
		return;
	}
// Solar current
	if (charging_A < ONE_COIL_LOAD_A && soc < 95)
	{
		// turn off if sun power is too low
		mState = eElh_NoFreePower;
		SwitchOffImmediatelly();
		return;
	}
// Battery actual load
	if (((load_A - charging_A) + ONE_COIL_LOAD_A) < MAX_LOAD_A )
	{
		mIncreaseRequest_cnt ++;
		mState = eElh_Heating;
	}
	else if ((load_A - charging_A) < MAX_LOAD_A )
	{
		// do nothig, this is the sweet spot we want to reach
	}
	else
	{
		mDecreaseRequest_cnt ++;

		if (mHeaterLoad_A == 0)
		{
			mState = eElh_NoFreePower;
		}
	}

	// increase power only every X seconds, to prevent switching all switches simultaneously
	if (mIncreaseRequest_cnt > INCREASE_PERIOD_S)
	{
		mIncreaseRequest_cnt = 0;
		IncreasePower();
		mState = eElh_Heating;
	}

	if (mDecreaseRequest_cnt > INCREASE_PERIOD_S)
	{
		mDecreaseRequest_cnt = 0;
		DecreasePower();
		mState = eElh_Heating;
		if (mHeaterLoad_A == 0)
		{
			mState = eElh_NoFreePower;
		}
	}

}

uint16_t ELH_GetStatus(void);

void ELH_SetTemp(int16_t tempTop, int16_t tempMiddle)
{
	mReqTankTemp = tempTop;
	// TBD
}



/*Private methods */
void IncreasePower(void)
{
	mHeaterMask |= 0x40;
	mHeaterMask = mHeaterMask >> 1;
	DO_SetElHeaters(mHeaterMask);
	CalculateHeaterLoad();
}

void DecreasePower(void)
{
	mHeaterMask = mHeaterMask << 1;
	DO_SetElHeaters(mHeaterMask);
	CalculateHeaterLoad();
}


void SwitchOffImmediatelly(void)
{
	mHeaterMask = 0;
	DO_SetElHeaters(mHeaterMask);
	mHeaterLoad_A = 0;
}

// calculate actual heaters load
void CalculateHeaterLoad(void)
{
	int16_t numOfHeatingElements = 0;
	uint32_t overallCurrent_mA = 0;
	uint8_t i;
	for (i = 0; i < 6; i++)
	{
		if (mHeaterMask & (0x01 << i))
		{
			numOfHeatingElements++;
		}
		overallCurrent_mA += mHeaterCurrents_mA[i];
	}
	mHeaterLoad_A = numOfHeatingElements * ONE_COIL_LOAD_A;
	mOveralCurrent_10mA = (int16_t)(overallCurrent_mA/10);
}


inline int16_t ConvertAdcToCurrent_mA(uint16_t adcval)
{
	int16_t current = 0;
	uint32_t sense_mv = ADC_VREF_MV/4096.0 * adcval;
	current = (int16_t)((sense_mv * 1000) / 84);
	return current;
}

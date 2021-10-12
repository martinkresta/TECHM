// Module: Temperature measurement module working above OW library
// Author: Martin Kresta
// Date: 23.22021

// Brief: The module contains the ROM codes of available temperature sensors.
// Aplication has to define witch sensors are connected, and should be measured

/* Dependencies */
#include "TEMP.h"
#include "OW.h"
#include "main.h"
#include "VARS.h"
#include "ADC.h"
#include "leds.h"

#include <math.h>



/* Private variables */
uint8_t mNumOfAssignedSensors;
sTemp mSensors[NUM_OF_ALL_SENSORS];
uint16_t mTimer;
uint8_t mReadId;
uint8_t mConvertId;
uint8_t mNumOfBuses;

int16_t mPtcTemp;

void AssignSensor(uint8_t sensorId, uint8_t varId, uint8_t busId);
void ConvertPtc(void);

// temp sensors ROM codes (see TempSensIDs.xlsx)
uint8_t mSensorsAddress[NUM_OF_ALL_SENSORS][8] =   // LSB on the left, transmit LSB first!!
{
		{0x28,  0xFF,  0x2A,  0x19,  0xB2,  0x15, 0x04,  0x27},                      //T1
		{0x28,  0xFF,  0xC5,  0x6D,  0xB2,  0x15, 0x03,  0xB7},                      //T2
		{0x28,  0xFF,  0xD3,  0x03,  0xB2,  0x15, 0x01,  0x6E},                      //T3
		{0x28,  0xFF,  0x51,  0x74,  0xB2,  0x15, 0x03,  0xB4},                      //T4
		{0x28,  0xFF,  0x00,  0x94,  0xB2,  0x15, 0x03,  0xA5},                      //T5
		{0x28,  0xFF,  0xE4,  0x02,  0xB2,  0x15, 0x01,  0x34},                      //T6
		{0x28,  0xFF,  0x74,  0x2F,  0xB2,  0x15, 0x01,  0x6E},                      //T7
		{0x28,  0xFF,  0x5A,  0x9A,  0xB2,  0x15, 0x01,  0x24},                      //T8
		{0x28,  0xFF,  0xB5,  0x82,  0xB2,  0x15, 0x03,  0x09},                      //T9
		{0x28,  0xFF,  0x62,  0x1D,  0xB2,  0x15, 0x01,  0xC1},                      //T10
		{0x28,  0xC3,  0x04,  0x56,  0xB5,  0x01, 0x3C,  0x46},                      //T101
		{0x28,  0x76,  0xAE,  0x56,  0xB5,  0x01, 0x3C,  0x24},                      //T102
		{0x28,  0xB5,  0x1B,  0x56,  0xB5,  0x01, 0x3C,  0x7F},                      //T103
		{0x28,  0x0D,  0x88,  0x56,  0xB5,  0x01, 0x3C,  0x20},                      //T104
		{0x28,  0xBE,  0xDA,  0x56,  0xB5,  0x01, 0x3C,  0x68},                      //T105
		{0x28,  0x33,  0xCB,  0x56,  0xB5,  0x01, 0x3C,  0x79},                      //T106
		{0x28,  0xC2,  0x1B,  0x56,  0xB5,  0x01, 0x3C,  0x62},                      //T107
		{0x28,  0xF4,  0xA2,  0x56,  0xB5,  0x01, 0x3C,  0x81},                      //T108
		{0x28,  0xB2,  0x25,  0x56,  0xB5,  0x01, 0x3C,  0xDC},                      //T109
		{0x28,  0x15,  0xF8,  0x56,  0xB5,  0x01, 0x3C,  0xB7},                      //T110
		{0x28,  0x59,  0xCB,  0x46,  0x3A,  0x19, 0x01,  0xA9},                      //T301
		{0x28,  0x59,  0xB1,  0x56,  0xB5,  0x01, 0x3C,  0xA5},                      //T302
		{0x28,  0xF4,  0x55,  0x4D,  0x3A,  0x19, 0x01,  0x3E},                      //T303
		{0x28,  0x48,  0xD8,  0xBB,  0x1E,  0x19, 0x01,  0x11},                      //T304
		{0x28,  0x3C,  0x36,  0x67,  0x3A,  0x19, 0x01,  0x34},                      //T305
		{0x28,  0x5F,  0x74,  0x56,  0xB5,  0x01, 0x3C,  0xE7},                      //T306
		{0x28,  0x51,  0x37,  0x56,  0xB5,  0x01, 0x3C,  0x53},                      //T307
		{0x28,  0x62,  0x45,  0x56,  0xB5,  0x01, 0x3C,  0x09},                      //T308
		{0x28,  0xE4,  0x27,  0x7B,  0x3A,  0x19, 0x01,  0xCA},                      //T309
		{0x28,  0x1E,  0x36,  0x02,  0x3A,  0x19, 0x01,  0xFD},                      //T310
		{0x28,  0x4C,  0xA3,  0x7E,  0x0C,  0x00, 0x00,  0x6D},                      //T_IOBOARD
		{0x28,  0x60,  0x99,  0x7E,  0x0C,  0x00, 0x00,  0x9F},                      //T_TECHM

};


/* Public variables */

/* Private methods */

/* Public methods */

void TEMP_Init(void)
{
	mTimer = 0;
	mReadId = 0;
	mConvertId = 0;
	mNumOfAssignedSensors = 0;

	// define hardware OW busses
	OW_AddBus(0,OW1_GPIO_Port, OW1_Pin);
	OW_AddBus(1,OW2_GPIO_Port, OW2_Pin);
	// OW_AddBus(2,OW3_GPIO_Port, OW3_Pin);  // not used so far

	mNumOfBuses = 2;

	// assign sensors on OW1 :
	AssignSensor(T_TECHM, VAR_TEMP_TECHM_BOARD, 0);
	AssignSensor(T108, VAR_TEMP_TANK_IN_H, 0);
	AssignSensor(T109, VAR_TEMP_TANK_OUT_H, 0);
	AssignSensor(T301, VAR_TEMP_TANK_1, 0);
	AssignSensor(T302, VAR_TEMP_TANK_2, 0);
	AssignSensor(T103, VAR_TEMP_TANK_3, 0);
	AssignSensor(T104, VAR_TEMP_TANK_4, 0);
	AssignSensor(T105, VAR_TEMP_TANK_5, 0);
	AssignSensor(T306, VAR_TEMP_TANK_6, 0);

	// assign sensors on OW2 :
	AssignSensor(T303, VAR_TEMP_BOILER, 1);
	AssignSensor(T110, VAR_TEMP_BOILER_IN, 1);
	AssignSensor(T107, VAR_TEMP_BOILER_OUT, 1);
	AssignSensor(T101, VAR_TEMP_RAD_H, 1);
	AssignSensor(T106, VAR_TEMP_RAD_C, 1);
	AssignSensor(T102, VAR_TEMP_TANK_IN_C, 1);
	AssignSensor(T8, VAR_TEMP_TANK_OUT_C, 1);
	AssignSensor(T2, VAR_TEMP_WALL_IN, 1);
	AssignSensor(T3, VAR_TEMP_WALL_OUT, 1);
}

void TEMP_Update100ms(void)
{
	// cyclically read all assigned sensors and trig the conversion

	if (mTimer == 0)
	{
		// copy results to VARS
		int i;
		for(i = 0; i < mNumOfAssignedSensors; i++)
		{
			mSensors[i].errorCnt++;  // just assume error to simplify the code :-)
			// check the validity of last readout
			if ((mSensors[i].rawTempC_10ths >= -300) && (mSensors[i].rawTempC_10ths <= 1250) )  // valid range from -30 to 125 deg C
			{
				mSensors[i].status = ets_Valid;
				mSensors[i].errorCnt = 0;
				mSensors[i].tempC_10ths = mSensors[i].rawTempC_10ths;  // copy the raw value to valid value
				VAR_SetVariable(mSensors[i].VarId, mSensors[i].tempC_10ths, 1);
			}

			if (mSensors[i].errorCnt > MAX_ERR_TO_INVALIDATE)
			{
				mSensors[i].status = ets_NotValid;
				mSensors[i].errorCnt = 0;
				VAR_SetVariable(mSensors[i].VarId, mSensors[i].tempC_10ths, 0);  // set invalid flag also to variables..
				Buzzer_SetMode(eLED_BEEP_ONCE);
				// TBD LOG ERR
			}

		}
		// trig conversion

	}

	if (mTimer < mNumOfBuses)
	{
		OW_ConvertAll(mTimer);  //  convert one Bus
	}


	if (mTimer > 20)  // 2 sec delay
	{

		OW_ReadSensor(mSensors[mReadId].owBusId,
									&(mSensorsAddress[mSensors[mReadId].sensorId]),
									&(mSensors[mReadId].rawTempC_10ths));
		mReadId++;

	}
	mTimer++;

	if (mReadId >= mNumOfAssignedSensors)
	{
		mReadId = 0;
		mTimer = 0;
		ConvertPtc();
	}

}


void AssignSensor(uint8_t sensorId, uint8_t varId, uint8_t busId)
{
	if (mNumOfAssignedSensors < NUM_OF_ALL_SENSORS)
	{
		mSensors[mNumOfAssignedSensors].sensorId = sensorId;
		mSensors[mNumOfAssignedSensors].tempC_10ths = 0x8000;
		mSensors[mNumOfAssignedSensors].status = ets_NotReady;
		mSensors[mNumOfAssignedSensors].VarId = varId;
		mSensors[mNumOfAssignedSensors].owBusId = busId;
		mSensors[mNumOfAssignedSensors].errorCnt = 0;
		mNumOfAssignedSensors++;
	}
}



void ConvertPtc(void)
{
	uint16_t PtcRaw = ADC_GetValue(ADC_CHANNEL_PTC);  // raw ADC result
	double Ptc_mV = ADC_VREF_MV/4096.0 * PtcRaw;  // convert to milivolts

	Ptc_mV += 60;  // compensation of offset error ?   TODO checked in full scale!

	double V2 = (double)ADC_VREF_MV * R2/(R1+R2);   // Opamp inputs in miliVolts,
	double Vptc = (V2*(R3+R4) - Ptc_mV*R3)/R4;
	double Rpt = (Vptc*R5)/(ADC_VREF_MV - Vptc);   // resistance of PT1000 in ohms
	double temp = (Rpt - 1000) * 2.61;		// 10ths of degree C
	mPtcTemp = (int16_t)temp;

	if(PtcRaw != 0 && PtcRaw != 4096)
	{
		VAR_SetVariable(VAR_TEMP_BOILER_EXHAUST, mPtcTemp, 1);
	}
	else
	{
		VAR_SetVariable(VAR_TEMP_BOILER_EXHAUST, mPtcTemp, 0);
		// TBD LOG ERR
	}

}




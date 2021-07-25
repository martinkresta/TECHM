/*
 * ADC.c
 *
 *  Created on: Jul 25, 2021
 *      Author: Martin
 */

#include "main.h"
#include "adc.h"


ADC_HandleTypeDef* adc;
DMA_HandleTypeDef* dma;

uint16_t mResults[NUM_OF_CHANNELS];
uint16_t mPtcRaw;


void ADC_Init(ADC_HandleTypeDef*  ADC, DMA_HandleTypeDef* DMA)
{
	adc = ADC;
	dma = DMA;
	HAL_ADCEx_Calibration_Start(adc, ADC_SINGLE_ENDED);
}


void ADC_StartConversion(void)
{
	HAL_ADC_Start_DMA(adc, mResults, NUM_OF_CHANNELS);
}

uint16_t ADC_GetValue(uint8_t channel)
{
	return mResults[channel];
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{

	mPtcRaw = mResults[0];
	HAL_ADC_Stop_DMA(adc);

}

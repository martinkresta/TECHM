/*
 * ADC.h
 *
 *  Created on: Jul 25, 2021
 *      Author: Martin
 */

#ifndef INC_ADC_H_
#define INC_ADC_H_


#include "main.h"


#define NUM_OF_CHANNELS		7

#define ADC_CHANNEL_PTC				0
#define ADC_CHANNEL_HEATER_1	1
#define ADC_CHANNEL_HEATER_2	2
#define ADC_CHANNEL_HEATER_3	3
#define ADC_CHANNEL_HEATER_4	4
#define ADC_CHANNEL_HEATER_5	5
#define ADC_CHANNEL_HEATER_6	6


#define ADC_VREF_MV					3313   // mV

void ADC_Init(ADC_HandleTypeDef*  adc, DMA_HandleTypeDef* dma);

void ADC_StartConversion(void);
uint16_t ADC_GetValue(uint8_t channel);



#endif /* INC_ADC_H_ */

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

void ADC_Init(ADC_HandleTypeDef*  adc, DMA_HandleTypeDef* dma);

void ADC_StartConversion(void);
uint16_t ADC_GetValue(uint8_t channel);



#endif /* INC_ADC_H_ */

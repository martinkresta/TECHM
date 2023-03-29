/*
 * APP.h
 *
 *  Created on: Aug 14, 2021
 *      Author: Martin
*				Brief:  Main application and compile time configuration
 */

#ifndef INC_APP_H_
#define INC_APP_H_

#include "main.h"
#include "MCAN.h"



// CAN node ID
#define THIS_NODE				NODEID_TECHM

// ADC channels

#define NUM_OF_ADC_CHANNELS		7

#define ADC_CHANNEL_PTC				0
#define ADC_CHANNEL_HEATER_1	1
#define ADC_CHANNEL_HEATER_2	2
#define ADC_CHANNEL_HEATER_3	3
#define ADC_CHANNEL_HEATER_4	4
#define ADC_CHANNEL_HEATER_5	5
#define ADC_CHANNEL_HEATER_6	6


void APP_Init(void);
void APP_Start(void);
void APP_ProcessMessages(void);
void APP_Update_1s(void);


#endif /* INC_APP_H_ */

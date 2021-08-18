/*
 * WM.h
 *
 *  Created on: Aug 18, 2021
 *      Author: Martin
 */

#ifndef INC_WM_H_
#define INC_WM_H_

#include "main.h"

void WM_Init(void);

void WM_Update_1s(void);

int16_t WM_GetFlowCold(void);

int16_t WM_GetFlowHot(void);

int16_t WM_GetConsumptionCold(void);

int16_t WM_GetConsumptionHot(void);

void WM_ResetConsupmtions(void);

#endif /* INC_WM_H_ */

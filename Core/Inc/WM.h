/*
 * WM.h
 *
 *  Created on: Aug 18, 2021
 *      Author: Martin
 */

#ifndef INC_WM_H_
#define INC_WM_H_

#include "main.h"
#include "uha_vars_gen.h"   /* sVarsWater */

/* produced: pVars (Vars2.Water), published in WM_Update_10ms() (main loop) */
void WM_Init(sVarsWater* pVars);

void WM_Update_10ms(void);

int16_t WM_GetFlowCold(void);

int16_t WM_GetFlowHot(void);

int16_t WM_GetConsumptionCold(void);

int16_t WM_GetConsumptionHot(void);

void WM_ResetConsupmtions(void);

void WM_ExtiCallback(uint16_t GPIO_Pin);

#endif /* INC_WM_H_ */

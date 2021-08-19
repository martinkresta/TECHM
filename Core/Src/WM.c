/*
 * WM.c
 *
 *  Created on: Aug 18, 2021
 *      Author: Martin
 */

#include "OW.h"
#include "VARS.h"

// private variables

static uint32_t mConsCold,mConsHot;
static int16_t mFlowCold_dlpm,mFlowHot_dlpm;   // deciliters per minute

static uint32_t mLastConsCold, mLastConsHot;

static uint32_t mP2PCold, mP2PHot;   // pulse to pulse period

void WM_Init(void)
{
	mFlowCold_dlpm = 0;
	mFlowHot_dlpm = 0;
	mConsCold = 0;
	mConsHot = 0;
}

void WM_Update_10ms(void)
{
	// calculate flow.  wateremter gives one pulse every 0,3 liter
	if(mConsCold != mLastConsCold)
	{
		mLastConsCold = mConsCold;
		mFlowCold_dlpm =  18000 / mP2PCold;   // ( 60 * 100 * 30 / 10 ) / mP2PCold
		mP2PCold = 0;
		VAR_SetVariable(VAR_FLOW_COLD, mFlowCold_dlpm, 1);
	}
	else
	{
		mP2PCold ++;
		// zero the flow, if current P2P is twice longer then previous
		if (mFlowCold_dlpm > 0)
		{
			if (mP2PCold > 2*(18000/mFlowCold_dlpm))
			{
				mFlowCold_dlpm = 0;
				VAR_SetVariable(VAR_FLOW_COLD, mFlowCold_dlpm, 1);
			}
		}
	}


	if(mConsHot != mLastConsHot)
		{
			mLastConsHot = mConsHot;
			mFlowHot_dlpm =  18000 / mP2PHot;   // ( 60 * 100 * 30 / 10 ) / mP2PCold
			mP2PHot = 0;
			VAR_SetVariable(VAR_FLOW_HOT, mFlowHot_dlpm, 1);
		}
		else
		{
			mP2PHot ++;
			// zero the flow, if current P2P is twice longer then previous
			if (mFlowHot_dlpm > 0)
			{
				if (mP2PHot > 2*(18000/mFlowHot_dlpm))
				{
					mFlowHot_dlpm = 0;
					VAR_SetVariable(VAR_FLOW_HOT, mFlowHot_dlpm, 1);
				}
			}
		}

}

int16_t WM_GetFlowCold(void)
{
	return mFlowCold_dlpm;
}

int16_t WM_GetFlowHot(void)
{
	return mFlowCold_dlpm;
}


int16_t WM_GetConsumptionCold(void)
{
	return (int16_t)(mConsCold/10);
}

int16_t WM_GetConsumptionHot(void)
{
	return (int16_t)(mConsHot/10);
}

void WM_ResetConsupmtions(void)
{
	mConsCold = 0;
	mConsHot = 0;
}



// exti interrupt callback
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == WM1_Pin)
	{
		mConsCold += 3;
		VAR_SetVariable(VAR_CONS_COLD, (int16_t)(mConsCold), 1);
	}
	if (GPIO_Pin == WM2_Pin)
	{
		mConsHot += 3;
		VAR_SetVariable(VAR_CONS_HOT, (int16_t)(mConsHot), 1);
	}

}

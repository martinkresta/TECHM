/*
 * WM.c
 *
 *  Created on: Aug 18, 2021
 *      Author: Martin
 */

#include "WM.h"
#include <RTC.h>
#include "OW.h"
#include "VARS2.h"


// private variables

static uint32_t mConsCold,mConsHot;
static int16_t mFlowCold_dlpm,mFlowHot_dlpm;   // deciliters per minute

static uint32_t mLastConsCold, mLastConsHot;

static uint32_t mP2PCold, mP2PHot;   // pulse to pulse period

static uint8_t mTodayDayNumber;

static sVarsWater* mpVars;   /* produced */

void WM_Init(sVarsWater* pVars)
{
	mpVars = pVars;
	mFlowCold_dlpm = 0;
	mFlowHot_dlpm = 0;
	mConsCold = 0;
	mConsHot = 0;
	mTodayDayNumber = 0;
}

void WM_Update_10ms(void)
{
	// calculate flow.  wateremter gives one pulse every 0,3 liter
	if(mConsCold != mLastConsCold)
	{
		mLastConsCold = mConsCold;
		mFlowCold_dlpm =  18000 / mP2PCold;   // ( 60 * 100 * 30 / 10 ) / mP2PCold
		mP2PCold = 0;
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
			}
		}
	}


	if(mConsHot != mLastConsHot)
		{
			mLastConsHot = mConsHot;
			mFlowHot_dlpm =  18000 / mP2PHot;   // ( 60 * 100 * 30 / 10 ) / mP2PCold
			mP2PHot = 0;
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
			}
		}
	}

	// publish (main loop; the EXTI callback only counts the pulses). Flow in dl/min, consumption in dl
	mpVars->ColdFlow_lpm.value = (float)mFlowCold_dlpm / 10.0f;
	mpVars->HotFlow_lpm.value = (float)mFlowHot_dlpm / 10.0f;
	mpVars->ColdCons_l.value = (float)mConsCold / 10.0f;
	mpVars->HotCons_l.value = (float)mConsHot / 10.0f;
	VARS2_SetGroupValid(mpVars, 1);

/*	// reset the energy counter at midnight
	if (mTodayDayNumber != RTC_GetTime().Day)
	{
		mConsHot = 0;
		mConsCold = 0;
		mTodayDayNumber = RTC_GetTime().Day;
	}*/

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


void WM_ExtiCallback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == WM1_Pin)
	{
		mConsCold += 3;   // interrupt: count only, published in WM_Update_10ms()
	}
	if (GPIO_Pin == WM2_Pin)
	{
		mConsHot += 3;
	}
}


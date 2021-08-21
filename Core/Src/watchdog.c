// Simple wrapper for using STM32 IWDG watchdog. using HAL library
// Author: martin kresta
// Date : 18.3.2021

#include "main.h"
#include "watchdog.h"


void WDG_Init(uint32_t period_ms)
{
	__HAL_DBGMCU_FREEZE_IWDG();  // stop the watchdog in debug mode
	if (period_ms > 32000)
	{
		period_ms = 32000;
	}
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
	hiwdg.Init.Reload = period_ms/8;
	HAL_IWDG_Init(&hiwdg);
}

void WDG_Refresh()
{
	HAL_IWDG_Refresh(&hiwdg);
}

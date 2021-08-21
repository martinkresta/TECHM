// Simple wrapper for using STM32 IWDG watchdog. using HAL library
// Author: martin kresta
// Date : 18.3.2021


#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "main.h"

void WDG_Init(uint32_t ms);
void WDG_Refresh();




#endif 
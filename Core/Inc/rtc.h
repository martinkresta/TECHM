/**
  ******************************************************************************
  * File Name          : RTC.h
  * Description        : This file provides code for the configuration
  *                      of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __rtc_H
#define __rtc_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN Private defines */

#define RTC_PREDIV_S		255
#define RTC_PREDIV_A		127


/* Public methods */

/* USER CODE END Private defines */

void MX_RTC_Init(void);

/* USER CODE BEGIN Prototypes */
typedef struct
{
  uint8_t Hour;
  uint8_t Minute;
  uint8_t Second;
  uint8_t Day;
  uint8_t Month;
  uint8_t WeekDay;
  uint16_t Year;
}sDateTime;


void RTC_Init(void);

sDateTime RTC_GetTime(void);

void RTC_SetTime(sDateTime dt);

uint32_t RTC_GetUnixTime(void);

void RTC_PrintTimeStamp(char * ts);
 void RTC_PrintTimeStampMs(char* ts);

void RTC_PrintDate(char* ts);

void RTC_DataReceived(uint8_t* data);



/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ rtc_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

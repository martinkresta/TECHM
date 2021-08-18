/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern CAN_HandleTypeDef hcan1;

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Life_Pin GPIO_PIN_13
#define LED_Life_GPIO_Port GPIOC
#define BUZZ_Pin GPIO_PIN_14
#define BUZZ_GPIO_Port GPIOC
#define ETS_Pin GPIO_PIN_15
#define ETS_GPIO_Port GPIOC
#define PTC_Pin GPIO_PIN_0
#define PTC_GPIO_Port GPIOC
#define WM1_Pin GPIO_PIN_1
#define WM1_GPIO_Port GPIOC
#define WM1_EXTI_IRQn EXTI1_IRQn
#define WM2_Pin GPIO_PIN_2
#define WM2_GPIO_Port GPIOC
#define WM2_EXTI_IRQn EXTI2_IRQn
#define WM3_Pin GPIO_PIN_3
#define WM3_GPIO_Port GPIOC
#define WM4_Pin GPIO_PIN_0
#define WM4_GPIO_Port GPIOA
#define PUMP_RAD_Pin GPIO_PIN_1
#define PUMP_RAD_GPIO_Port GPIOA
#define PUMP_WALL_Pin GPIO_PIN_2
#define PUMP_WALL_GPIO_Port GPIOA
#define PUMP_BOIL_Pin GPIO_PIN_3
#define PUMP_BOIL_GPIO_Port GPIOA
#define PG_48V_Pin GPIO_PIN_0
#define PG_48V_GPIO_Port GPIOB
#define OUT4_Pin GPIO_PIN_2
#define OUT4_GPIO_Port GPIOB
#define OUT5_Pin GPIO_PIN_10
#define OUT5_GPIO_Port GPIOB
#define OUT6_Pin GPIO_PIN_11
#define OUT6_GPIO_Port GPIOB
#define OUT1_Pin GPIO_PIN_12
#define OUT1_GPIO_Port GPIOB
#define OUT2_Pin GPIO_PIN_13
#define OUT2_GPIO_Port GPIOB
#define OUT3_Pin GPIO_PIN_14
#define OUT3_GPIO_Port GPIOB
#define OW3_Pin GPIO_PIN_15
#define OW3_GPIO_Port GPIOB
#define OW2_Pin GPIO_PIN_6
#define OW2_GPIO_Port GPIOC
#define OW1_Pin GPIO_PIN_7
#define OW1_GPIO_Port GPIOC
#define AV_ENC2_Pin GPIO_PIN_8
#define AV_ENC2_GPIO_Port GPIOC
#define AV_ENC1_Pin GPIO_PIN_9
#define AV_ENC1_GPIO_Port GPIOC
#define SERVO_WALL_Pin GPIO_PIN_8
#define SERVO_WALL_GPIO_Port GPIOA
#define SERVO_RAD_Pin GPIO_PIN_9
#define SERVO_RAD_GPIO_Port GPIOA
#define AV4_Pin GPIO_PIN_10
#define AV4_GPIO_Port GPIOC
#define AV3_Pin GPIO_PIN_11
#define AV3_GPIO_Port GPIOC
#define AV2_Pin GPIO_PIN_12
#define AV2_GPIO_Port GPIOC
#define AV1_Pin GPIO_PIN_2
#define AV1_GPIO_Port GPIOD
#define AV_PWM_Pin GPIO_PIN_3
#define AV_PWM_GPIO_Port GPIOB
#define ELV_Pin GPIO_PIN_5
#define ELV_GPIO_Port GPIOB
#define LED_B_Pin GPIO_PIN_3
#define LED_B_GPIO_Port GPIOH
#define LED_G_Pin GPIO_PIN_8
#define LED_G_GPIO_Port GPIOB
#define LED_R_Pin GPIO_PIN_9
#define LED_R_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

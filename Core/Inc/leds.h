/**
  ******************************************************************************
  * @file    leds.c
  * @author  Martin Kresta
  ******************************************************************************/
	
#ifndef LEDS_H
#define LEDS_H

#include "main.h"

typedef enum
{
	eLEDL_NEGATIVE		= 0x00, 		/**< If is set negative -> <b> Pin in logic 0 means ACTIVE and pin in logic 1 mean INACTIVE</b> */
	eLEDL_POSITIVE								/**< If is set positive -> <b> Pin in logic 0 means INACTIVE and pin in logic 1 mean ACTIVE</b> */
}eLEDL;

/** 
  * @brief  LED status Enum
  */ 
typedef enum
{
	eLED_OFF						= 0x00,
	eLED_ON,
	eLED_BLINKING_SLOW,
	eLED_BLINKING_FAST,
	eLED_BLINK_ONCE,
	eLED_FLASH, 
	eLED_COMBI_1,
	eLED_TOGGLE
}eLED_Status;

/** 
  * @brief LED control structure 
  */
typedef struct
{
	uint16_t				Led_Cnt;
	eLED_Status			Status;
	GPIO_TypeDef*		Port;
	uint32_t 				Pin;
	eLEDL						Logic;
}sLED;


void LED_Init (void);
void LED_Update_10ms(void);
void LED_R_SetMode(eLED_Status status);
void LED_G_SetMode(eLED_Status status);
void LED_B_SetMode(eLED_Status status);

#endif // LEDS_H

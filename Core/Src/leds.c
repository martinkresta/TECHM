/**
  ******************************************************************************
  * @file    leds.c
  * @author  Martin Kresta
  ******************************************************************************/

/*************************
	I N C L U D E
*************************/
#include "leds.h"

/*************************
	V A R I A B L E 
*************************/
sLED 	LED_Life;
sLED	LED_R;
sLED	LED_G;
sLED	LED_B;
sLED  Buzzer;



/*************************
	F U N C T I O N
*************************/


static void LED_Control (sLED *led,uint8_t led_switch);
void LED_Struct_Initialisation (sLED init_struct, sLED *target);



void LED_Init (void)
{
	sLED init_struct;
	
	init_struct.Status			= eLED_OFF;
	init_struct.Logic				= eLEDL_POSITIVE;
	init_struct.Led_Cnt			= 0x00;
	
	init_struct.Pin					= LED_Life_Pin;
	init_struct.Port				= LED_Life_GPIO_Port;
	init_struct.Status			= eLED_BLINKING_SLOW;
	LED_Struct_Initialisation (init_struct, &LED_Life);
	
	init_struct.Status			= eLED_OFF;
	init_struct.Pin					= LED_R_Pin;
	init_struct.Port				= LED_R_GPIO_Port;
	LED_Struct_Initialisation (init_struct, &LED_R);

	init_struct.Pin					= LED_G_Pin;
  init_struct.Port				= LED_G_GPIO_Port;
	LED_Struct_Initialisation (init_struct, &LED_G);

	init_struct.Pin					= LED_B_Pin;
	init_struct.Port				= LED_B_GPIO_Port;
  LED_Struct_Initialisation (init_struct, &LED_B);

  init_struct.Pin					= BUZZ_Pin;
  init_struct.Port				= BUZZ_GPIO_Port;
  LED_Struct_Initialisation (init_struct, &Buzzer);





}

/**
* @brief  Reading of actual states on defined Digital input
* @param	*di_input	Pointer to Digital input
*/
void LED_Struct_Initialisation (sLED init_struct, sLED *target)
{
	*target = init_struct;
	LED_Control(target,0);
}


// periodicaly scheduled updtae function
void LED_Update_10ms()
{
	LED_Control(&LED_Life, 	0x00);
	LED_Control(&LED_R, 	0x00);
	LED_Control(&LED_G, 	0x00);
	LED_Control(&LED_B, 	0x00);
	LED_Control(&Buzzer, 	0x00);
}


void LED_R_SetMode(eLED_Status status)
{
	if (LED_R.Status != status)
	{
		LED_R.Status = status;
		LED_R.Led_Cnt = 0;
	}
}

void LED_G_SetMode(eLED_Status status)
{
	if (LED_G.Status != status)
	{
		LED_G.Status = status;
		LED_G.Led_Cnt = 0;
	}
}

void LED_B_SetMode(eLED_Status status)
{
	if (LED_B.Status != status)
	{
		LED_B.Status = status;
		LED_B.Led_Cnt = 0;
	}
}

void LED_Life_SetMode(eLED_Status status)
{
	if (LED_Life.Status != status)
	{
		LED_Life.Status = status;
		LED_Life.Led_Cnt = 0;
	}
}

void Buzzer_SetMode(eLED_Status status)
{
	if (Buzzer.Status != status)
	{
		Buzzer.Status = status;
		Buzzer.Led_Cnt = 0;
	}
}

/**
  * @brief  Controlling states of LED 
  * @param  led: Pointer to specific LED
  * @param  led_switch: If isn't this parameter null, the function inverts status of specific LED, but LED has to be in TOGGLE mode!
  */
static void LED_Control (sLED *led,uint8_t led_switch)
{

	switch(led->Status)
	{
		case eLED_OFF:
			if(led->Logic == eLEDL_NEGATIVE)
			{
				HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
			}
			else
			{
				HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
			}
			break;
		case eLED_ON:
			if(led->Logic == eLEDL_NEGATIVE)
			{
				HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
			}
			else
			{
				HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
			}			
			break;
		case eLED_BLINKING_SLOW:
			if(led->Led_Cnt <= 50)
			{
				if(led->Logic == eLEDL_NEGATIVE)
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
				}
				else
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
				}	
			}
			else
			{
				if(led->Logic == eLEDL_NEGATIVE)
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
				}
				else
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
				}
			}

			if(led->Led_Cnt >= 100)
			{
				led->Led_Cnt = 0;
			}
			break;
		case eLED_BLINKING_FAST:
			if(led->Led_Cnt <= 10 )
			{
				if(led->Logic == eLEDL_NEGATIVE)
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
				}
				else
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
				}	
			}
			else
			{
				if(led->Logic == eLEDL_NEGATIVE)
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
				}
				else
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
				}
			}

			if(led->Led_Cnt >= 20)
			{
				led->Led_Cnt = 0;
			}			
			break;
		case eLED_BLINK_ONCE:
			if(led->Led_Cnt <= 2)  // blink for 20ms
			{
				if(led->Logic == eLEDL_NEGATIVE)
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
				}
				else
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
				}
			}				
			else
			{
				if(led->Logic == eLEDL_NEGATIVE)
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
				}
				else
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
				}
				
				led->Status = eLED_OFF;   // go to permanent off state
				led->Led_Cnt = 0;
			}		
			break;
		 case eLED_BEEP_ONCE:
					if(led->Led_Cnt <= 25)  // blink for 250ms
					{
						if(led->Logic == eLEDL_NEGATIVE)
						{
							HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
						}
						else
						{
							HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
						}
					}
					else
					{
						if(led->Logic == eLEDL_NEGATIVE)
						{
							HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
						}
						else
						{
							HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
						}

						led->Status = eLED_OFF;   // go to permanent off state
						led->Led_Cnt = 0;
					}
					break;
		case eLED_FLASH:
			if(led->Led_Cnt < 2)
			{
				if(led->Logic == eLEDL_NEGATIVE)
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
				}
				else
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
				}	
			}
			else
			{
				if(led->Logic == eLEDL_NEGATIVE)
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
				}
				else
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
				}
			}

			if(led->Led_Cnt >= 200)
			{
				led->Led_Cnt = 0;
			}			
			break;
		case eLED_TOGGLE:
			if(led_switch)
			{
				HAL_GPIO_TogglePin(led->Port,led->Pin);
			}
			break;
		case eLED_COMBI_1:
			if(led->Led_Cnt < 10)
			{
				if(led->Logic == eLEDL_NEGATIVE)
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
				}
				else
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
				}	
			}
			else if((led->Led_Cnt >= 10)&&(led->Led_Cnt < 50))
			{
				if(led->Logic == eLEDL_NEGATIVE)
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
				}
				else
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
				}
			}
			else if((led->Led_Cnt >= 50)&&(led->Led_Cnt < 60))
			{
				if(led->Logic == eLEDL_NEGATIVE)
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
				}
				else
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
				}	
			}
			else if((led->Led_Cnt >= 60)&&(led->Led_Cnt < 100))
			{
				if(led->Logic == eLEDL_NEGATIVE)
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
				}
				else
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
				}
			}
			else if((led->Led_Cnt >= 100)&&(led->Led_Cnt < 150))
			{
				if(led->Logic == eLEDL_NEGATIVE)
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
				}
				else
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
				}	
			}
			else if((led->Led_Cnt >= 150)&&(led->Led_Cnt < 200))
			{
				if(led->Logic == eLEDL_NEGATIVE)
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_SET);
				}
				else
				{
					HAL_GPIO_WritePin(led->Port,led->Pin,GPIO_PIN_RESET);
				}
			}

			if(led->Led_Cnt >= 200)
			{
				led->Led_Cnt = 0;
			}			
			
			break;
		default:
			break;
	}
	
	if(led->Led_Cnt < 0xFFFF)
	{
		led->Led_Cnt++;		
	}	
}

// Digital inputs 
// Author : Martin Kresta, credits: Vaclav Silar
// Date   : 14.10.2019
// Project: Robodof


#ifndef _DIN_H
#define _DIN_H

/*************************
	I N C L U D E
*************************/
#include "main.h"

/*************************
	T Y P E D E F
*************************/


#define NUM_OF_INPUTS		3

#define IN1_STOVE_BTN		0U    // Stove control button
#define IN2_STOVE_DOOR	1U    // Wood stove doorswitch
#define IN3_AV_HOMEPOS  2U    // Airvalve home pos


/** 
  * @brief  Enum for recognize of status on pin. 
	*
	* It is depend on setting polarity which is active. Is possible set both options. Not together!
  */ 
typedef enum
{
	eDI_LO		= 0x00, /**< Digital input is reset. */
	eDI_HI		= 0x01	/**< Digital input is set. */
}eDI;




/**
	* @brief Typedef to service function for OnChange event on digital input
	*/
//typedef void (*action_fce_DI_on_change)(int cnt_arg, ...);
typedef void (*action_fce_DI_on_change)(void);

/**
	* @brief Typedef to service fuction for "Long press" on digital input
	*/
typedef void (*action_fce_DI_long_action)(void);

/** 
	*	@brief Digital Input struct
	*/
typedef struct
{
	GPIO_TypeDef*								Port;									/**< Define GATE, where is pin mounted. This variable is pointer to GATE. */
	uint32_t 										Pin;									/**< Position of pin, number of pin on mounted gate */	
	eDI		 											Main_Status;					/**< Stable value on pin (Filtered value) */
	eDI		 											Last_Status;					/**< Last value on pin (Filtered value) */
	eDI													Actual_Status;				/**< Actual vaulue on pin (Unfiltered value) */
	uint16_t										Cnt_DI;								/**< Counter how long is in Actual status */
	uint8_t											Cnt_Filter;						/**< Counter of countinuosly reading same values	*/	
	uint8_t											Filter_Value;					/**< How many same Actual_Status has to be reading continuously, for state changing */	
	uint8_t											DiId;									// input id
}sDI;

/*************************
	P R O T O T Y P E
*************************/
void 		DI_Read												  (sDI *di_input);
eDI			DI_Get													(uint8_t input);
void		DI_Init												(void);
void		DI_Read_All										(void);

#endif // _DIN_H

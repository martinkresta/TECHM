// Digital inputs 
// Author : Martin Kresta, credits: Vaclav Silar
// Date   : 14.10.2019
// Project: Robodof


#include "di.h"
#include "APP.h"
#include "GEST.h"

static sDI DI[NUM_OF_INPUTS];

/* Private functions prototypes */
static void 		DI_Struct_Initialisation (sDI init_struct, uint8_t input);

static void 	OnChange(sDI *di_input);

/* Public functions definitions */

void DI_Init(void)
{
	
	sDI init_struct;
	
	init_struct.Main_Status				=	eDI_LO;
	init_struct.Actual_Status			= eDI_LO;	
	init_struct.Last_Status				= eDI_LO;
	init_struct.Cnt_DI						= 0;
	init_struct.Cnt_Filter				= 0;
	init_struct.Filter_Value			= 20;   // 20ms
	
	//----------------------------------------------------------------------------------------
	init_struct.Port							= DI1_GPIO_Port;
	init_struct.Pin								= DI1_Pin;
	DI_Struct_Initialisation(init_struct,IN1_STOVE_BTN);
	//----------------------------------------------------------------------------------------
	init_struct.Port							= DI2_GPIO_Port;   // stove door
	init_struct.Pin								= DI2_Pin;
	DI_Struct_Initialisation(init_struct,IN2_STOVE_DOOR);
	//----------------------------------------------------------------------------------------
	init_struct.Main_Status       = eDI_HI;
	init_struct.Actual_Status     = eDI_HI;
	init_struct.Last_Status       = eDI_HI;
  init_struct.Port              = AV_HOME_GPIO_Port;   // air valve home pos
  init_struct.Pin               = AV_HOME_Pin;
  DI_Struct_Initialisation(init_struct,IN3_AV_HOMEPOS);
  //----------------------------------------------------------

	//----------------------------------------------------------------------------------------

	
}


/**
* @brief  Reading of actual states on defined Digital input
* @param	*di_input	Pointer to Digital input
*/
static void DI_Struct_Initialisation (sDI init_struct, uint8_t input)
{
	init_struct.DiId = input;
	DI[input] = init_struct;
	DI_Read(&(DI[input]));
}

/**
* @brief  Reading of actual states on defined Digital input
* @param	*di_input	Pointer to Digital input
*/
void DI_Read (sDI *di_input)
{	
	di_input->Actual_Status = (eDI)HAL_GPIO_ReadPin(di_input->Port,di_input->Pin);
	if(di_input->Actual_Status != di_input->Main_Status)
	{
		di_input->Cnt_Filter++;
		if(di_input->Cnt_Filter > di_input->Filter_Value)
		{
			di_input->Main_Status = di_input->Actual_Status;							
			di_input->Cnt_Filter = 0;
			di_input->Cnt_DI = 0;
			if (di_input->Main_Status != di_input->Last_Status)
			{
				di_input->Last_Status = di_input->Main_Status;
				OnChange(di_input);
			}
		}
	}
	else
	{
		if(di_input->Cnt_DI!=0xFFFF)
		{
			di_input->Cnt_DI++;				
		}
	}
}


// return s status of one specified digital input
eDI DI_Get(uint8_t input)
{
	if (input < NUM_OF_INPUTS)
	{
		return DI[input].Main_Status;
	}
	return eDI_LO;
}



// sequentialy reads all digital inputs 
void DI_Read_All(void)
{
	uint8_t i;
	for (i = 0; i < NUM_OF_INPUTS; i++)
	{
		DI_Read(&DI[i]);
	}
}


// notify about input change
static void OnChange(sDI *di)
{
	//APP_DiInputChanged(di->DiId,di->Main_Status);
	GEST_DiInputChanged(di->DiId,di->Main_Status);
}

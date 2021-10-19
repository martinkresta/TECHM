/*
 * VARS.c
 *
 *  Created on: Jul 13, 2021
 *      Author: Martin
 */


#include "VARS.h"
#include "main.h"

sVar Vars[NUM_OF_VARIABLES];

int16_t mDummyVar = 0x8000;


void VAR_Init(void)
{
	int i;
	for(i = 0; i < NUM_OF_VARIABLES; i++)
	{
		Vars[i].valid = 0;
		Vars[i].local = 0;
		Vars[i].value = 0;
	}
}

int16_t VAR_GetVariable(uint16_t varId, uint16_t* invalid)
{
	if (varId < NUM_OF_VARIABLES)
	{
		if(Vars[varId].valid == 0)
		{
			if (invalid != NULL) *invalid |= INVALID_FLAG;
		}
		return Vars[varId].value;
	}
	else
	{
		if (invalid != NULL) *invalid |= INVALID_FLAG;
		return 0;
	}
}


int16_t* VAR_GetVariablePointer(uint8_t varId, uint16_t* invalid)
{
	if (varId < NUM_OF_VARIABLES)
	{
		if(Vars[varId].valid == 0)
		{
			if (invalid != NULL) *invalid |= INVALID_FLAG;
		}
		return &(Vars[varId].value);
	}
	else
	{
		if (invalid != NULL) *invalid |= INVALID_FLAG;
		return &(Vars[varId].value);
	}
}

void VAR_SetVariable(uint8_t varId, int16_t value, uint8_t valid)
{
	if (varId < NUM_OF_VARIABLES)
	{
		Vars[varId].valid = valid;
		Vars[varId].value = value;
	}
}


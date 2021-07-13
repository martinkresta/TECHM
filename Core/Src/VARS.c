/*
 * VARS.c
 *
 *  Created on: Jul 13, 2021
 *      Author: Martin
 */


#include "vars.h"
#include "main.h"

int16_t* VarPointers[0xFF];

int16_t mDummyVar = 0x8000;

int16_t VAR_GetVariable(uint8_t varId)
{
	if (VarPointers[varId] != NULL)
	{
		return *(VarPointers[varId]);
	}
	else
	{
		return 0xFFFF;
	}
}

int16_t* VAR_GetVariablePointer(uint8_t varId)
{
	if (VarPointers[varId] != NULL)
	{
		return VarPointers[varId];
	}
	else
	{
		return &mDummyVar;
	}
}

void VAR_SetVariablePointer(uint8_t varId, int16_t* ptr)
{
	VarPointers[varId] = ptr;
}


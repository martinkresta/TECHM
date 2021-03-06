/*
 * SCOM.c
 *
 *  Created on: Aug 14, 2021
 *      Author: Martin
 *      Brief:  Universal CAN communication protocol between MCAN devices.
 *
 */

#include "main.h"
#include "COM.h"
#include "VARS.h"
#include "MCAN.h"
#include "rtc.h"


typedef struct
{
	uint8_t enable;
	uint16_t varId;
	uint16_t sendPeriod;  // 10ms
	uint16_t timer;
	void* next;
}sStreamVariable;



sStreamVariable mStreamList[STREAM_LIST_SIZE];
uint8_t mNodeId;



static void UpdateStreamList(uint16_t varId, uint16_t period);
static void SendVariable(uint16_t id);
static void InitStreamList(void);
static void ProcessMessage(s_CanRxMsg* msg);

void COM_Init(uint8_t nodeId)
{
	mNodeId = nodeId;
	InitStreamList();
}

uint8_t COM_GetNodeStatus(uint8_t nodeId)
{
	if (MCAN_GetNodesPt()[nodeId].canStatus == eNS_NMT_RUN)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int16_t COM_GetNetworkStatus(void)
{
	int16_t ns = 0;
	int16_t i;
	for (i = 0; i < MAX_NUM_OF_NODES; i++)
	{
		if (COM_GetNodeStatus(i) == 1)
		{
			ns |= (1 << i);
		}
	}
	return ns;
}

void COM_Update_10ms(void)
{
	// Process messages
	s_CanRxMsg msg;
	while(1 == MCAN_GetRxMessage(&msg))  // process all messages in buffer
	{
			ProcessMessage(&msg);
	}

	// stream the variables to CAN
	int i;
	for(i = 0; i < STREAM_LIST_SIZE; i++)
	{
		mStreamList[i].timer+=10;
	}
	for(i = 0; i < STREAM_LIST_SIZE; i++)
	{
		if (mStreamList[i].enable == 1 && mStreamList[i].sendPeriod != 0)
		{
			if (mStreamList[i].timer >= mStreamList[i].sendPeriod)
			{
				SendVariable(mStreamList[i].varId);
				mStreamList[i].timer = 0;
				break;  // send just 1 value every 10ms
			}
		}
	}
}

// transmit Button state change to CAN
/*void COM_SendBtnState(uint8_t btnId, eDI state)
{
	uint8_t data[8];
	data[0] = btnId >> 8;
	data[1] = btnId & 0xFF;
	data[2] = state >> 8;
	data[3] = state  & 0xFF;
	data[4] = 0;
	data[5] = 0;
	data[5] = 0;
	data[7] = 0;
	MCAN_SendFrame(CMD_BUTTON_STATE + mNodeId, data, 8);
}*/


void COM_SendVariable(uint16_t varId)
{
	SendVariable(varId);
}

void COM_SendMessage(uint8_t cmd, uint8_t* data, uint8_t dlc)
{
	MCAN_SendFrame(cmd, data, dlc);
}

/*Private methods*/

// Compile-time initialization of list of variables periodically sent toi the PC app
static void InitStreamList(void)
{
	UpdateStreamList(VAR_TEMP_TECHM_BOARD,1000);

	UpdateStreamList(VAR_EL_HEATER_STATUS, 3000);
	UpdateStreamList(VAR_EL_HEATER_POWER, 3000);
	UpdateStreamList(VAR_EL_HEATER_CURRENT, 3000);
	UpdateStreamList(VAR_EL_HEATER_CONS, 3000);

	UpdateStreamList(VAR_FLOW_COLD, 3000);
	UpdateStreamList(VAR_FLOW_HOT, 3000);
	UpdateStreamList(VAR_CONS_COLD, 3000);
	UpdateStreamList(VAR_CONS_HOT, 3000);


	UpdateStreamList(VAR_TEMP_BOILER, 3000);
	UpdateStreamList(VAR_TEMP_BOILER_IN, 3000);
	UpdateStreamList(VAR_TEMP_BOILER_OUT, 3000);
	UpdateStreamList(VAR_TEMP_TANK_IN_H, 3000);
	UpdateStreamList(VAR_TEMP_TANK_OUT_H, 3000);
	UpdateStreamList(VAR_TEMP_TANK_1, 3000);
	UpdateStreamList(VAR_TEMP_TANK_2, 3000);
	UpdateStreamList(VAR_TEMP_TANK_3, 3000);
	UpdateStreamList(VAR_TEMP_TANK_4, 3000);
	UpdateStreamList(VAR_TEMP_TANK_5, 3000);
	UpdateStreamList(VAR_TEMP_TANK_6, 3000);
	UpdateStreamList(VAR_TEMP_WALL_IN, 3000);
	UpdateStreamList(VAR_TEMP_WALL_OUT, 3000);
	UpdateStreamList(VAR_TEMP_BOILER_EXHAUST, 3000);
	UpdateStreamList(VAR_TEMP_RAD_H, 3000);
	UpdateStreamList(VAR_TEMP_RAD_C, 3000);
	UpdateStreamList(VAR_TEMP_TANK_IN_C, 3000);
	UpdateStreamList(VAR_TEMP_TANK_OUT_C, 3000);

}

static void ProcessMessage(s_CanRxMsg* msg)
{
	uint16_t cmd = msg->header.StdId & 0xFF0;  // maskout nodeid
	uint8_t producer = msg->header.StdId & 0x00F;  // maskout cmd
	int16_t par1,par2,par3,par4;
	uint32_t unixtime = 0;
	par1 = msg->data[0]*0xFF + msg->data[1];
	par2 = msg->data[2]*0xFF + msg->data[3];
	par3 = msg->data[4]*0xFF + msg->data[5];
	par4 = msg->data[6]*0xFF + msg->data[7];

	switch (cmd)
	{
		case CMD_BUTTON_STATE:
			break;
		case  CMD_VAR_VALUE:
			VAR_SetVariable(par1, par2, par3);  // tbd check valid flag
			break;
		case CMD_RPI_RTC_SYNC: // set RTC time
			unixtime = (uint32_t)par1 * 0xFFFF + par2;
			RTC_SetUnixTime(unixtime);
			break;
	}
	// TBD change cobID ! and put it to switch case
	if (msg->header.StdId == CMD_RPI_RTC_SYNC)
	{
		unixtime |= msg->data[0] << 24;
		unixtime |= msg->data[1] << 16;
		unixtime |= msg->data[2] << 8;
		unixtime |= msg->data[3];
		RTC_SetUnixTime(unixtime);
	}
	return;
}

static void UpdateStreamList(uint16_t varId, uint16_t period)
{
	// go thru the list to find if entry already exists
	int i;
	for(i = 0; i < STREAM_LIST_SIZE; i++)
	{
		if(mStreamList[i].varId == varId)
		{
			if(period != 0)
			{
				mStreamList[i].sendPeriod = period;
				mStreamList[i].enable = 1;
				return;
			}
			else
			{
				mStreamList[i].sendPeriod = 0;
				mStreamList[i].enable = 0;
			}
		}
	}

	// if not add variable to the list
	for(i = 0; i < STREAM_LIST_SIZE; i++)
	{
		if(mStreamList[i].enable == 0)
		{
			if(period != 0)
			{
				mStreamList[i].varId = varId;
				mStreamList[i].sendPeriod = period;
				mStreamList[i].enable = 1;
				return;
			}
		}
	}
}

static void SendVariable(uint16_t id)
{
	uint16_t invalid = 0;
	uint16_t validflag = 0;
	int16_t tmp = VAR_GetVariable(id, &invalid);
	validflag = (invalid == INVALID_FLAG ? 0 : 1);
	uint8_t data[8];
	data[0] = id >> 8;
	data[1] = id  & 0xFF;
	data[2] = tmp >> 8;
	data[3] = tmp & 0xFF;
	data[4] = validflag >> 8;
	data[5] = validflag & 0xFF;
	data[6] = 0;
	data[7] = 0;
	MCAN_SendFrame(CMD_VAR_VALUE + mNodeId, data, 8);
}



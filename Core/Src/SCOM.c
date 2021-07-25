/*
 * SCOM.c
 *
 *  Created on: Jul 13, 2021
 *      Author: Martin
 *      Brief:  Universal serial communication protocol between device and computer.
 *      Resources: DMA channels 5+4, USART1,
 */

#include "main.h"
#include "SCOM.h"
#include "VARS.h"
#include "DO.h"


typedef struct
{
	uint8_t enable;
	uint16_t varId;
	uint16_t sendPeriod;  // 10ms
	uint16_t timer;
	void* next;
}sScanVariable;



uint8_t mRxBuffer[COM_BUFLEN];
uint8_t mTxBuffer[COM_BUFLEN];
uint8_t mRxLength, mNewDataReady, mTxBusy;

UART_HandleTypeDef* ComUart;
sScanVariable mScanList[20];



void UpdateScanList(uint16_t varId, uint16_t period);
void SendVariable(uint16_t id);

void SCOM_Init(UART_HandleTypeDef* uart)
{

	ComUart = uart;
	mRxLength = 0;
	mNewDataReady = 0;
	mTxBusy = 0;

	// enable receiver
	HAL_UART_Receive_DMA(ComUart, mRxBuffer, 10);

}


//returns 0 when OK, 1 if transceiver is busy
uint8_t Send(void)
{

	if (mTxBusy == 1)  // check if transciever is ready
	{
		return 1; // error: Tx Busy
	}

	mTxBusy = 1;

	HAL_UART_Transmit_DMA(ComUart, mTxBuffer, 10);

	return 0;
}

void ProcessMessage()
{
		uint16_t varId, sendPeriod;
		uint16_t id = (mRxBuffer[0]<<8) | mRxBuffer[1];

		uint16_t data1, data2, data3, data4;

		switch (id )  // message ID
		{
		case CMD_READ_VAR_REQUEST:
			varId = (mRxBuffer[2]<<8) | mRxBuffer[3];
			sendPeriod = (mRxBuffer[4]<<8) | mRxBuffer[5];
			UpdateScanList(varId, sendPeriod);
			SendVariable(varId);
			break;
		case CMD_TM_SET_ELV:
			data1 = (mRxBuffer[2]<<8) | mRxBuffer[3];
			DO_SetElv(data1);
			break;
		case CMD_TM_SET_PUMPS:
			data1 = (mRxBuffer[2]<<8) | mRxBuffer[3];
			DO_SetPumps(data1);
			break;
		}

	HAL_UART_Receive_DMA(ComUart, mRxBuffer, 10);
	return;
}



void SCOM_Update_10ms(void)
{
	int i;
	for(i = 0; i < 20; i++)
	{
		mScanList[i].timer++;
	}


	for(i = 0; i < 20; i++)
	{
		if (mScanList[i].enable == 1 && mScanList[i].sendPeriod != 0)
		{
			if (mScanList[i].timer >= mScanList[i].sendPeriod)
			{
				SendVariable(mScanList[i].varId);
				mScanList[i].timer = 0;
				break;  // send just 1 value every 10ms
			}
		}
	}
}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == ComUart)
	{
		mTxBusy = 0;
	}
}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == ComUart)
	{
		mNewDataReady = 1;
		mRxLength = 1;
		ProcessMessage();
	}
}

void UpdateScanList(uint16_t varId, uint16_t period)
{
	// go thru the list to find if entry already exists
	int i;
	for(i = 0; i < 20; i++)
	{
		if(mScanList[i].varId == varId)
		{
			if(period != 0)
			{
				mScanList[i].sendPeriod = period;
				mScanList[i].enable = 1;
				return;
			}
			else
			{
				mScanList[i].sendPeriod = 0;
				mScanList[i].enable = 0;
			}
		}
	}

	// if not add variable to the list
	for(i = 0; i < 20; i++)
	{
		if(mScanList[i].enable == 0)
		{
			if(period != 0)
			{
				mScanList[i].varId = varId;
				mScanList[i].sendPeriod = period;
				mScanList[i].enable = 1;
				return;
			}
		}
	}
}

void SendVariable(uint16_t id)
{
	uint16_t invalid = 0;
	int16_t tmp = VAR_GetVariable(id, &invalid);
	id |= invalid;
	mTxBuffer[0] = CMD_TM_VAR_VALUE >> 8;
	mTxBuffer[1] = CMD_TM_VAR_VALUE & 0xFF;
	mTxBuffer[2] = id >> 8;
	mTxBuffer[3] = id  & 0xFF;
	mTxBuffer[4] = tmp >> 8;
	mTxBuffer[5] = tmp & 0xFF;
	//memcpy(&(mTxBuffer[4]),VAR_GetVariablePointer(id), sizeof(int16_t));
	Send();
}



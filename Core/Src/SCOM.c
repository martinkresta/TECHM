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

uint8_t mRxBuffer[COM_BUFLEN];
uint8_t mTxBuffer[COM_BUFLEN];
uint8_t mRxLength, mNewDataReady, mTxBusy;

UART_HandleTypeDef* ComUart;


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

/*	if (mTxBusy == 1)  // check if transciever is ready
	{
		return 1; // error: Tx Busy
	} */

	mTxBusy = 1;

	HAL_UART_Transmit_DMA(ComUart, mTxBuffer, 10);

	return 0;
}

void ProcessMessage()
{
		uint16_t varId;
		uint16_t id = (mRxBuffer[0]<<8) | mRxBuffer[1];

		switch (id )  // message ID
		{
		case CMD_READ_VAR_REQUEST:
			varId = (mRxBuffer[2]<<8) | mRxBuffer[3];
			mTxBuffer[0] = CMD_TM_VAR_VALUE >> 8;
			mTxBuffer[1] = CMD_TM_VAR_VALUE & 0xFF;
			memcpy(&(mTxBuffer[2]),VAR_GetVariablePointer(varId), sizeof(int16_t));
			Send();
			break;
		}

	HAL_UART_Receive_DMA(ComUart, mRxBuffer, 10);
	return;
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

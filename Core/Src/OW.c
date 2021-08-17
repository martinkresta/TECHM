/*
 * OW.c
 *  One wire library with timing realized by timer interrupts
 *  Created on: Feb 21, 2021
 *      Author: Martin
 *  Brief: Modul is based on basic timer interrupts with variable period. i.e TIM6 is used.
 *  			 The timer clock after prescaler should be 10MHz (see TIM_CLK_PER_US)
 *  			 This module rewrites the ARR register of that timer within each interrupt handler, to define duration of next period.
 *
 *  How to Use:
 *  			 Configure GPIO pin as Open Drain Output. And assign the pin in OW.h
 *  			 Configure the basic timer and enable its OVF interrupt, hook the OW_IRQ_Handler to this interrupt
 *
 */

#include "OW.h"
#include "main.h"




uint8_t mBytesToWrite;
uint8_t mBytesToRead;
uint8_t mCurrBit;
uint8_t mCurrByte;

eResetStage mResetStage;
eBitStage mBitStage;
eTransferStage mTrStage;
eTransferType mCurrentTranfer;
uint8_t mPresencePulse;
eOwResult mLastTransferResult;

uint16_t mTimReset[3];
uint16_t mTimWriteBit[3];
uint16_t mTimReadBit[3];
uint16_t cc[3];


uint8_t mTxBuff[12];
uint8_t mRxBuff[12];

int16_t* mResultPtr;

int16_t  mLastTemp;
uint8_t mBusy;
sOwBus mBus[MAX_NUM_OF_BUSES];  // max number of hardware busses is 3

GPIO_TypeDef* mOwPort;   //  variables holding values of just active (selected) bus (quicker access then to array)
uint32_t mOwClearMask;
uint32_t mOwSetMask;


int16_t mTemp;

uint8_t mROM[8];



static void ActivateBus(uint8_t mBusId)
{
	mOwPort = mBus[mBusId].Port;
	mOwClearMask = mBus[mBusId].ClearBitMask;
	mOwSetMask = mBus[mBusId].SetBitMask;
}

//initialization of GPIO, Timer, and timing of OW bus;
void OW_Init(void)
{
//reset list of busses
	uint8_t i;
	for (i = 0; i < MAX_NUM_OF_BUSES; i++)
	{
		mBus[i].Port = NULL;
	}

 // debug support (stop Timer when halted)
	DBGMCU->APB1FZR1 |= DBGMCU_APB1FZR1_DBG_TIM6_STOP;
// configure the OW pin as a open drain output


	OW_TIM->DIER |= TIM_DIER_UIE;
	OW_TIM->PSC = 4;
	OW_TIM->CR1 |= TIM_CR1_ARPE;  // preload enable

	mTimReset[0] = DEL_RES_PULSE * TIM_CLK_PER_US;
	mTimReset[1] = DEL_RES_SAMPLE * TIM_CLK_PER_US;
	mTimReset[2] = DEL_RES_REST * TIM_CLK_PER_US;

	mTimWriteBit[0] = DEL_WB_PULSE * TIM_CLK_PER_US;
	mTimWriteBit[1] = DEL_WB_WRITE * TIM_CLK_PER_US;
	mTimWriteBit[2] = DEL_WB_REST * TIM_CLK_PER_US;

	mTimReadBit[0] = DEL_RB_PULSE * TIM_CLK_PER_US;
	mTimReadBit[1] = DEL_RB_SAMPLE * TIM_CLK_PER_US;
	mTimReadBit[2] = DEL_RB_REST * TIM_CLK_PER_US;

}


// define bus connected to some GPIO pin
void OW_AddBus(uint8_t busId, GPIO_TypeDef* port, uint32_t pin)
{
	if (busId < MAX_NUM_OF_BUSES)
	{
		// store the Bus - Pin assignment
		mBus[busId].Port = port;
		mBus[busId].SetBitMask = pin;
		mBus[busId].ClearBitMask = pin << 16;

		// configure the pin
		 /*Configure GPIO pin Output Level */
		HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);

		GPIO_InitTypeDef GPIO_InitStruct;
		GPIO_InitStruct.Pin = pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(port, &GPIO_InitStruct);
	}
}


void ClearRxBuffer()
{
	mRxBuff[0] = 0;
	mRxBuff[1] = 0;
	mRxBuff[2] = 0;
	mRxBuff[3] = 0;
	mRxBuff[4] = 0;
	mRxBuff[5] = 0;
	mRxBuff[6] = 0;
	mRxBuff[7] = 0;
}

eOwResult OW_ReadSensor(uint8_t busId, uint8_t* address, int16_t* result)
{
	eOwResult res = etr_OK;

	if (mBus[busId].Port == NULL)
	{
		res = etr_UnknownBus;
	}
	else if (mBusy)
	{
		res = etr_Busy;
	}
	else // all OK
	{
		ActivateBus(busId);  // asign active bus
		ClearRxBuffer();
		// initialize transfer parameters
		mResultPtr = result;
		mTxBuff[0] = CMD_MATCH_ROM;
		mTxBuff[1] = address[0];
		mTxBuff[2] = address[1];
		mTxBuff[3] = address[2];
		mTxBuff[4] = address[3];
		mTxBuff[5] = address[4];
		mTxBuff[6] = address[5];
		mTxBuff[7] = address[6];
		mTxBuff[8] = address[7];
		mTxBuff[9] = CMD_READ_SCRATCHPAD;


		mTrStage = ets_Reset;
		mResetStage = ers_ResetPulse;
		mBitStage = ebs_Init;
		mPresencePulse = 0;
		mBytesToWrite = 10;
		mBytesToRead = 2;
		mCurrBit = 0;
		mCurrByte = 0;
		mCurrentTranfer = ett_ReadTemp;

		mBusy = 1;
		OW_TIM->ARR = mTimReset[ers_ResetPulse];  // first timed period
		OW_TIM->CR1 |= TIM_CR1_CEN;
		OW_TIM->EGR = 1;
	}
	mLastTransferResult = res;
	return res;
}


void OW_Read_SingleSensor(uint8_t busId)
{
	OW_ReadSensor(busId, mROM, &mTemp);
}


eOwResult OW_ConvertAll(uint8_t busId)
{
	eOwResult res = etr_OK;

	if (mBus[busId].Port == NULL)
	{
		res = etr_UnknownBus;
	}
	else if (mBusy)
	{
		res = etr_Busy;
	}
	else // all OK
	{
		ActivateBus(busId);  // asign active bus
		ClearRxBuffer();
		// initialize transfer parameters
		mTxBuff[0] = CMD_SKIP_ROM;
		mTxBuff[1] = CMD_CONVERT;

		mTrStage = ets_Reset;
		mResetStage = ers_ResetPulse;
		mBitStage = ebs_Init;
		mPresencePulse = 0;
		mBytesToWrite = 2;
		mBytesToRead = 0;
		mCurrBit = 0;
		mCurrByte = 0;
		mCurrentTranfer = ett_Convert;

		mBusy = 1;
		OW_TIM->ARR = mTimReset[ers_ResetPulse];  // first timed period
		OW_TIM->CR1 |= TIM_CR1_CEN;
		OW_TIM->EGR = 1;
	}
	mLastTransferResult = res;
	return res;
}

eOwResult OW_ReadRom(uint8_t busId)
{
	int16_t dummy;
	eOwResult res = etr_OK;

	if (mBus[busId].Port == NULL)
	{
		res = etr_UnknownBus;
	}
	else if (mBusy)
	{
		res = etr_Busy;
	}
	else // all OK
	{
		ActivateBus(busId);  // asign active bus
		ClearRxBuffer();
		mResultPtr = mROM;
		// initialize transfer parameters
		mTxBuff[0] = CMD_READ_ROM;

		mTrStage = ets_Reset;
		mResetStage = ers_ResetPulse;
		mBitStage = ebs_Init;
		mPresencePulse = 0;
		mBytesToWrite = 1;
		mBytesToRead = 8;
		mCurrBit = 0;
		mCurrByte = 0;
		mCurrentTranfer = ett_ReadRom;

		mBusy = 1;
		OW_TIM->ARR = mTimReset[ers_ResetPulse];  // first timed period
		OW_TIM->CR1 |= TIM_CR1_CEN;
		OW_TIM->EGR = 1;
	}

	mLastTransferResult = res;
	return res;
}


// private fcn called from last IRQ of OW transfer
void TransferComplete()
{
	if (mPresencePulse == 0)
	{
		mLastTransferResult = etr_NotPresent;
	}
	switch (mCurrentTranfer)
	{
		case ett_ReadRom:
			memcpy(mROM, mRxBuff, 8);
			break;
		case ett_Convert:
			// nothing
			break;
		case ett_ReadTemp:
			if (mResultPtr != NULL)
			{
				*mResultPtr = (int16_t)((double)(((uint16_t)mRxBuff[0] | ((uint16_t)mRxBuff[1]) << 8)) / 1.6);
				mResultPtr = NULL; // clear the pointer to prevent overwrite in next cycle.
			}
			break;
	}
	mBusy = 0;
}

eOwResult OW_GetLastTransferResult(void)
{
	return mLastTransferResult;
}

uint8_t* OW_GetRom(void)
{
	return mROM;
}


void OW_IRQHandler(void)
{
	OW_TIM->SR = 0;  // Clear the UIF flag

	if (mTrStage == ets_Write)
	{
		switch (mBitStage)
		{
			case ebs_Init:
				// write 0 to GPIO
				mOwPort->BSRR = mOwClearMask;
				break;
			case ebs_WriteSample:
				// write the bit  value to the GPIO
				if (mTxBuff[mCurrByte] & (1 << mCurrBit))
				{  // write 1
					mOwPort->BSRR = mOwSetMask;
				}
				else
				{
					//write 0;
					mOwPort->BSRR = mOwClearMask;
				}
				break;
			case ebs_Rest:
				// write 1 to GPIO
				mOwPort->BSRR = mOwSetMask;
				// increment CurrBit
				break;
		}

		mBitStage++;
		if (mBitStage <=2)
		{
			OW_TIM->ARR = mTimWriteBit[mBitStage];   // prepare ARR for next bit stage
		}
		else  // go to next bit
		{
			mBitStage = 0;
			OW_TIM->ARR = mTimWriteBit[mBitStage];   // prepare ARR for first stage of next write bit
			mCurrBit++;
			if (mCurrBit > 7)
			{
				mCurrBit = 0;
				mCurrByte++;
				if (mCurrByte >= mBytesToWrite)  // all bytes written
				{
					mCurrByte = 0;
					if (mBytesToRead > 0)
					{
						mTrStage = ets_Read;   // switch to reading stage
						OW_TIM->ARR = mTimReadBit[mBitStage];   // prepare ARR for first stage of first read bit
					}
					else
					{
						OW_TIM->CR1 &= ~TIM_CR1_CEN;
						TransferComplete();
					}

				}
			}
		}

	}
	else if (mTrStage == ets_Read)
	{
		switch (mBitStage)
			{
				case ebs_Init:
					// write 0 to GPIO
					mOwPort->BSRR = mOwClearMask;
					break;
				case ebs_WriteSample:
					// write 1 to GPIO
					mOwPort->BSRR = mOwSetMask;
					break;
				case ebs_Rest:
					// Read the pin value
					if (mOwPort->IDR & mOwSetMask)
					{  // store 1
						mRxBuff[mCurrByte] |= (1 << mCurrBit);
					}
					else  // store 0
					{
						mRxBuff[mCurrByte] &= ~(1 << mCurrBit);
					}
					break;
			}

				mBitStage++;
				if (mBitStage > 2)
				{
					mBitStage = 0;
					mCurrBit++;
					if (mCurrBit > 7)
					{
						mCurrBit = 0;
						mCurrByte++;
						if (mCurrByte >= mBytesToRead)  // all bytes written
						{

							// transaction is finished disable the timer
							OW_TIM->CR1 &= ~TIM_CR1_CEN;
							// copy the received bytes
							TransferComplete();
							mCurrByte = 0;
							mBusy  = 0;
						}
					}
				}

				OW_TIM->ARR = mTimReadBit[mBitStage]; // prepare ARR for next period

	}
	else if (mTrStage == ets_Reset)
	{
		// write new value to ARR register
		switch (mResetStage)
		{
			case ers_ResetPulse:
				// write 0 to GPIO
				mOwPort->BSRR = mOwClearMask;
				break;
			case ers_PresencePulse:
				// write 1 to GPIO
				mOwPort->BSRR = mOwSetMask;
				break;
			case ers_Rest:
				if (mOwPort->IDR & mOwSetMask)
				{  // store 1
					mPresencePulse = 0;
				}
				else  // store 0
				{
					mPresencePulse = 1;
				}
				break;
		}
		mResetStage++;
		if (mResetStage <= 2)   // still in Reset phase
		{
			OW_TIM->ARR = mTimReset[mResetStage];  // prepare period for next reset stage
		}
		else   // switching from reset to write phase
		{
			mResetStage = 0;
			mBitStage = 0;   // reset the bit stage for next phase
			if (mPresencePulse)  // device detected switch to write phase
			{
				mTrStage = ets_Write;
				OW_TIM->ARR = mTimWriteBit[mBitStage];  // prepare period for first write bit stage
			}
			else  // no device on bus - finish transaction
			{
				OW_TIM->CR1 &= ~TIM_CR1_CEN;
				mBusy = 0;
			}
		}
	}

}



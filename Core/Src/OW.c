/*
 * OW.c
 *  One wire library with timing realized by timer interrupts
 *  Created on: Feb 21, 2021
 *      Author: Martin
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
eTransferResult mLastTransferResult;

uint16_t mTimReset[3];
uint16_t mTimWriteBit[3];
uint16_t mTimReadBit[3];
uint16_t cc[3];


uint8_t mTxBuff[12];
uint8_t mRxBuff[12];

int16_t* mResultPtr;


uint8_t mBusy;


/// sensors


int16_t mTemperatures[NUM_OF_SENSORS];
int16_t mTemp;

uint8_t mROM[8];

uint8_t mSensorsAddress[NUM_OF_SENSORS][8] =   // MSB on the left, transmit LSB first!!
{
{0x28,  0x4C,  0xA3,  0x7E,  0x0C,  0x0, 0x0,  0x6D},                      //T_IOBOARD_D

//{0x28, 0xFF, 0xB5, 0x82, 0xB2, 0x15, 0x03, 0x09}, // 9
};





//initialization of GPIO, Timer, and timing of OW bus;
void OW_Init(void)
{

	//DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM6_STOP;


	OW_TIM->DIER |= TIM_DIER_UIE;
	OW_TIM->PSC = 4;

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

eTransferResult OW_ReadSensor(uint8_t* address, int16_t* result)
{
	eTransferResult res = etr_OK;
	if (!mBusy)
	{
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

		OW_TIM->ARR = 1000;
		OW_TIM->CR1 |= TIM_CR1_CEN;
	}
	else
	{
		res = etr_Busy;
	}
	mLastTransferResult = res;
	return res;
}

void OW_Read(uint8_t sensorIndex)
{
	// TBD
	OW_ReadSensor(mSensorsAddress[0], mTemperatures);
}

void OW_Read_SingleSensor(void)
{
	// TBD
	OW_ReadSensor(mROM, mTemperatures);
}



void OW_ConvertAll(void)
{
	if (!mBusy)
	{
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

		OW_TIM->ARR = 1000;
		OW_TIM->CR1 |= TIM_CR1_CEN;
	}
}

void OW_ReadRom(void)
{
	int16_t dummy;


	if (!mBusy)
	{
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

		OW_TIM->ARR = 1000;
		OW_TIM->CR1 |= TIM_CR1_CEN;
	}
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
			}
			break;
	}
	mBusy = 0;
}

eTransferResult OW_GetLastTransferResult(void)
{
	return mLastTransferResult;
}

uint8_t* OW_GetRom(void)
{
	return mROM;
}

int16_t* OW_GetTemp(uint8_t id)
{
	if (id < NUM_OF_SENSORS)
	{
		return &(mTemperatures[id]);
	}
	else
	{
		return NULL;
	}
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
				OW_PORT->BSRR = CLEAR_BIT_MASK;
				break;
			case ebs_WriteSample:
				// write the bit  value to the GPIO
				if (mTxBuff[mCurrByte] & (1 << mCurrBit))
				{  // write 1
					OW_PORT->BSRR = SET_BIT_MASK;
				}
				else
				{
					//write 0;
					OW_PORT->BSRR = CLEAR_BIT_MASK;
				}
				break;
			case ebs_Rest:
				// write 1 to GPIO
				OW_PORT->BSRR = SET_BIT_MASK;
				// increment CurrBit
				break;
		}

		OW_TIM->ARR = mTimWriteBit[mBitStage];
		mBitStage++;
		if (mBitStage > 2)
		{
			mBitStage = 0;
		//	OW_TIM->ARR = mTimWriteBit[mBitStage];
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
						OW_TIM->ARR = mTimReadBit[mBitStage];
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
					OW_PORT->BSRR = CLEAR_BIT_MASK;
					break;
				case ebs_WriteSample:
					// write 1 to GPIO
					OW_PORT->BSRR = SET_BIT_MASK;
					break;
				case ebs_Rest:
					// Read the pin value
				if (OW_PORT->IDR & ONE_WIRE_Pin)
					{  // store 1
						mRxBuff[mCurrByte] |= (1 << mCurrBit);
					}
					else  // store 0
					{
						mRxBuff[mCurrByte] &= ~(1 << mCurrBit);
					}
					break;
			}
				OW_TIM->ARR = mTimReadBit[mBitStage];
				mBitStage++;
				if (mBitStage > 2)
				{
					mBitStage = 0;
				//	OW_TIM->ARR = mTimReadBit[mBitStage];
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

	}
	else if (mTrStage == ets_Reset)
	{
		// write new value to ARR register
		switch (mResetStage)
		{
			case ers_ResetPulse:
				// write 0 to GPIO
				OW_PORT->BSRR = CLEAR_BIT_MASK;
				break;
			case ers_PresencePulse:
				// write 1 to GPIO
				OW_PORT->BSRR = SET_BIT_MASK;
				break;
			case ers_Rest:
				if (OW_PORT->IDR & ONE_WIRE_Pin)
				{  // store 1
					mPresencePulse = 0;
				}
				else  // store 0
				{
					mPresencePulse = 1;
				}
				break;
		}
		OW_TIM->ARR = mTimReset[mResetStage];
		mResetStage++;
		if (mResetStage > 2)
		{
			mResetStage = 0;
			mBitStage = 0;
		//	OW_TIM->ARR = mTimWriteBit[mBitStage];
			if (mPresencePulse)
			{
				mTrStage = ets_Write;
			}
			else
			{  // no device on bus - finish transaction
				OW_TIM->CR1 &= ~TIM_CR1_CEN;
				mBusy = 0;
			}

		}


	}

}



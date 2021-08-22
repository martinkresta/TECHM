/*
 * MCAN.c
 *
 *  Created on: Aug 14, 2021
 *      Author: Martin
 */

#include "MCAN.h"
#include "leds.h"
#include "circbuf.h"


uint16_t Filter_Calc_Mask_Code (uint16_t from,uint16_t to);
void Set_Filter_Id_List (CAN_HandleTypeDef *hcan, uint8_t fifo, uint8_t bank, uint16_t id_1, uint8_t rtr_1, uint16_t id_2, uint8_t rtr_2, uint16_t id_3, uint8_t rtr_3, uint16_t id_4, uint8_t rtr_4);
void Set_Filter_Id_Mask (CAN_HandleTypeDef *hcan, uint8_t fifo,  uint8_t bank, uint16_t id_1_from, uint16_t id_1_to, uint16_t id_2_from, uint16_t id_2_to);

static uint8_t Send(s_CanTxMsg msg);

static CAN_handle mMcan;

static s_CanNode mNodes[MAX_NUM_OF_NODES];



// initialization of CAN node, and buffers
void MCAN_Init(CAN_HandleTypeDef	* can, uint8_t id)
{
	uint8_t bank_offset = 0;
	uint8_t i;

	mMcan.Rx0Buf = CB_Create(sizeof(s_CanRxMsg),RX_NMT_BUFF_SIZE);
	mMcan.Rx1Buf = CB_Create(sizeof(s_CanRxMsg),RX_BUFF_SIZE);
	mMcan.TxBuf = CB_Create(sizeof(s_CanTxMsg),TX_BUFF_SIZE);
	if (mMcan.Rx0Buf == NULL || mMcan.Rx1Buf == NULL ||mMcan.TxBuf == NULL)
	{
		// TBD
	}

	mMcan.CanHw = can;
	// for devices with two cans check which HW is used and adjust filter banks offset
	#ifdef CAN2
	if(mMcan.CanHw->Instance == CAN2)
	{
		bank_offset = 15;
	}
	#endif


	mMcan.Node.nodeId = id;
	mMcan.Node.timeout = CO_DEFAULT_HB_TO/2;
	mMcan.Node.timeout_cnt = 0;

	mMcan.HbMsg.header.DLC = 1;
	mMcan.HbMsg.header.StdId = HB + mMcan.Node.nodeId;
	mMcan.HbMsg.header.ExtId = 0x00;
	mMcan.HbMsg.header.RTR = CAN_RTR_DATA;
	mMcan.HbMsg.header.IDE = CAN_ID_STD;

	// init network
	for (i = 0; i < MAX_NUM_OF_NODES; i++)
	{
		mNodes[i].timeout = CO_DEFAULT_HB_TO;
		mNodes[i].canStatus = eNS_NMT_UNKNOWN;
		mNodes[i].nodeId = i;
	}
	mNodes[mMcan.Node.nodeId].canStatus = mMcan.Node.canStatus;



	// set the filter of FIFO0 for hearbeats (0x700 - 0x70F)
	Set_Filter_Id_Mask(mMcan.CanHw, CAN_FILTER_FIFO0, 1 + bank_offset , HB, HB+ID_RANGE, 0,0); // HeartBeats from all devices => FMI = 0/4

	// set the filter of FIFO1 for all other messages (0x000 - 0x6FF)
	Set_Filter_Id_Mask(mMcan.CanHw, CAN_FILTER_FIFO1, 2 + bank_offset , 0x000, 0x6FF, 0,0);


	HAL_CAN_ActivateNotification(mMcan.CanHw, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING | CAN_IT_LAST_ERROR_CODE| CAN_IT_ERROR);

	mMcan.CanHw->Instance->MCR &= ~0x00010000;	// enable CAN in debug mode (clear DBF bit)
}



// Start CANOPEN protocols
void MCAN_Start(void)
{
	HAL_CAN_Start(mMcan.CanHw);  // start can
	mMcan.Node.canStatus = eNS_NMT_RUN;
	mNodes[mMcan.Node.nodeId].canStatus = mMcan.Node.canStatus;
}

s_CanNode* MCAN_GetNodesPt(void)
{
	return mNodes;
}

// Periodic update function for synchronous CAN handling
void MCAN_Update_10ms(void)
{

	int i;
	s_CanRxMsg msg;
	s_CanTxMsg TxMsg;
	s_CanNode* tmp;
	uint32_t	TxMailbox;


	// Check messages from FIFO 0 (NMT, HB)

	while(0 == CB_Get(mMcan.Rx0Buf,(uint8_t*)&msg))  // process all messages in buffer
	{
		if (msg.header.StdId >= HB && msg.header.StdId <= HB + ID_RANGE) // received HB
		{
			uint8_t nodeId = msg.header.StdId - HB;
			if (nodeId < MAX_NUM_OF_NODES)  // valid heartbeat received
			{
				mNodes[nodeId].canStatus = (e_NodeStatus)msg.data[0];
				mNodes[nodeId].timeout_cnt = 0;
			}
		}

		// Check timeout of all nodes
		for(i = 0; i < MAX_NUM_OF_NODES; i++)
		{
			mNodes[i].timeout_cnt += 10;  // 10ms period of execution of this fcn
			if (mNodes[i].timeout_cnt > mNodes[i].timeout &&  // HB period elapsed
					mNodes[i].canStatus != eNS_NMT_UNKNOWN)
			{
				mNodes[i].timeout_cnt = 0;
				mNodes[i].canStatus = eNS_NMT_UNKNOWN;
				// TBD notify app about elapsed timeout
				//Send(NmtMsg);
			}
		}

	}
	// Send heartbeat if it's time
	mMcan.Node.timeout_cnt += 10;
	if (mMcan.Node.timeout_cnt >= mMcan.Node.timeout)
	{
		mMcan.Node.timeout_cnt = 0;
		mMcan.HbMsg.data[0] = mMcan.Node.canStatus;
		Send(mMcan.HbMsg);
	}

}


// function to be called periodically at a rate of transmitting can messages. (for example every 5 ms)
// One CAN message is is sent if TX buffer is not empty
void MCAN_Transmit(void)
{
	s_CanTxMsg TxMsg;
	s_CanNode* tmp;
	uint32_t	TxMailbox;

	if  (0 == CB_Probe(mMcan.TxBuf,(uint8_t*)&TxMsg))  // fetch the message from the buffer
	{
		if (HAL_OK == HAL_CAN_AddTxMessage(mMcan.CanHw, &TxMsg.header, TxMsg.data, &TxMailbox))  // transmit the message
		{
			CB_Remove(mMcan.TxBuf);  // remove the message from the buffer only if transmission was succesfull
		}
	}
}

// Gets a message from receive buffer to be processed by application
// returns 1 if message is available, otherwise return 0
uint8_t MCAN_GetRxMessage( s_CanRxMsg* msg)
{
	if (0 == CB_Get(mMcan.Rx1Buf,(uint8_t*)msg))
	{
		return 1;
	}
	return 0;
}



// returns CAN satatus of connected node
e_NodeStatus MCAN_GetNodeStatus(uint8_t nodeId)
{
	if (nodeId < MAX_NUM_OF_NODES)
	{
		if (nodeId == mMcan.Node.nodeId)  // this node
		{
			return mMcan.Node.canStatus;
		}
		else   // remote node
		{
			return mNodes[nodeId].canStatus;
		}
	}
	return eNS_NMT_UNKNOWN;
}


// Sends general CAN data message
uint8_t MCAN_SendFrame(uint16_t cobid, uint8_t* data, uint8_t dlc)
{
	s_CanTxMsg msg;
	msg.header.DLC = dlc;
	msg.header.StdId = cobid;
	msg.header.ExtId = 0x00;
	msg.header.RTR = CAN_RTR_DATA;
	msg.header.IDE = CAN_ID_STD;

	memcpy(&msg.data[0], data, dlc);
	return Send(msg);
}


/*************** Private Fcns **************/

// CAN transmit
static uint8_t Send(s_CanTxMsg msg)
{
	// instert to Tx buffer
	CB_Put(mMcan.TxBuf,(uint8_t*) &msg);
}


// interrupt handlers - callbacks
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
 {
	s_CanRxMsg tmp;
	HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &tmp.header, tmp.data);
	if (mMcan.CanHw == hcan)
	{
		CB_Put(mMcan.Rx0Buf,(uint8_t*) &tmp);
		LED_B_SetMode(eLED_BLINK_ONCE);  // blue blink;
	}

}


void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	s_CanRxMsg tmp;
	HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &tmp.header, tmp.data);
	if (mMcan.CanHw == hcan)
	{
		CB_Put(mMcan.Rx1Buf,(uint8_t*) &tmp);
		LED_B_SetMode(eLED_BLINK_ONCE);  // blue blink
	}

}


/*********************************************************************/
void Set_Filter_Id_List (CAN_HandleTypeDef *hcan, uint8_t fifo, uint8_t bank, uint16_t id_1, uint8_t rtr_1, uint16_t id_2, uint8_t rtr_2, uint16_t id_3, uint8_t rtr_3, uint16_t id_4, uint8_t rtr_4)
{
	CAN_FilterTypeDef			 filterInitStructure;

	filterInitStructure.SlaveStartFilterBank  = 14;


	filterInitStructure.FilterBank									= bank;
	filterInitStructure.FilterMode									=	CAN_FILTERMODE_IDLIST;
	filterInitStructure.FilterScale									=	CAN_FILTERSCALE_16BIT;
	filterInitStructure.FilterFIFOAssignment				=	fifo;


	filterInitStructure.FilterIdLow 						= id_1 << 5;
	if(rtr_1)
	{
		filterInitStructure.FilterIdLow 					+= 16;
	}

	filterInitStructure.FilterMaskIdLow 				= id_2 << 5;
	if(rtr_2)
	{
		filterInitStructure.FilterMaskIdLow 			+= 16;
	}

	filterInitStructure.FilterIdHigh 						= id_3 << 5;
	if(rtr_3)
	{
		filterInitStructure.FilterIdHigh 					+= 16;
	}

	filterInitStructure.FilterMaskIdHigh 				= id_4 << 5;
	if(rtr_4)
	{
		filterInitStructure.FilterMaskIdHigh 			+= 16;
	}
	filterInitStructure.FilterActivation				=	ENABLE;


	HAL_CAN_ConfigFilter(hcan,&filterInitStructure);
}

/************************************************************************
  Function	  : CAN_FILTER_Filter_Mask
  Input		 		: -
  Output	  	: -
  Description : -
************************************************************************/
void Set_Filter_Id_Mask (CAN_HandleTypeDef *hcan, uint8_t fifo,  uint8_t bank, uint16_t id_1_from, uint16_t id_1_to, uint16_t id_2_from, uint16_t id_2_to)
{
	CAN_FilterTypeDef			 filterInitStructure;

	filterInitStructure.SlaveStartFilterBank  = 14;


	filterInitStructure.FilterBank									= bank;


	filterInitStructure.FilterMode									=	CAN_FILTERMODE_IDMASK;
	filterInitStructure.FilterScale									=	CAN_FILTERSCALE_16BIT;
	filterInitStructure.FilterFIFOAssignment				=	fifo;

	filterInitStructure.FilterIdHigh								=	id_2_from << 5;
	filterInitStructure.FilterIdLow 								=	id_1_from << 5;
	filterInitStructure.FilterMaskIdHigh						= Filter_Calc_Mask_Code(id_2_from,id_2_to);
	filterInitStructure.FilterMaskIdLow 						= Filter_Calc_Mask_Code(id_1_from,id_1_to);

	filterInitStructure.FilterActivation						=	ENABLE;

	HAL_CAN_ConfigFilter(hcan,&filterInitStructure);
}

/************************************************************************
  Function	  : CAN_FILTER_Calc_Mask_Code
  Input		 		: -
  Output	  	: -
  Description : -
************************************************************************/
uint16_t Filter_Calc_Mask_Code (uint16_t from,uint16_t to)
{
	uint16_t	help			 	= 0x00;
	uint8_t		shift_cnt		= 0x00;
	uint8_t			first			= FALSE;

	for (shift_cnt = 0; shift_cnt < 16; shift_cnt++)
	{
		if(((from & 0x8000) == (to & 0x8000)) && (!first))
		{
			from = from << 1;
			to = to << 1;
		}
		else
		{
			help = help << 1;
			first = TRUE;
			help |= 0x01;

			from = from << 1;
			to = to << 1;
		}
	}
	help = (~help) << 5;
	return help;
}


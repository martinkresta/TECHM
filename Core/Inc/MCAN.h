/*
 * MCAN.h
 *
 *  Created on: Aug 14, 2021
 *      Author: Martin
 */


#ifndef INC_MCAN_H_
#define INC_MCAN_H_


#include "main.h"
#include "circbuf.h"


/// boolean for false condition
#ifndef FALSE
#define FALSE (0)
#endif
/// boolean for true condition
#ifndef TRUE
#define TRUE  (1)
#endif

#define RX_NMT_BUFF_SIZE 200
#define RX_BUFF_SIZE 80
#define TX_BUFF_SIZE 100      // needs 6,4kB of free Heap = 0x1900
#define RX_BUFF_WARN RX_BUFF_SIZE/2
#define TX_BUFF_WARN TX_BUFF_SIZE/2
#define MAX_NUM_OF_NODES	8
#define CO_DEFAULT_HB_TO	2000  // 2sec


#define ID_RANGE	0x0F    // maximal nodeID



#define NMT 0x000
#define HB	0x700

#define NODEID_IOBOARD_D			1
#define NODEID_IOBOARD_U			2
#define NODEID_TECHM		 			3
#define NODEID_ELECON					4
#define NODEID_RECU 					5



typedef struct
{
	CAN_RxHeaderTypeDef header;
	uint8_t data[8];
}s_CanRxMsg;

typedef struct
{
	CAN_TxHeaderTypeDef header;
	uint8_t data[8];
}s_CanTxMsg;

typedef enum
{
    eNS_NMT_STOPPED                = 0x00,
    eNS_NMT_RUN				             = 0x01,
		eNS_NMT_UNKNOWN								 = 0xFF
}e_NodeStatus;


typedef enum
{
	eET_NONE = 0,
	eET_MALLOC_FAIL,
	eET_RX0_BUFF_WARN,
	eET_RX1_BUFF_WARN,
	eET_TX_BUFF_WARN,
	eET_RX0_BUFF_FULL,
	eET_RX1_BUFF_FULL,
	eET_TX_BUFF_FULL
}e_ErrType;


typedef struct
{
	uint8_t											nodeId;
	uint16_t               	 		timeout_cnt;
	uint16_t               	 		timeout;
	e_NodeStatus           	 		canStatus;
}s_CanNode;


typedef struct
{
	s_CanNode Node;  						// this node
	CAN_HandleTypeDef	*CanHw;		// underlying CAN driver HAL handle
	CB_handle	 Rx0Buf;
	CB_handle	 Rx1Buf;
	CB_handle	 TxBuf;						// TX buffer
	s_CanTxMsg HbMsg;						// prepared template for heartbeat Ms
	s_CanNode	 *SlaveNodes;		  // Array for holding slave nodes
}CAN_handle;


void MCAN_Init(CAN_HandleTypeDef	* can, uint8_t id);

void MCAN_Update_10ms(void);

void MCAN_Transmit(void);

void MCAN_Start(void);

e_NodeStatus MCAN_GetNodeStatus(uint8_t nodeId);

uint8_t MCAN_GetRxMessage(s_CanRxMsg* msg);

uint8_t MCAN_SendFrame(uint16_t cobid, uint8_t* data, uint8_t dlc);


// Required interface ! - This fcns have to be defined in application
//void MCAN_NotifySlaveStateReceived(uint8_t nodeid, e_NodeStatus status);

#endif /* INC_MCAN_H_ */

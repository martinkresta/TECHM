/*
 * COM.h
 *
 *  Created on: Aug 14, 2021
 *      Author: Martin
 */

#ifndef INC_COM_H_
#define INC_COM_H_

#include "main.h"


// CAN ids
#define  CMD_TM_DEV_ID  0x210
#define  CMD_TM_STATUS  0x212
#define  CMD_TM_VAR_VALUE  0x221


#define  CMD_BUTTON_STATE  0x100
#define  CMD_VAR_VALUE		 0x110

#define  STREAM_LIST_SIZE		30


void COM_Init(uint8_t nodeId);

void COM_Update_10ms(void);

uint8_t COM_GetNodeStatus(uint8_t nodeId);
int16_t COM_GetNetworkStatus(void);

#endif /* INC_COM_H_ */

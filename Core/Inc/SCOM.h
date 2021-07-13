/*
 * SCOM.h
 *
 *  Created on: Jul 13, 2021
 *      Author: Martin
 *      Brief:  Universal serial communication protocol between device and computer.
 *      Resources: DMA channels 5+4, USART1,
 */

#ifndef INC_SCOM_H_

#include "main.h"


#define SCOM_BAUDRATE 57600

#define COM_UART USART1  // do not change used UART!
#define COM_BUFLEN	10



// all commands has to fit to 11-bit value .  0x0000 - 0x07FF
// PC application commands   range 0x
#define  CMD_GET_DEV_ID  					0x710
#define  CMD_GET_STATUS  					0x712
#define  CMD_READ_VAR_REQUEST  		0x713
#define  CMD_TM_SET_POWER_OUTPUTS 0x721
#define  CMD_TM_SET_ELV 					0x722
#define  CMD_TM_SET_AV  					0x723
#define  CMD_TM_SET_SERVOVALVES  	0x724
#define  CMD_TM_SET_PUMPS 				0x725

#define  CMD_TM_DEV_ID  0x210
#define  CMD_TM_STATUS  0x212
#define  CMD_TM_VAR_VALUE  0x221



#define TEMP_READ_ROM	 0x51
#define TEMP_CONVERT	 0x52
#define TEMP_GET_TEMP	 0x53
#define TEMP_READ_TEMP	 0x54



void SCOM_Init(UART_HandleTypeDef* uart);

#define INC_SCOM_H_



#endif /* INC_SCOM_H_ */

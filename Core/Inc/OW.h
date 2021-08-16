/*
 * OW.h
 *
 *  Created on: Feb 21, 2021
 *      Author: Martin
 */

#ifndef INC_OW_H_
#define INC_OW_H_

#include "main.h"

#define TIM_CLK_PER_US		10		// timer clock per microseconds

#define DEL_RES_PULSE		  480  // 480
#define DEL_RES_SAMPLE		100
#define DEL_RES_REST			380

#define DEL_RB_PULSE			1 //3//2
#define DEL_RB_SAMPLE			11 //15//11
#define DEL_RB_REST			  60 //55//63

#define DEL_WB_PULSE			6
#define DEL_WB_WRITE			65 //60 //54
#define DEL_WB_REST			  15


#define CMD_SEARCH_ROM				0xF0
#define CMD_READ_ROM					0x33
#define CMD_MATCH_ROM					0x55
#define CMD_SKIP_ROM					0xCC
#define CMD_ALARM_SEARCH			0xEC

#define CMD_CONVERT						0x44
#define CMD_WRITE_SCRATCHPAD	0x4E
#define CMD_READ_SCRATCHPAD		0xBE


#define ONE_WIRE_Pin 								OW1_Pin
#define ONE_WIRE_GPIO_Port 					OW1_GPIO_Port

#define OW_TIM	TIM6
#define OW_PORT ONE_WIRE_GPIO_Port

#define SET_BIT_MASK   (uint32_t)ONE_WIRE_Pin;
#define CLEAR_BIT_MASK   ((uint32_t)ONE_WIRE_Pin) << 16;



#define NUM_OF_SENSORS	1

typedef enum
{
	ets_Reset = 0,
	ets_Write,
	ets_Read
}eTransferStage;


typedef enum
{
	ebs_Init = 0,
	ebs_WriteSample,
	ebs_Rest
}eBitStage;

typedef enum
{
	ers_ResetPulse = 0,
	ers_PresencePulse,
	ers_Rest
}eResetStage;

typedef enum
{
	ett_ReadRom = 0,
	ett_ReadTemp,
	ett_Convert
}eTransferType;


typedef enum
{
	etr_OK = 0,
	etr_Busy,
	etr_NotPresent,
	etr_ComFailure
} eTransferResult;

void OW_Init(void);

eTransferResult OW_ReadSensor(uint8_t* address, int16_t* result);

void OW_IRQHandler(void);

void OW_Read(uint8_t sensorIndex);

void OW_ConvertAll(void);

void OW_ReadRom(void);

uint8_t* OW_GetRom(void);

int16_t* OW_GetTemp(uint8_t id);

void OW_Read_SingleSensor(void);

eTransferResult OW_GetLastTransferResult(void);

#endif /* INC_OW_H_ */

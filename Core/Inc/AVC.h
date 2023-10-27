/*
 * RPISERP.h
 *
 *  Created on: 16. 10. 2022
 *      Author: marti
 *      Brief:

 */

#ifndef INC_AVC_H_
#define INC_AVC_H_

#include "main.h"
#include <stdbool.h>


typedef enum
{
  evs_Stopped,
  evs_RunOpen,
  evs_RunClose,
  evs_Homing   // going to home position
}eValveState;

typedef enum
{
  evd_Opening,
  evd_Closing
}eValveDir;


#define AVC_DEFAULT_DUTYCYCLE   55
#define AVC_MIN_DUTYCYCLE       8

#define AVC_POS_TOLERANCE  2
#define AVC_VALVE_MECH_PLAY  4    // compensation of mechanical play

#define AVC_FULL_OPEN_PCT     70
#define AVC_FULL_CLOSE_PCT    0
#define AVC_MINIMAL_OPEN_PCT  18
#define AVC_DEFAULT_PCT       45
#define AVC_COOLDOWN_PCT      AVC_MINIMAL_OPEN_PCT


void AVC_Init(void);

void AVC_SetRequestPos(uint16_t pos);

void AVC_Update_10ms(void);


void AVC_GoHome(void);

void AVC_ExtiCallback(uint16_t GPIO_Pin);




#endif /* INC_RPISERP_H_ */

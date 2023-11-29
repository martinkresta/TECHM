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



#define   AVC_PCT_HOME             0
#define   AVC_PCT_FULL_OPEN        80
#define   AVC_POS_TOLERANCE        0


#define   AVC_SERVO_ANGLE_OFFSET_D      3         // angle between axis of tubing and mounted servo drive (ideally 0)


void AVC_Init(void);

int16_t AVC_GetValvePos(void);
int16_t AVC_GetRequestPos(void);

void AVC_SetRequestPos(uint16_t pos);
void AVC_ChangePosBy(int16_t pos_diff);

void AVC_Update_10ms(void);

void AVC_GoHome(void);



#endif /* INC_RPISERP_H_ */

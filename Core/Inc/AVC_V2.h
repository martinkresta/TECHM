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



#define   AVC_PCT_HOME             5
#define   AVC_PCT_FULL_OPEN        90
#define   AVC_POS_TOLERANCE        0

/*#define AVC_FULL_OPEN_PCT     55
#define AVC_FULL_CLOSE_PCT    0
#define AVC_MINIMAL_OPEN_PCT  18
#define AVC_DEFAULT_PCT       45
#define AVC_COOLDOWN_PCT      AVC_MINIMAL_OPEN_PCT  */


void AVC_Init(void);

int16_t AVC_GetValvePos(void);
int16_t AVC_GetRequestPos(void);

void AVC_SetRequestPos(uint16_t pos);
void AVC_ChangePosBy(int16_t pos_diff);

void AVC_Update_10ms(void);

void AVC_GoHome(void);






#endif /* INC_RPISERP_H_ */

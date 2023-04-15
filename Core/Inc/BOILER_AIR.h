/*
 * BOILER_AIR.h
 *
 *  Created on: 29. 3. 2023
 *      Author: marti
 */

#ifndef BOILER_AIR_H
#define BOILER_AIR_H

#include "main.h"
#include <stdbool.h>

#define HEATUP_MAX_TEMP       200
#define HEATUP_TIMEOUT        (60*30)   // 30 minutes

#define COOLDOWN_ENTER_TEMP   140
#define COOLDOWN_LEAVE_TEMP   180

#define STOP_TEMP             80


#define AV_FULL_CLOSE_PCT   0

#define CONTROL_PERIOD      3  // 3sec



typedef enum
{
  ecm_Manual,
  ecm_Auto
}eControlMode;


typedef enum
{
  es_Off,
  es_Idle,
  es_HeatUp,
  es_AirControl,
  es_CoolDown,
}eState;

// cyclic update function executing auto control algorithm
void BAC_Update_1s(void);

// activates automatic air valve control
void BAC_SetAutoMode(void);

// event door open detected
void BAC_DoorOpenDetected(void);

// emergency closing the boiler air (oveheating)
void BAC_SafetyCloseAir(void);

// start the new hating cycle
void BAC_StartHeating(void);

// Activates the manual control mode and toggles the vlave state (full open/full close)
void BAC_ManualToggle(void);




#endif

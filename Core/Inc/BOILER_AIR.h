/*
 * BOILER_AIR.h
 *
 *  Created on: 29. 3. 2023
 *      Author: marti
 */

#ifndef BOILER_AIR_H
#define BOILER_AIR_H

#include "main.h"
#include "AVC_V2.h"
#include <stdbool.h>

#define HEATUP_MAX_TEMP       196
#define CONTROL_MIN_TEMP      200
#define CONTROL_MAX_TEMP      205

#define HEATUP_TIMEOUT        (60*30)   // 30 minutes

#define COOLDOWN_ENTER_TEMP   140
#define COOLDOWN_LEAVE_TEMP   190

#define STOP_TEMP             80


#define BAC_FULL_CLOSE_PCT    AVC_PCT_HOME
#define BAC_CTRL_MAX_PCT      55
#define BAC_CTRL_MIN_PCT      15
#define BAC_DEFAULT_PCT       35
#define BAC_COOLDOWN_PCT      BAC_CTRL_MIN_PCT
#define BAC_HEATUP_PCT        70

//#define AV_FULL_CLOSE_PCT   0

#define CONTROL_PERIOD      10  // 10sec



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

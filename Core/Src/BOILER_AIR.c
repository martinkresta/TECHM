

#include "BOILER_AIR.h"
#include "AVC_V2.h"
#include "VARS.h"
#include "PID.h"



static eControlMode mBacMode;
static eState mBacState;
static uint16_t mRequestValvePct;
static uint16_t mBacStateTimer;
//static int16_t mConVal;  // actual value of controlled variable
//static int16_t mLastConVal;  // last value of controlled variable

//static int16_t mConErr;  // control error
//static int16_t mConValDiff; // time derivation of controlled variable
static uint16_t mControlLogicTimer;  // measuring period of valve control logic


uint16_t mExhaustTemp_C;

float mPidSetpoint_C;

tPid mExhaustPid;

void SetState(eState newState);
void ControlLogic(uint16_t temp);
void ControlAlgoV2(void);

// Init Function
void BAC_Init(void)
{
  // configure the PID controller
  mExhaustPid.maxAction = BAC_CTRL_MAX_PCT - BAC_DEFAULT_PCT;
  mExhaustPid.minAction = BAC_CTRL_MIN_PCT - BAC_DEFAULT_PCT;
  mExhaustPid.period_s = BAC_PID_PERIOD;
  mExhaustPid.pFactor = BAC_PID_KP;
  mExhaustPid.iFactor = BAC_PID_KI;
  mExhaustPid.dFactor = BAC_PID_KD;

}

// cyclic update function executing auto control algorithm
void BAC_Update_1s(void)
{
  uint16_t invalid;
  mExhaustTemp_C = ( VAR_GetVariable(VAR_TEMP_BOILER_EXHAUST, &invalid) / 10);

  if(mBacStateTimer < 10000)
  {
    mBacStateTimer ++;
  }

  switch (mBacState)
  {
    case es_Off:
      // manual control, do nothing
      if(mBacMode == ecm_Auto)
      {
        AVC_SetRequestPos(mRequestValvePct);
      }
      break;
    case es_Idle:
      // only waiting for start signal
      if(mBacMode == ecm_Auto)
      {
        AVC_SetRequestPos(mRequestValvePct);
      }
      break;
    case es_HeatUp:
      ControlAlgoV2();
      if(mBacMode == ecm_Auto)
      {
        AVC_SetRequestPos(mRequestValvePct);
      }
      if(mExhaustTemp_C > BAC_DEF_HEATUP_TEMP)
      {
        SetState(es_AirControl);
      }
      if(mBacStateTimer >= HEATUP_TIMEOUT)
      {
        SetState(es_AirControl);
      }
      break;
    case es_AirControl:
      //ControlLogic(mExhaustTemp_C);
      ControlAlgoV2();
      if(mBacMode == ecm_Auto)
      {
        AVC_SetRequestPos(mRequestValvePct);
      }
      if(mExhaustTemp_C < BAC_DEF_COOLDOWN_ENTER_TEMP)
      {
        SetState(es_CoolDown);
      }
      break;
    case es_CoolDown:
      if(mExhaustTemp_C > BAC_DEF_COOLDOWN_LEAVE_TEMP)
      {
        SetState(es_AirControl);
      }
      if(mExhaustTemp_C < BAC_DEF_STOP_TEMP)
      {
        SetState(es_Idle);
      }
      if(mBacMode == ecm_Auto)
      {
        AVC_SetRequestPos(mRequestValvePct);
      }
      break;
  }
}

// activates automatic air valve control
void BAC_SetAutoMode(void)
{
  mBacMode = ecm_Auto;
  if (mExhaustTemp_C < 60)
  {
    SetState(es_Idle);
  }
  else
  {
    SetState(es_AirControl);
  }
}

// event door open detected
void BAC_DoorOpenDetected(void)
{
  if (mBacState == es_AirControl || mBacState == es_CoolDown)
  {
    SetState(es_HeatUp);
  }
}

// emergency closing the boiler air (overheating)
void BAC_SafetyCloseAir(void)
{
  SetState(es_Idle);
  mRequestValvePct = BAC_FULL_CLOSE_PCT;
  AVC_GoHome();
}

// start the new hating cycle
void BAC_StartHeating(void)
{
  if(mBacState == es_Idle)
  {
    SetState(es_HeatUp);
  }
}

// Activates the manual control mode and toggles the vlave state (full open/full close)
void BAC_ManualToggle(void)
{
  mBacMode = ecm_Manual;
  SetState(es_Off);
  if(mRequestValvePct < 50)
  {
    mRequestValvePct = AVC_PCT_FULL_OPEN;
    AVC_SetRequestPos(mRequestValvePct);
  }
  else
  {
    mRequestValvePct = AVC_PCT_HOME;
    AVC_SetRequestPos(mRequestValvePct);
    AVC_GoHome();
  }

}


void SetState(eState newState)
{
  switch (newState)
  {
    case es_Off:  // (manual control)
      mBacState = es_Off;
      break;
    case es_Idle:
      mRequestValvePct = BAC_FULL_CLOSE_PCT;
      AVC_SetRequestPos(BAC_FULL_CLOSE_PCT);
      AVC_GoHome();
      mBacState = es_Idle;
      break;
    case es_HeatUp:
      PID_Init(&mExhaustPid);
      mRequestValvePct = BAC_HEATUP_PCT;
      AVC_SetRequestPos(BAC_HEATUP_PCT);
      mPidSetpoint_C = BAC_DEF_HEATUP_TEMP;
      mBacState = es_HeatUp;
      break;
    case es_AirControl:
      mControlLogicTimer = 0;
      mRequestValvePct = BAC_DEFAULT_PCT;
      AVC_SetRequestPos(BAC_DEFAULT_PCT);
      mPidSetpoint_C = BAC_DEF_CONTROL_TEMP;
      mBacState = es_AirControl;
      break;
    case es_CoolDown:
      mRequestValvePct = BAC_COOLDOWN_PCT;
      AVC_SetRequestPos(BAC_COOLDOWN_PCT);
      mBacState = es_CoolDown;
      break;
  }
  mBacStateTimer = 0;
}


void ControlAlgoV2(void)
{
  // read the regulated variable
  uint16_t invalid;
  int16_t action;
  float ExhaustTemp_C, err;


  if(mControlLogicTimer < BAC_CONTROL_PERIOD) // control period not occurred
  {
    mControlLogicTimer++;
    //mFilterSum += temp;
  }
  else   // do the control magic now
  {
    mControlLogicTimer = 1;

    ExhaustTemp_C = (VAR_GetVariable(VAR_TEMP_BOILER_EXHAUST, &invalid) / 10.0);
    // calculate the error
    err = ExhaustTemp_C - mPidSetpoint_C;
    // execute the PID
    action = (int16_t)PID_Update(&mExhaustPid, err);
    // adjust the action variable (offset)
    action += BAC_DEFAULT_PCT;
    // Set the flap
    if(action > BAC_CTRL_MAX_PCT)
     {
      action = BAC_CTRL_MAX_PCT;
     }
     if(action < BAC_CTRL_MIN_PCT)
     {
       action = BAC_CTRL_MIN_PCT;
     }
    mRequestValvePct = (uint16_t)(action);
  }



 // AVC_SetRequestPos((uint16_t)(action));
}
/*
void ControlLogic(uint16_t temp)
{
  mRequestValveDiffPct = 0;

  if(mControlLogicTimer < BAC_CONTROL_PERIOD) // control period not occurred
  {
    mControlLogicTimer++;
    mFilterSum += temp;
  }
  else   // do the control magic now
  {
    mControlLogicTimer = 0;
    mConVal = mFilterSum / BAC_CONTROL_PERIOD;  // calculate filtered value
    mConValDiff = mConVal - mLastConVal;
    mLastConVal = mConVal;
    mFilterSum = 0;
    mConErr = 0;

    if(mConVal > CONTROL_MAX_TEMP)  // band of non-sensitivity
    {
      mConErr = mConVal - CONTROL_MAX_TEMP;
    }
    else if(mConVal < CONTROL_MIN_TEMP)
    {
      mConErr = mConVal - CONTROL_MIN_TEMP;
    }

    // control logic

    if (AVC_GetValvePos() <  BAC_CTRL_MAX_PCT)  // opening enabled
    {
      // if temp is too low but is raising, do nothing
      if(mConErr < 0  && mConValDiff > 1)
      {
        mRequestValveDiffPct = 0;
      }
      //if temp is too low and is  falling
      if(mConErr < 0 &&  mConValDiff < -1)
      {
        mRequestValveDiffPct = 2;
        //mRequestValvePct += 3;   // open the valve more
      }
      // temp is too low and is steady
      else if (mConErr < 0)
      {
        mRequestValveDiffPct = 1;
        //mRequestValvePct += 1;   // open the valve bit more
      }
    }


    if (AVC_GetValvePos() > BAC_CTRL_MIN_PCT) // closing enabled
    {
       // if temp is too high  but is falling, do nothing
       if(mConErr > 0  && mConValDiff < -1)
       {
         mRequestValveDiffPct = 0;
       }

       //if temp is too high and is  rising
       else if(mConErr > 0 &&  mConValDiff < -1)
       {
         if (AVC_GetValvePos() > 34)
         {
           mRequestValveDiffPct = 32 - AVC_GetValvePos();
         }
         else
         {
           mRequestValveDiffPct = -2;
         }
         //mRequestValvePct -= 3;   // close the valve more
       }
       // temp is too high and is steady
       else if (mConErr > 0)
       {
         if (AVC_GetValvePos() > 34)
          {
            mRequestValveDiffPct = 32 - AVC_GetValvePos();
          }
          else
          {
            mRequestValveDiffPct = -1;
          }
         //mRequestValvePct -= 1;   // close the valve bit more
       }
    }
  }


  // limit the requested valve position to Control limits

  if(mRequestValvePct > BAC_CTRL_MAX_PCT)
  {
    mRequestValvePct = BAC_CTRL_MAX_PCT;
  }
  if(mRequestValvePct < BAC_CTRL_MIN_PCT)
  {
    mRequestValvePct = BAC_CTRL_MIN_PCT;
  }

}*/

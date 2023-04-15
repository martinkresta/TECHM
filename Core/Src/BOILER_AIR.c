

#include "BOILER_AIR.h"
#include "AVC.h"
#include "VARS.h"



static eControlMode mMode;
static eState mState;
static uint16_t mRequestValvePct;
static uint16_t mStateTimer;
static int16_t mConVal;  // actual value of controlled variable
static int16_t mLastConVal;  // last value of controlled variable

static int16_t mConErr;  // control error
static int16_t mConValDiff; // time derivation of controlled variable

static uint16_t mFilterSum;

volatile uint16_t mExhaustTemp_C;

void SetState(eState newState);

void ControlLogic(uint16_t temp);

// cyclic update function executing auto control algorithm
void BAC_Update_1s(void)
{
  uint16_t invalid;
  mExhaustTemp_C = ( VAR_GetVariable(VAR_TEMP_BOILER_EXHAUST, &invalid) / 10);

  if(mStateTimer < 10000)
  {
    mStateTimer ++;
  }



  switch (mState)
  {
    case es_Off:
      // manual control, do nothing
      break;
    case es_Idle:
      // only waiting for start signal
      break;
    case es_HeatUp:
      if(mExhaustTemp_C > HEATUP_MAX_TEMP)
      {
        SetState(es_AirControl);
      }
      if(mStateTimer >= HEATUP_TIMEOUT)
      {
        SetState(es_Idle);
      }
      break;
    case es_AirControl:
      ControlLogic(mExhaustTemp_C);
      AVC_SetRequestPos(mRequestValvePct);
      if(mExhaustTemp_C < COOLDOWN_ENTER_TEMP)
      {
        SetState(es_CoolDown);
      }
      break;
    case es_CoolDown:
      if(mExhaustTemp_C > COOLDOWN_LEAVE_TEMP)
      {
        SetState(es_AirControl);
      }
      if(mExhaustTemp_C < STOP_TEMP)
      {
        SetState(es_Idle);
      }

      break;
  }

  if(mMode == ecm_Auto)
  {
    AVC_SetRequestPos(mRequestValvePct);
  }
}

// activates automatic air valve control
void BAC_SetAutoMode(void)
{
  mMode = ecm_Auto;
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
  if (mState == es_AirControl || mState == es_CoolDown)
  {
    SetState(es_HeatUp);
  }
}

// emergency closing the boiler air (oveheating)
void BAC_SafetyCloseAir(void)
{
  SetState(es_Idle);
  mRequestValvePct = AVC_FULL_CLOSE_PCT;
  AVC_SetRequestPos(mRequestValvePct);
  AVC_GoHome();
}

// start the new hating cycle
void BAC_StartHeating(void)
{
  if(mState == es_Idle)
  {
    SetState(es_HeatUp);
  }
}

// Activates the manual control mode and toggles the vlave state (full open/full close)
void BAC_ManualToggle(void)
{
  mMode = ecm_Manual;
  SetState(es_Off);
  if(mRequestValvePct < 50)
  {
    mRequestValvePct = AVC_FULL_OPEN_PCT;
    AVC_SetRequestPos(mRequestValvePct);
  }
  else
  {
    mRequestValvePct = AVC_FULL_CLOSE_PCT;
    AVC_SetRequestPos(mRequestValvePct);
    AVC_GoHome();
  }

}


void SetState(eState newState)
{
  switch (newState)
  {
    case es_Off:  // (manual control)
      mState = es_Off;
      break;
    case es_Idle:
      mRequestValvePct = AVC_FULL_CLOSE_PCT;
      AVC_SetRequestPos(AVC_FULL_CLOSE_PCT);
      AVC_GoHome();
      mState = es_Idle;
      break;
    case es_HeatUp:
      mRequestValvePct = AVC_FULL_OPEN_PCT;
      AVC_SetRequestPos(AVC_FULL_OPEN_PCT);
      mState = es_HeatUp;
      break;
    case es_AirControl:
      mRequestValvePct = AVC_DEFAULT_PCT;
      AVC_SetRequestPos(AVC_DEFAULT_PCT);
      mState = es_AirControl;
      break;
    case es_CoolDown:
      mRequestValvePct = AVC_COOLDOWN_PCT;
      AVC_SetRequestPos(AVC_COOLDOWN_PCT);
      mState = es_CoolDown;
      break;
  }
  mStateTimer = 0;
}

void ControlLogic(uint16_t temp)
{

  static uint16_t controlTimer;

  if(controlTimer < CONTROL_PERIOD) // control period not occurred
  {
    controlTimer++;
    mFilterSum += temp;
  }
  else   // do the control magic now
  {
    controlTimer = 0;
    mConVal = mFilterSum / CONTROL_PERIOD;  // calculate filtered value
    mConValDiff = mConVal - mLastConVal;
    mLastConVal = mConVal;
    mFilterSum = 0;
    mConErr = 0;

    if(mConVal > HEATUP_MAX_TEMP)  // band of non-sensitivity
    {
      mConErr = mConVal - HEATUP_MAX_TEMP;
    }
    else if(mConVal < COOLDOWN_LEAVE_TEMP)
    {
      mConErr = mConVal - COOLDOWN_LEAVE_TEMP;
    }

    // control logic
    // if temp is too low but is raising, do nothing
    if(mConErr < 0  && mConValDiff > 1)
    {
      // do nothing
    }
    //if temp is too low and is  falling
    else if(mConErr < 0 &&  mConValDiff < -1)
    {
      mRequestValvePct += 3;   // open the valve more
    }
    // temp is too low and is steady
    else if (mConErr < 0)
    {
      mRequestValvePct += 1;   // open the valve bit more
    }

    // if temp is too high  but is falling, do nothing
    if(mConErr > 0  && mConValDiff < -1)
    {
      // do nothing
    }
    //if temp is too high and is  rising
    else if(mConErr > 0 &&  mConValDiff < -1)
    {
      mRequestValvePct -= 3;   // close the valve more
    }
    // temp is too high and is steady
    else if (mConErr > 0)
    {
      mRequestValvePct -= 1;   // close the valve bit more
    }
  }


  // limit the requested valve position to Control limits

  if(mRequestValvePct > AVC_FULL_OPEN_PCT)
  {
    mRequestValvePct = AVC_FULL_OPEN_PCT;
  }
  if(mRequestValvePct < AVC_MINIMAL_OPEN_PCT)
  {
    mRequestValvePct = AVC_MINIMAL_OPEN_PCT;
  }

}

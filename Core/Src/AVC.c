/*
 * AVC.c
 *
 *  Created on: Mar 4, 2023
 *      Author: marti
 *      Brief: Low-level Air valve driver, Takes a requested valve position in percents and bahaves like simple servo mechanism.
 *
 *
 *     TECHM wiring :
 *            H-bridge enable, PWM output, PB3, TIM2_CH2
 *            H-bridge inputs, PD2, PC12, PC11, PC10                (AV out connector)
 *            HomePos switch : AV_HOME,  PC9, EXTI9 interrupt       (AVO connector)
 *            Encoder1 AV_ENC1 : GPIO2, PA10, EXTI10 (both edges)   (GPIO connector)
 *            Encoder2 AV_ENC2 : GPIO1, PB1, GPIO input             (GPIO connector)
 *                    Encoder GND                                   (GPIO connector)
 *                    Encoder 3V3                                   (GPIO connector)
 *
 *
 */


#include "AVC.h"
#include "main.h"
#include "stdlib.h"
#include "di.h"

TIM_OC_InitTypeDef mPwmChannel = {0};
GPIO_InitTypeDef GPIO_InitStruct = {0};

int32_t mValvePosRaw;
int16_t mValvePosPct;
int16_t mRequestPosPct;
int16_t mIterm;

eValveState mValveState;
eValveDir mLastDirection;  // direction of last movement

void RunOpen(void);
void RunClose(void);
void Stop(void);


// initialize the peripherals and close the valve
void AVC_Init(void)
{
  mPwmChannel.OCMode = TIM_OCMODE_FORCED_INACTIVE;
  mPwmChannel.Pulse = 0;
  mPwmChannel.OCPolarity = TIM_OCPOLARITY_HIGH;
  mPwmChannel.OCFastMode = TIM_OCFAST_DISABLE;

  GPIO_InitStruct.Pin = AV_PWM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  htim2.Instance->CCMR1 &= ~TIM_CCMR1_OC2M;
  htim2.Instance->CCMR1 |= (TIM_OCMODE_FORCED_INACTIVE <<8);
  htim2.Instance->CCR2 = AVC_DEFAULT_DUTYCYCLE;
  htim2.Instance->CCER |= TIM_CCER_CC2E;
  htim2.Instance->CR1 |= TIM_CR1_CEN;

  mValvePosRaw = 0;
  mValveState = evs_Stopped;

  mValvePosPct = 100;
  AVC_GoHome();


}

void AVC_Update_10ms(void)
{
  int16_t max_Iterm;
  mValvePosPct = (mValvePosRaw * 10) / 165;  // encoder resolution ratio  // 100% = valve in 90degree angle from closed position = fully open

  // PWM control - power to the motor
  if(mValveState != evs_Stopped)
  {

    uint16_t err = abs(mValvePosPct - mRequestPosPct);
    if(err < 1)
    {
      mIterm = 0;
    }
    else
    {
      if (mIterm < 600)
      {
        mIterm += (1 + (err/10));
      }
    }

    uint16_t dutycycle = AVC_MIN_DUTYCYCLE + err + mIterm/20;
    if(dutycycle > AVC_DEFAULT_DUTYCYCLE)
    {
      dutycycle = AVC_DEFAULT_DUTYCYCLE;

      max_Iterm = ((AVC_DEFAULT_DUTYCYCLE - err - 1)*20);

      if ( mIterm > max_Iterm)  // anti windup
      {
        mIterm = max_Iterm;  // limit the iTerm if it is growing uselessly too big
      }
    }

    if(mValveState == evs_Homing)
    {
      dutycycle = AVC_DEFAULT_DUTYCYCLE;
    }

    htim2.Instance->CCR2 = dutycycle;
  }


  // home pos switch
  if (!DI_Get(IN3_AV_HOMEPOS))
  {
    mValvePosRaw = 0;
    if(mValveState == evs_Homing)
    {
      Stop();
      mValveState = evs_Stopped;
    }
  }


  if(mValveState == evs_Homing)
  {
    return;
  }


  // safety limits
  if ((mValvePosPct < -10  || mValvePosPct > 110) && mValveState != evs_Stopped)
  {
    Stop();
  }


  // Run open condition
  if((mValvePosPct < (mRequestPosPct - AVC_POS_TOLERANCE)) && mValveState != evs_RunOpen)
  {
    if (mLastDirection == evd_Closing && mValvePosPct > 1)
     {
       mRequestPosPct += AVC_VALVE_MECH_PLAY;
     }
    RunOpen();
  }

  // run close condition
  if((mValvePosPct > (mRequestPosPct + AVC_POS_TOLERANCE)) && mValveState != evs_RunClose)
  {
     if (mLastDirection == evd_Opening)
     {
       mRequestPosPct -= AVC_VALVE_MECH_PLAY;
     }
     RunClose();
  }

  // target pos reached, hysteresis included
  if(((mValvePosPct >= (mRequestPosPct - AVC_POS_TOLERANCE)) && (mValvePosPct <= (mRequestPosPct + AVC_POS_TOLERANCE)) ) && mValveState != evs_Stopped)
  {
    Stop();
  }

}


// run in closing direction until Home switch is activated
void AVC_GoHome(void)
{
  if(GPIO_PIN_SET == HAL_GPIO_ReadPin(AV_HOME_GPIO_Port, AV_HOME_Pin))
  {
    mValveState = evs_Homing;
    RunClose();
    mValveState = evs_Homing;
  }
}


int16_t AVC_GetValvePos(void)
{
  return mValvePosPct;
}


int16_t AVC_GetRequestPos(void)
{
  return mRequestPosPct;
}


// Set request position
void AVC_SetRequestPos(uint16_t pos)
{


  if(pos > 100)
  {
    mRequestPosPct = 100;
  }
  else
  {
    mRequestPosPct = pos;
  }
}

// Changes actual valve position by pos_diff percent
void AVC_ChangePosBy(int16_t pos_diff)
{
  if (pos_diff < 0)
  {
    pos_diff -= AVC_POS_TOLERANCE;
  }
  if (pos_diff > 0)
  {
    pos_diff += AVC_POS_TOLERANCE;
  }

  mRequestPosPct = mValvePosPct + pos_diff;

  if(mRequestPosPct > 100)
  {
    mRequestPosPct = 100;
  }
  if(mRequestPosPct < 1)
  {
    mRequestPosPct = 0;
  }
}

// Private method, run in opening direction
void RunOpen(void)
{

  htim2.Instance->CCMR1 &= ~TIM_CCMR1_OC2M;
  htim2.Instance->CCMR1 |= (TIM_OCMODE_FORCED_INACTIVE << 8);  // channel 2 is shifted by 8 bits
  htim2.Instance->CCR2 = AVC_MIN_DUTYCYCLE;
  mLastDirection = evd_Opening;

  HAL_Delay(150);

  HAL_GPIO_WritePin(AV1_GPIO_Port, AV1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(AV2_GPIO_Port, AV2_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(AV3_GPIO_Port, AV3_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AV4_GPIO_Port, AV4_Pin, GPIO_PIN_SET);

  HAL_Delay(150);

  mValveState = evs_RunOpen;
  htim2.Instance->CCMR1 &= ~TIM_CCMR1_OC2M;
  htim2.Instance->CCMR1 |= (TIM_OCMODE_PWM1 << 8);

}

// Private method, run in closing direction
void RunClose(void)
{
  htim2.Instance->CCMR1 &= ~TIM_CCMR1_OC2M;
  htim2.Instance->CCMR1 |= (TIM_OCMODE_FORCED_INACTIVE << 8);  // channel 2 is shifted by 8 bits
  htim2.Instance->CCR2 = AVC_MIN_DUTYCYCLE;
  mLastDirection = evd_Closing;

  HAL_Delay(150);

  HAL_GPIO_WritePin(AV1_GPIO_Port, AV1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AV2_GPIO_Port, AV2_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AV3_GPIO_Port, AV3_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(AV4_GPIO_Port, AV4_Pin, GPIO_PIN_RESET);

  HAL_Delay(150);


  mValveState = evs_RunClose;
  htim2.Instance->CCMR1 &= ~TIM_CCMR1_OC2M;
  htim2.Instance->CCMR1 |= (TIM_OCMODE_PWM1 << 8);
}


// stop immediately
void Stop(void)
{
  mIterm = 0;

  htim2.Instance->CCMR1 &= ~TIM_CCMR1_OC2M;
  htim2.Instance->CCMR1 |= (TIM_OCMODE_FORCED_INACTIVE << 8);  // channel 2 is shifted by 8 bits

  mValveState = evs_Stopped;

  HAL_GPIO_WritePin(AV1_GPIO_Port, AV1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(AV2_GPIO_Port, AV2_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(AV3_GPIO_Port, AV3_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(AV4_GPIO_Port, AV4_Pin, GPIO_PIN_RESET);

}

//EXTI15_10_IRQHandler

void AVC_ExtiCallback(uint16_t GPIO_Pin)
{
  // encoder signal processing
  if(GPIO_Pin == AV_ENC1_Pin)
  {
    if(AV_ENC2_GPIO_Port->IDR & AV_ENC2_Pin)  // Pin ENC2 = 1
    {
      mValvePosRaw++;
    }
    else  // pin ENC2 = 0
    {
      mValvePosRaw--;
    }

    if((mValveState != evs_Stopped) && (mValveState != evs_Homing))
    {
      if(((mValvePosPct >= (mRequestPosPct - AVC_POS_TOLERANCE)) && (mValvePosPct <= (mRequestPosPct + AVC_POS_TOLERANCE))))
      {
        Stop();
      }
    }
  }
}

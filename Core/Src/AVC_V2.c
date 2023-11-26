/*
 * AVC_V2.c
 *
 *  Created on: 26.11.2023
 *      Author: marti
 *      Brief: Low-level Air valve driver Using LUFBERG servo for aira valves  Controlled by signal 0-10V. controlling signal is produced by PWM signal on PA10
 *
 *
 *     TECHM wiring :
 *            Servo control, PWM output PA10                        (GPIO connector)
 *            0-10V Signal converter,  GND                          (GPIO connector)
 *            0-10V Signal converter,  12V, PD2                     (AV out connector)  (Using H-bridge, just for 12V power supply, because there is no free connector with 12V)
 *            H-bridge enable, PWM output, PB3, TIM2_CH2
 *
 *            HomePos switch : AV_HOME,  PC9, EXTI9 interrupt       (AVO connector)    (Not used)
 *
 *
 */


#include "AVC_V2.h"
#include "main.h"
#include "stdlib.h"
#include "DO.h"

GPIO_InitTypeDef GPIO_InitStruct = {0};

int16_t mValvePosPct;
int16_t mRequestPosPct;

eValveDir mLastDirection;  // direction of last movement

void RunOpen(void);
void RunClose(void);
void Stop(void);


// initialize the peripherals and close the valve
void AVC_Init(void)
{
  // Initialize the power supply 12V for the Signal converter  (PWM -> 0-10V)

  HAL_GPIO_WritePin(AV1_GPIO_Port, AV1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AV2_GPIO_Port, AV2_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AV3_GPIO_Port, AV3_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(AV4_GPIO_Port, AV4_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(AV_EN_GPIO_Port, AV_EN_Pin, GPIO_PIN_SET);

  AVC_GoHome();

}

void AVC_Update_10ms(void)
{
  // Nothing here

}


// run in closing direction until Home switch is activated
void AVC_GoHome(void)
{
  DO_SetServoAirValve(AVC_PCT_HOME);
  mLastDirection = evd_Closing;
  mValvePosPct = AVC_PCT_HOME;
  mRequestPosPct = AVC_PCT_HOME;
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

  if(pos > AVC_PCT_FULL_OPEN)
  {
    mRequestPosPct = AVC_PCT_FULL_OPEN;
  }
  else
  {
    mRequestPosPct = pos;
  }
  DO_SetServoAirValve(mRequestPosPct);
  mValvePosPct = mRequestPosPct;
}

// Changes actual valve position by pos_diff percent
void AVC_ChangePosBy(int16_t pos_diff)
{
  AVC_SetRequestPos(mValvePosPct + pos_diff);
}

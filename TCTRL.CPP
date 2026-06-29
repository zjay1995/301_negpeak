#include "includes.h"
#pragma hdrstop

#include "mainwind.h"
#if INTERNAL()

#include "method.h"
#include "tctrl.h"


TempControl::TempControl(Method *_pMethod)
 : pMethod(_pMethod)
 , mode(T_INIT)
 , last_time(0)
{
  Begin();
}

TempControl::~TempControl()
{
  End();
}


void TempControl::Begin()
{
  mode = T_HOLD3;         /* set mode to last mode before call to setup. */
  SetupNextMode();
  OutputTemperatures();
}
void TempControl::Start()
{
  if(pMethod->method_table.RampingEnabled) {
    mode=T_INIT; // must be already in T_INIT
    SetupNextMode();
    OutputTemperatures();
  }
}
void TempControl::End()
{
  mode=T_HOLD3;
  SetupNextMode();
  OutputTemperatures();
}

#define TICS_PER_SEC 1000UL
#define SECS_PER_MIN 60.//60
#define HALF_PERIOD  0//30

const DWORD temperature_control_interval=100L; // 0.1 sec

void TempControl::ControlTemperature()
{
  DWORD time=GetTickCount();

  if(last_time+temperature_control_interval>time) return;
  last_time=time;

  switch( mode ) {
    case T_INIT:
      // nothing to do - waiting Start()
      /*if( pMethod->method_table.RampingEnabled == TRUE ) {
	SetupNextMode();
	OutputTemperatures();
      }*/
      OutputTemperatures();
    break;

    case T_HOLD1:
    case T_HOLD2:
    {
      OutputTemperatures();
      DWORD mode_time = time - mode_start_time;
      if( mode_time >= switch_time ) {
	SetupNextMode();
	OutputTemperatures();
      }
    }
    break;

    case T_RAMP1:
    case T_RAMP2:
    {
      DWORD mode_time = time - mode_start_time;
      t_oven = t_start + ( (rate * mode_time)/ TICS_PER_SEC + half_period_term )/SECS_PER_MIN;
      OutputTemperatures();
      if( mode_time >= switch_time )
	SetupNextMode();
	OutputTemperatures();
    }
    break;

    case T_HOLD3:
      // nothig to do - waiting end of run.
      OutputTemperatures();
    break;
  }
}


void TempControl::SetupNextMode()
{
  switch( mode ) {
    case T_INIT:
      mode = T_HOLD1;
      switch_time = pMethod->method_table.OvenHoldTime1*TICS_PER_SEC;
      t_oven = pMethod->method_table.oven_temp;
      t_inj = pMethod->method_table.inj_det_temp;
    break;

    case T_HOLD1:
      mode = T_RAMP1;
      rate = pMethod->method_table.OvenRampR1;
      if( rate == 0 ) {
	mode = T_HOLD3;
	break;
      }
      t_start = pMethod->method_table.oven_temp;
      half_period_term = HALF_PERIOD;
      t_oven = t_start;
      t_inj = pMethod->method_table.inj_det_temp;
      switch_time = ((pMethod->method_table.OvenHoldT2
	    - pMethod->method_table.oven_temp) * SECS_PER_MIN)*TICS_PER_SEC
	    / rate;
      if( switch_time < 0 ) {
	half_period_term = - half_period_term;
	switch_time = - switch_time;
	rate = -rate;
      }
    break;

    case T_RAMP1:
      mode = T_HOLD2;
      switch_time = pMethod->method_table.OvenHoldTime2*TICS_PER_SEC;
      t_oven = pMethod->method_table.OvenHoldT2;
      t_inj = pMethod->method_table.inj_det_temp;
    break;

    case T_HOLD2:
      mode = T_RAMP2;
      rate = pMethod->method_table.OvenRampR2;
      if( rate == 0 ) {
	mode = T_HOLD3;
	break;
      }
      t_start = pMethod->method_table.OvenHoldT2;
      t_oven = t_start;
      t_inj = pMethod->method_table.inj_det_temp;
      half_period_term = HALF_PERIOD;
      switch_time = ((pMethod->method_table.OvenHoldT3
	    - pMethod->method_table.OvenHoldT2) * SECS_PER_MIN)*TICS_PER_SEC
	    / rate;
      if( switch_time < 0 ) {
	half_period_term = - half_period_term;
	switch_time = - switch_time;
	rate = -rate;
      }
    break;

    case T_RAMP2:
      mode = T_HOLD3;
      t_oven = pMethod->method_table.OvenHoldT3;
      t_inj = pMethod->method_table.inj_det_temp;
    break;

    case T_HOLD3:
      mode = T_INIT;
      t_oven = pMethod->method_table.oven_temp;
      t_inj = pMethod->method_table.inj_det_temp;
    break;
  }
  mode_start_time = GetTickCount();
}


extern void SetTemperatures(double t_oven, double t_inj); // in ACQUIRE.CPP
void TempControl::OutputTemperatures()
{
  // in C degrees ??
  ::SetTemperatures(t_oven, t_inj); // in ACQUIRE.CPP
}



#endif  // internal
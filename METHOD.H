#ifndef  _METHOD_H
#define  _METHOD_H

#ifndef  _WCPPLIB_WOBJECT_H
#include "wcpplib\wobject.h"
#endif
#ifndef  _WCPPLIB_STR_H
#include "wcpplib\str.h"
#endif
class WCPPLIB_CLASS iwstream;
class WCPPLIB_CLASS owstream;

#ifndef  _DETECTOR_H
#include "detector.h"
#endif


class MethodTable {

 public:
  uint     inj_det_temp;
  uint     oven_temp;
  uint     line_temp;  // added.  P.P. 03/28/17
  uint desorb_temp;  // added P.P. 07/15/18 - for the case when 3 temp zones -  oven+inj/det+oncentrator
  uint fid_temp;  // added P.P. 07/16/18 - for the case when 4 temp zones -  oven+inj/det+oncentrator+fid
  // some extra fields for temperature programming in  GC 311 vesion 2.0
  uint RampingEnabled; // bool
  ulong OvenHoldTime1; // time
  uint OvenRampR1;
  uint OvenHoldT2;     // temp
  ulong OvenHoldTime2; // time
  uint OvenRampR2;
  uint OvenHoldT3;     // temp
  ulong OvenHoldTime3;  // time

  ulong    analysis_time;
  ulong    sample_time;
  ulong    inject_time;
  ulong    equilibrate_time;  // added. P.P. 04/06/02
  ulong    next_point_time;   // added. P.P. 04/09/02
  ulong    calibrate_time;    // added. P.P. 12/19/02
  ulong    purge_time;        //
  ulong    x1_time;  // added. P.P. 07/29/04
  ulong    x1_off_time;  // added. P.P. 06/18/06
  ulong    x2_time;  // added. P.P. 06/20/10
  ulong    x2_off_time;  
  ulong    x3_time;  // added. P.P. 06/20/10
  ulong    x3_off_time;  
  ulong    x4_time;  // added. P.P. 06/20/10
  ulong    x4_off_time;
  ulong    x5_time;  // added. P.P. 07/01/10
  ulong    x5_off_time;
  int      syringe_on_off;

  int      TWA_on_off;
  int      TWA_cycle;    // to be ulong  // 3 lines replaced. P.P. 01/03/03
  int      cal_cycle;    // to be ulong  //
  int      repeat_cycle; // to be ulong  //
  int      alarms_on_off;
  int      STEL_on_off;
  int      paper_save_on_off;
  int      cont_pump_on_off;
  int      plotter_on_off;
  int      check_alarm;    // added. P.P. 04/06/03

  ulong points; // all bytes zero - equal the first byte set
  ulong lalarms;
  ulong halarms;

  double set_point;   // pre-defined value for makeup ON
  double set_point_1; // pre-defined value for makeup OFF
  double set_point_2; // pre-defined value for Inj Flow
  double set_point_3; // pre-defined value for Column Flow
  double TempSlope;     // adjusting temp sensor parameters. P.P. 07/20/06
  double TempIntercept; //
  uint   TempSetting;   // bool, enable setting. P.P. 07/21/06
//  ulong desorb_temp;  //
  int	   second_pump_on_off; // added. P.P. 01/30/09
  ulong    dsb_preheat_time; // added P.P. 03/07/10
  ulong    quick_scan_time; // added P.P. 12/14/10
  int	   quick_scan_enabled; // added. P.P. 12/14/10
  ulong    zero_gas_time;     // P.P. 03/15/11
  int	   zero_gas_cycle;    //
  int      oven_door_manual; // P.P. 09/06/12
  ulong    desorb_time;  // 2 lines added. P.P. 02/11/06  and removed 02/20/06  dsb_time restored 03/12/17
  ulong    dsb_wait_time; // added P.P. 03/26/18

  MethodTable();
  ~MethodTable();

  void *read(iwstream &is, unsigned long version);
  void write(owstream &os);
};

class Method : public WObject {
 public:
  MethodTable method_table;              // method form data
  Detector det[NUMDETECTORS];	// Detector specific parameters
  Str Notes;

  Method();
  virtual ~Method();


  virtual void *readOld(iwstream &is); // Peak Work 4.0 for DOS
  virtual void *read(iwstream &is, unsigned long version);
  virtual void write(owstream &os);

  BOOL method_needs_transmitting;
};

#endif


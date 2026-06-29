#ifndef  _ACQUIRE_H
#define  _ACQUIRE_H


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#ifndef  _CALC_H
#include "calc.h"
#endif

#ifndef _GRPHWIND_H    // added. P.P. 11/02/12
#include "grphwind.h"  //
#endif                 //

#ifndef  _JOB_H
#include "job.h"
#endif
#ifndef  _WINLIB_WTIMER_H
#include "winlib\wtimer.h"
#endif

//class MainWindow;
#include "mainwind.h"

class TempControl;
class Comm;

#define  NUM_METHOD_ELEMENTS    37
#define  NUM_METHOD_ELEMENTS_2  8
#define  NUM_VERSION_ELEMENTS   2
class Acquire: public Integration, public WTimer {
  MainWindow *pMainWindow;
 public:
  int acquire_type;	/* Job or Cal. acquire */
  int nStandard;
  BOOL oven_door;  //added. P.P. 05/18/02
  BOOL waiting_for_response;	// State of CheckComm() function
#if STAND_ALONE()
  BOOL bNeedClose;    // added. P.P. 08/16/02
#endif

  enum {
    RECEIVING_DATA,
    START_ACQUIRE,
    START_CAL_ACQUIRE,
    END_ACQUIRE,
    GET_STATUS,
    ABORT,
    TRANS_METH,
    TRANS_METH_2,
    GET_VERSION,
    SYNC_CLOCK
    , WAIT_TEMPERATURES
    , WAIT_SAMPLE_TIME
    , WAIT_START_INJECT
    , WAIT_OVEN_DOWN           // new to cool oven down 5 degrees below initial. P.P. 08/08/02
  } comm_state;  // current state of acquire from comm.
#if INTERNAL()
  DWORD dwSampleTime;
  DWORD dwStartInjectTime;
  DWORD dwInjectTime;
  DWORD dwInjectTime2;    // for 2nd injection, to 2nd column. P.P. 01/29/17
  DWORD dwInjectTime2_off;    // for 2nd injection, to 2nd column. P.P. 01/29/17
  DWORD dwEquilibrateTime;
  DWORD dwEndRunTime;
  DWORD dwNextPointTime; // added for sampling ahead. P.P. 12/30/02
  DWORD dwPurgeTime;     // added. 02/11/06
  DWORD dwShowMessageTime; // added. P.P. 11/06/09
  DWORD dwWaterPumpTime;
  DWORD dwCleanPowerOnTime;     // added. 10/26/17
  DWORD dwCleanPowerOffTime;     // added. 10/26/17
  DWORD dwMoistureOutTime;       // P.P. 12/26/17
  DWORD dwDesorberBatteryCheckTime; // added 03/23/18
#endif  // interal
  char  num_det_active;   // # of detectors currently active
  BOOL not_cued;          // reset after cueing user for syringe inj.

  uint8 last_mess[124];   // last mess. sent to 311 GC
  int trans_len;          // length of current trans. mess.

  char trans_state;       // state of a table transmission
  char last_key;          // last method trans. key

  char det_A_B;           // current detector being transmitted


  uint32 rdata[33]; int ndata;

  Method *pMethod;
  Run *pRun;
  int32 huge *pCurRawData; // output to Run
  int32 huge *pCurRawDataLastPainted;

  Acquire(Comm *pComm, int _acquire_type, int acq_mode, int nStandard, Method *_pMethod, PeakList **_pPeakList, Run *_pRun, Job *_pJob, int _pointnumber, MainWindow *pMainWindow);
  virtual ~Acquire();

  void WriteRawData(uint32 *raw_data, int len);

  void Start();

  void StartMethTrans();
  void StartAcquire();
  virtual int Continue(); // CheckComm()

#if INTERNAL()
  int flagsAlarm;
  static void Alarm(BOOL on);   // added "static" as it was with Stop Alarm. P.P. 02/16/02
  int flagsLamp;
  static void Lamp(BOOL on);  // static added in both lamps. P.P. 06/21/06
  static void LampB(BOOL on); // added. P.P. 12/31/02
//  void AutoZero(BOOL on);
  static void AutoZero(BOOL on); // P.P. 08/28/09  here 12/04/09
  static void AutoZeroB(BOOL on); // P.P. 06/21/17  added for separate autozero for det B
  static void SamplePump(BOOL on);
  void Concentrator(BOOL on);  // added. P.P. 02/05/06
  void Purge(BOOL on);         //
  static void TCDDetector(BOOL on);

  static void Inject(BOOL on);
  void Equilibrate(BOOL on); // added. P.P. 10/30/02
  void CheckLamp(long curr_rt);
  void SendEndAcquire();
  static void Makeup(BOOL on); // added. P.P. 01/05/04
  static void SetFlowMode(int flow_mode);

  //void PointNumber();
  void PointNumber(int PointNumber);
  void WaterPump(int water_pump_number, BOOL on); // new function to pump water out. P.P. 12/31/16
  static void Low_Alarm(int pointnumber, BOOL on);  //P.P. 10/06/01 // static added. 11/18/02
  static void High_Alarm(int pointnumber, BOOL on); // P.P.         // same, to stop alarms after run stops
  static void Diagnostics_Alarm(int diagnostics_bitnumber, BOOL on); // added. P.P. 02/19/03
  void Filter_valve(BOOL on);       // P.P. 01/12/02
//  void OvenDoor(BOOL on);           // P.P. 04/02/02
  static void OvenDoor(BOOL on);      // P.P. 06/18/06
  static int  CheckRemoteStart();
  static void FlameDetector(BOOL on); // added. P.P. 06/28/05
  static void HydrogenValve(BOOL on); // P.P. 06/28/05
  static void EnableOven(BOOL on); // added. P.P. 06/30/06
  static void Fan(BOOL on); // added. P.P. 02/23/10
  static void X1_valve(BOOL on);  // 4 functions added. P.P. 06/20/10
  static void X2_valve(BOOL on);
  static void X3_valve(BOOL on);
  static void X4_valve(BOOL on);
  static void X5_valve(BOOL on);  // added P.P. 07/01/10
  static void ZeroGasValve(BOOL on);   // added P.P. 03/17/11
  static void SetLineTemperature(double temperature);
  static void SetDsbTemperature(double temperature); // added P.P. 07/27/18
  void Acquire::SecondPump(BOOL on);     // new function. P.P. 08/03/18.

//  static double Check_Temp_Safety();  // P.P. 04/20/09
#else
#endif // internal

  int CheckCalibration();

  DWORD dwRepeatTime;

  int32 uv_0[NUMDETECTORS];
  int32 uv_Max[NUMDETECTORS]; // added. P.P. 09/26/03
  DWORD dwUV_0_Time;
  DWORD dwUV_Max_Time; // added. P.P. 09/26/03
#if X1_VALVE() // cond changed. P.P. 12/10/04
  DWORD dwXvalveTime;      //
  DWORD dwXvalveOffTime;      // added. P.P. 06/18/06
  DWORD dwXvalve2Time;      //  3 more valves. P.P. 06/20/10
  DWORD dwXvalve2OffTime;      
  DWORD dwXvalve3Time;      //  3 more valves. P.P. 06/20/10
  DWORD dwXvalve3OffTime;      
  DWORD dwXvalve4Time;      //  3 more valves. P.P. 06/20/10
  DWORD dwXvalve4OffTime;
  DWORD dwXvalve5Time;      //  1 more valve. P.P. 07/01/10
  DWORD dwXvalve5OffTime;
  DWORD dwAutoZero1;       //
  DWORD dwAutoZero1_off;   //
  DWORD dwRange_1;   //    // P.P. 05/12/17  at this time, swich to new range.
  DWORD dwRange_1_off;   //    // P.P. 09/18/15  at this time, retutn to initial range.
  DWORD dwAutoZero2;       //   added. P.P. 09/11/15
  DWORD dwAutoZero2_off;   //
  DWORD dwRange_1_2;   //    // P.P. 06/20/17  at this time, swich to new range det B.
  DWORD dwRange_1_2_off;   //    // P.P. 09/18/15,  and again
  DWORD dwAutoZero3;       //   added. P.P. 09/11/15
  DWORD dwAutoZero3_off;   //
  DWORD dwAutoZero4;       //   added. P.P. 06/20/17
  DWORD dwAutoZero4_off;   //
//  DWORD dwRange_1_3_off;   //    // P.P. 09/18/15, and again  rem - not needed. P.P. 06/20/17
#endif                     //
 //not used DWORD dwTemperatureDelayTime; // added. P.P. 12/30/04
#if FID()
  DWORD dwFidDelayTime; // added. P.P. 07/03/05
  DWORD dwFidTempTimeout; // added. P.P. 07/24/05
  DWORD dwHydrogenDelayTime; // added. P.P. 09/20/05
#endif
#if RAMPING() // case added. P.P. 06/23/06
  DWORD dwOvenHoldTime1;
  DWORD dwOvenHoldTime2;
  DWORD dwOvenHoldTime3;
  DWORD dwOvenRamp1Time;
  DWORD dwOvenRamp2Time;
#endif // ramping  P.P. 06/23/06
  DWORD dwDesorbTime; // P.P. 03/12/17
  void SetLamps(BOOL bOn);

#if INTERNAL()
  DWORD dwLastTime;
  TempControl *tempcontrol;
#else
//  Comm *pComm; // rem for short
#endif

  int pointnumber;  // why lower case here but upper case in acquire.cpp? P.P. 02/19/03 
  int NextPointNumber; // added. P.P. 01/08/03

  int gain[NUMDETECTORS];
  int gain_1[NUMDETECTORS]; // added. P.P. 11/09/12
  BOOL bRepeatCalSend;
#if QUICK_SCAN() // added. P.P. 12/31/10
  int quick_scan_active;  // 01/02/11
  DWORD dwAnalysisTime; // added. P.P. 01/15/11
  DWORD dwZeroGasAutozeroTime; // P.P. 03/18/11
#endif
  DWORD dwMotorOffTime; // added. P.P. 09/30/11
  static void DoorMotor(BOOL on); // added P.P. 10/04/11
};


#if INTERNAL()
void EvTimer(MainWindow *pMainWindow);
//void InitInternal();
int InitInternal(); // replacement. P.P. 01/01/07
void StartInternal(BOOL on); // added. P.P. 05/30/04
#endif // internal

#endif // ndef _acquire







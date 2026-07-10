#ifndef  _CALC_H
#define  _CALC_H

#ifndef  _CONSTS_H
#include "consts.h"
#endif
#ifndef  _SHORTS_H
#include "shorts.h"
#endif

#define  NOISE                0
#define  BETWEEN_PEAKS        1
#define  PEAK_STARTING        2
#define  PEAK_STILL_STARTING  4
#define  PEAK_RISING          5
#define  STILL_DESCENDING     6
#define  END_OF_PEAK          7
#define  NEW_PEAK_RISING      8
#define  NEW_PEAK             11
#define  PEAK_DESCENDING      13


#ifndef  _TIME_T
#define  _TIME_T
typedef long time_t;
#endif


#include "internal.h" // POINTS()
class Method;
class Run;
class PeakList;
//class Acquire; // added. P.P. 01/03/11  and rem 01/16/11
#include <windows.h>

extern float TWA_results[POINTS()][NUMDETECTORS][COMPONENT_ROWS];  	// TWA results
extern float max_TWA_conc[POINTS()][NUMDETECTORS][COMPONENT_ROWS];	// max. conc. in TWA
extern float min_TWA_conc[POINTS()][NUMDETECTORS][COMPONENT_ROWS];	// max. conc. in TWA
#if USE_TOTAL()
extern float TWA_total[POINTS()][NUMDETECTORS];  	// TWA for total
extern float max_TWA_total[POINTS()][NUMDETECTORS];	// total max. in TWA
extern float min_TWA_total[POINTS()][NUMDETECTORS];	// total min. in TWA
extern float total_point[POINTS()][NUMDETECTORS];  //added for total concentrations in points. P.P. 12/26/04
extern int TWA_runs_total[POINTS()][NUMDETECTORS]; // P.P. 12/26/04
extern int flags_point_total[POINTS()][NUMDETECTORS];  //added for total concentrations in points. P.P. 12/26/04
#endif // USE_TOTAL()

extern uint32 TWA_start_time;      			   	// time TWA cycle began
extern int TWA_runs[POINTS()][NUMDETECTORS][COMPONENT_ROWS]; // P.P. 01/11/03	// # of runs in a TWA
void InitTWA();
void InitDAC();                        // added P.P. 12/22/01
void TWA_calc(Method *pMethod, Run *pRun, PeakList **pPeakList, int pointnumber);
extern DWORD TWA_cycle_start_time;
extern float concen_point[POINTS()][NUMDETECTORS][COMPONENT_ROWS]; // added. P.P. 12/24/02
extern int flags_point[POINTS()][NUMDETECTORS][COMPONENT_ROWS];  //added for current concentrations in points
extern long time_point[POINTS()][NUMDETECTORS][COMPONENT_ROWS];  //added for current concentrations in points
extern time_t start_time[POINTS()][NUMDETECTORS][COMPONENT_ROWS];  //added for current concentrations in points


#if TOTAL_SUMMARY()
#include "wcpplib/wlist.h"
class TotalListNode : public List::Node
{
 public:
  float conc[POINTS()][NUMDETECTORS];
  time_t tim[POINTS()][NUMDETECTORS];
};

class TotalList : public List {
 public :
  TotalList() {}
  ~TotalList() { shutDown(); }
};
extern TotalList total_list;
void InitTotalList();
void AddConcentration(int pointnumber, int n, float conc, time_t start_time);
#endif // TOTAL_SUMMARY()



extern float STEL_results[STEL_RUNS][NUMDETECTORS][COMPONENT_ROWS];	// STEL results
extern float max_STEL_conc[NUMDETECTORS][COMPONENT_ROWS];	// max. conc. in a STEL */
extern int   STEL_q_idx;					// current index into the STEL Qs
extern uint32 STEL_start_time;					// time STEL cycle began
extern int   STEL_runs;					// # of runs in a STEL


//class Method; // rem - exists above! P.P. 02/17/12
//class Run;    //
class Acq;

//#define MAX_Q		32
#define MAX_Q		42 // 40 in DialogBox
#define MULT_Q          3

class DataQueue {
 public:
  long data[MAX_Q  *MULT_Q];
  int i;
  int segment_width;
  int nMultipleSegment;
  int real_segment_width;

  DataQueue(int _segment_width);

  void Put(long d);
  void SetMultipleSegment(int _nMultipleSegment)
  {
    nMultipleSegment=_nMultipleSegment; // Will be used when  Put set i=0;
  }

  long GetScaledSample();
  long GetRawSample();
  int IsSampleFinished();
};

#ifndef  _DATASTRC_H
#include "datastrc.h"
#endif

class RawData {
 public:
//  _raw_struct  raw_data;
  struct {
//    int      time;
    int32      time;    // changed to allow runs longer than 23 min. P.P. 11/06/02
//    uint16   raw_units;
//    long     atten_sample;
  } raw_data;

  int         gain;	      // gain from first data point, for tracing autoscale
//  long        cur_autoscale;  // current autoscale values. Uncommented P.P. 12/27/12
  int        cur_autoscale;  // P.P. 01/11/13
  uchar       acquire_finished;
//  uint        analysis_time;
  ulong        analysis_time;  // P.P. 09/06/09

//  int samp_cnt;  // # points in current sample
  DataQueue dq;
  int nErrorMesages;

//  RawData(int _segment_width, uint _analysis_time);
  RawData(int _segment_width, ulong _analysis_time);  // P.P. 09/06/09
  ~RawData();

  //void ProcessRawData(uint16 data);
  void ProcessData(uint32 data);           // restored. P.P. 11/09/14
//  void ProcessData(int32 data);
};

//typedef long CalRes[COMPONENT_ROWS][STAND_NUM];
typedef double CalRes[COMPONENT_ROWS][STAND_NUM]; // P.P. 11/18/17. To use with area as double.

#ifndef  _PEAK_H
#include "peak.h"
#endif
class PeakList;
class Detector;
class Run;
class Integrator: public RawData {

 public:
////  int gain;  // from Run
  CalRes &cal_results;  // from Run
  int acquire_type;           // Job, Cal. or Check Cal. acquire
  int cal_passed;  // cal. val. flags

  int nStandard;
  /* --- */
  /*
  char       acq_NandBtime[6];
  char       acq_NandBlen[6];
  long       acq_thresh;
  int        acq_segment_width;
  char       acq_peak_alg;
  */
  Method *pMethod;
  Detector *pDetector;
  Run *pRun;
  int nDetector;
//  Acquire *pAcquire; // added. P.P. 01/03/11 rem 01/05/11
  long temp_noise;  // noise and baseline accumulators
  long temp_base;
  int NandBctr;

  double peak_area; // temporary clculation need more accuracy
  long start_of_peak;
  long end_of_peak;
  long time_peak_ends;
  long skim_ending_point;
  long skim_starting_point;
//  long peak_half_width;
  double peak_half_width;  // to overcome problem with too large values of area. P.P. 08/25/17

  long prev_raw_unit;
  double prev_slope;
  double prev_prev_scaled_sample;
  BOOL prev_raw_invalid;
  long prev_scaled_sample;
  long noise;
  long act_thresh;

  long prev_retention_time;
  long last_time;
  long next_skim_starting_point;
  long next_start_of_peak;
  double next_peak_area;

  BOOL peak_rising;

  int analyze_segment;
  //int peakX, peakY;

  // negative-peak detection state for the production state machine
  // (peak_alg 0/1). Detection only - see DetectNegativePeak(). 06/29/26
  long neg_start_of_peak;  // ret. time when current neg. peak began (-1 = none)
  long neg_peak_min;       // most negative height (<=0) seen in current neg. peak
  double neg_peak_area;    // accumulated (negative) area under baseline. 07/10/26

  Peak peak;	// current peak
  PeakList *pPeakList;

///???  float comp_slope[COMPONENT_ROWS];  // slope for each component
//  int   ref_comp_idx;  // index to reference component


  long scaled_sample;
  long raw_sample;
  int ext_det_done; // added P.P. 11/22/08
  int quick_run;  // added. P.P. 01/05/11

  Integrator(Method *_pMethod, Detector *_pDetector, Run *_pRun, CalRes *_cal_results, PeakList *_pPeakList, int _acquire_type, int _nDetector, int _nStandard);
  virtual ~Integrator();


  int DetectPeaks();

  void NoiseAndBaseline( void );
  void BetweenPeaks( void );
  void PeakStarting( void );
  void PeakStillStarting( void );
  void PeakRising( void );
  int StillDescending( void );
  void NewPeakRising( void );
  void NewPeak( void );
  void PeakDescending( void );
  void DetectNegativePeak( void ); // neg. peak detection in main algorithm. 06/29/26
  int EndOfPeak( );
  int PeakMatchup( void );
  int PeakMatchup( Peak *pPeak, int bPreliminary );
  static int CheckRT( const Peak *pPeak, Component *pComponent );
};



class Integration
{
 public:
  int acquire_type;
  int nStandard;
  Run *pRun;

  PeakList **pPeakList;
  Integrator *pIntegrator[NUMDETECTORS];
  Method *pMethod;

  Integration(Method *_pMethod, PeakList **_pPeakList, Run *_pRun, int _acquire_type, int _nStandard);

  virtual ~Integration();
  virtual int Continue() = 0;
};


#ifndef  _RUN_H
#include "run.h" // RawDataIterator
#endif
class RunIntegration: public Integration {
 public:
  Run *pRun;
  long iCur;
  int bRun[NUMDETECTORS];
  #if 1
  RawDataIterator *it[NUMDETECTORS];
  #endif

  RunIntegration(Method *_pMethod, PeakList **_pPeakList, Run *_pRun, int _nStandard);
  virtual ~RunIntegration();

  virtual int Continue();

};



#endif


